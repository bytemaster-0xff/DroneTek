#include "HMC5883L.h"
#include "Comms.h"
#include "eeprom.h"
#include "RoverEEAddresses.h"

HMC5883L Compass;
bool HasCompass;

float CurrentHeading;

static bool __isCalibrating = false;
static bool _isCompassCalibrated = 0;
static int16_t __min_x;
static int16_t __min_y;

static int16_t __max_x;
static int16_t __max_y;

static int16_t __range_x;
static int16_t __range_y;


static void writeCompassCalibration()
{
	EEPROM.write(COMPASS_CALIBRATED, 0x33);

	EEPROM.write(COMPASS_MAX_X, __max_x >> 8);
	EEPROM.write(COMPASS_MAX_X, __max_x & 0xFF);

	EEPROM.write(COMPASS_MAX_Y, __max_y >> 8);
	EEPROM.write(COMPASS_MAX_Y, __max_y & 0xFF);

	EEPROM.write(COMPASS_MIN_X, __min_x >> 8);
	EEPROM.write(COMPASS_MIN_X, __min_x & 0xFF);

	EEPROM.write(COMPASS_MIN_Y, __min_y >> 8);
	EEPROM.write(COMPASS_MIN_Y, __min_y & 0xFF);
}

static void readCompassCalibration()
{
	__max_x = EEPROM.read(COMPASS_MAX_X) << 8 | EEPROM.read(COMPASS_MAX_X + 1);
	__max_y = EEPROM.read(COMPASS_MAX_Y) << 8 | EEPROM.read(COMPASS_MAX_Y + 1);
	__min_x = EEPROM.read(COMPASS_MIN_X) << 8 | EEPROM.read(COMPASS_MIN_X + 1);
	__min_y = EEPROM.read(COMPASS_MIN_Y) << 8 | EEPROM.read(COMPASS_MIN_Y + 1);
}

void BeginCompassCalibration()
{
	uint8_t m_motorOutBuffer[6];
	sprintf_s(OutputBuffer, OUTPUT_BUFFER_SIZE, "Begin Compass Calibration");
	WriteOutput();
	__min_x = 10000;
	__min_y = 10000;

	__max_x = -10000;
	__max_y = -10000;

	Wire.beginTransmission(0x42);
	m_motorOutBuffer[0] = 111;
	m_motorOutBuffer[1] = 20;
	m_motorOutBuffer[2] = 0;

	Wire.write(m_motorOutBuffer, 3);
	Wire.endTransmission();
	__isCalibrating = true;

}


void EndCompassCalibration()
{
	__isCalibrating = false;
	uint8_t m_motorOutBuffer[6];

	Wire.beginTransmission(0x42);
	m_motorOutBuffer[0] = 100;
	Wire.write(m_motorOutBuffer, 3);
	Wire.endTransmission();

	sprintf_s(OutputBuffer, OUTPUT_BUFFER_SIZE, "End Compass Calibration");
	WriteOutput();

//	writeCompassCalibration();
}

void InitCompass()
{
	Compass = HMC5883L();
	int error = Compass.SetScale(470); // Set the scale of the compass.
	if (error != 0) // If there is an error, print it out.
	{
		Serial.println(Compass.GetErrorText(error));
		HasCompass = false;
		return;
	}

	error = Compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
	if (error != 0) // If there is an error, print it out.
	{
		Serial.println(Compass.GetErrorText(error));
		HasCompass = false;
		return;
	}

	__min_x = -210;
	__max_x = 47;

	__min_y = -283;
	__max_y = -62;

	__range_x = __max_x - __min_x;
	__range_y = __max_y - __min_y;


/*	_isCompassCalibrated = EEPROM.read(COMPASS_CALIBRATED) == 0x33;
	if (_isCompassCalibrated)
		readCompassCalibration();
		*/
	HasCompass = true;
}

void Output(MagnetometerRaw raw, MagnetometerScaled scaled, float heading, float headingDegrees)
{
	int deltaX = raw.XAxis - __min_x;
	int deltaY = raw.YAxis - __min_y;

	float y = ((float) (deltaY) / (float) (__range_y / 2)) - 1;
	float x = ((float) (deltaX) / (float) (__range_x / 2)) - 1;

	sprintf_s(OutputBuffer, OUTPUT_BUFFER_SIZE, "Heading: %d\t%f\t%f\r\n", (int) headingDegrees, x, y);
	WriteOutput();
}

void ReadCompass()
{
	if (!HasCompass)
		return;

	MagnetometerRaw raw = Compass.ReadRawAxis();

	if (__isCalibrating)
	{
		if (raw.XAxis < __min_x) __min_x = raw.XAxis;
		if (raw.YAxis < __min_y) __min_y = raw.YAxis;
		if (raw.XAxis > __max_x) __max_x = raw.XAxis;
		if (raw.YAxis > __max_y) __max_y = raw.YAxis;
	}

	// Retrived the scaled values from the compass (scaled to the configured scale).
	MagnetometerScaled scaled = Compass.ReadScaledAxis();

	// Values are accessed like so:
	int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)

	int deltaX = raw.XAxis - __min_x;
	int deltaY = raw.YAxis - __min_y;

	float y = ((float) (deltaY) / (float) (__range_y / 2)) - 1;
	float x = ((float) (deltaX) / (float) (__range_x / 2)) - 1;

	// Calculate heading when the magnetometer is level, then correct for signs of axis.
	float heading = atan2(x, y);

	// Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
	// Find yours here: http://www.magnetic-declination.com/
	// Mine is: 2� 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
	// If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
	float declinationAngle = 0.0457;
	heading += declinationAngle;

	// Correct for when signs are reversed.
	if (heading < 0)
		heading += 2 * PI;

	// Check for wrap due to addition of declination.
	if (heading > 2 * PI)
		heading -= 2 * PI;

	// Convert radians to degrees for readability.
	float headingDegrees = heading * 180 / M_PI;
	 
	// Output the data via the serial port.
	//Output(raw, scaled, heading, headingDegrees);

	//CurrentHeading = 360 - ((headingDegrees * 0.25) + (CurrentHeading * 0.75));

	CurrentHeading = 360 - headingDegrees;
}