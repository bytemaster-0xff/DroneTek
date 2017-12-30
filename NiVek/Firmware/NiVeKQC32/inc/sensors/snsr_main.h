#ifndef SNSR_MAIN_H_
#define SNSR_MAIN_H_

#include "common/twb_common.h"
#include "twb_config.h"
#include "common/twb_i2c.h"
#include "commo/commo.h"

iOpResult_e TWB_InitSensorArray(void);

iOpResult_e TWB_IMU_RestoreDefaults(void);

iOpResult_e TWB_Sensors_Init(void);

iOpResult_e TWB_IMU_ProcessSensors(uint16_t ticks);

iOpResult_e TWB_Sensors_Zero(float ticksMS);
iOpResult_e TWB_Sensors_Start_Zero(void);

iOpResult_e TWB_Sensors_Start(void);
iOpResult_e TWB_Sensors_Pause(void);
iOpResult_e TWB_Sensors_Resume(void);
iOpResult_e TWB_Sensors_Stop(void);

extern uint8_t __commonSensorBuffer[30];

void TWB_Sensors_QueueMsg(void);
iOpResult_e TWB_Sensor_Calibrate(uint8_t i2cAddr, uint8_t regAddr, uint16_t eepromAddr, uint8_t sampleSize, float pauseMS, s16_vector_t *cals, bool bigEndian);

iOpResult_e TWB_Sensors_HandleMessage(TWB_Commo_Message_t *msg);

typedef struct {
	/* Will be the most recent rotation rate from the gyros */
	float RateXDPS;
	float RateYDPS;
	float RateZDPS;

	/* Will be the integrated degrees moved over deltaT
	 * these values will be cleared by the consumer of
	 * the values.
	 */
	float DeltaX;
	float DeltaY;
	float DeltaZ;

	/* Amount of time the above deltas have been
	 * accumulated
	 */
	float DeltaT;

} GryoExtendedData_t;

typedef enum {
	SensorStatus_Idle,
	SensorStatus_Disabled,
	SensorStatus_Ready,
	SensorStatus_Online,
	SensorStatus_Failure
} SensorStatus_t;

typedef struct {
	char *Name;				 /* Name of the sensor */
	SensorDevice_e SensorId;

	/*
	 * Simple value to determine if the sensor
	 * is enabled or not.
	 */
	Enabled_t IsEnabled;

	/*
	 * Where config starts in EEPROM for this sensor
	 * Each sensor has 64 bytes to store configuration data (as defined in twb_eeprom.h)
	 *
	 * The EEPROMBaseAddr + CONFIG_BLOCK_SIZE is used by telemetry when updating
	 * configuration values for the sensor.  If the address falls within this
	 * range, then we know the update is for that sensor.
	 */
	uint16_t EEPROMBaseAddr;

	 /*
	  * Ticks for Last Update, a tick is 1/100 of a MS
	  * and is used to determine when the sensor should
	  * be read again.  This values is set right before
	  * the sensor read value is called and should be
	  * pretty accurate.
	  */
	uint16_t LastUpdate;

	 /*
	  * To allow processing to take place in our
	  * main thread (driven from TIM7 IRQ) any background
	  * tasks that need to have some data processed or signal
	  * the completion of their work will set this flag.
	  *
	  * The main thread loop will look to this flag and call
	  * IRQ method to do the actual processing.
	  *
	  * Set from IRQ Channel (usually EXTI, cleared in software,
	  * usually in the IRQ method.)
	  *
	  * Any time this is set on the sensor (and the sensor is
	  * configured with a sample rate of IRQ, this method is
	  * called)
	  *
	  */
	IRQAssertionStatus_e IRQAsserted;

	/*
	 * This is set when data is ready to be processed
	 * DataReady is different than IRQAsserted in that at
	 * this point, all the data is queued up in the internal
	 * buffers for the sensor and the data just needs
	 * to be calculated.
	 *
	 * When this flag is set on an enabled sensor,
	 * the ProcessData() method is called.
	 *
	 */
	SensorDataReady_e DataReady;

	/* Current status of Sensor */
	SensorStatus_t Status;

	/*
	 * Enumeration value of how often sensor should be sampled,
	 * it is either an enumeration to be resolved to a ms
	 * or it is set to be triggered by an IRQ.
	 */
	SampleRate_e SampleRate;


	/*
	 * Under normal circumstances, this is set to
	 * OK, if there was a recent failure, it will
	 * be set to the failure reason.
	 */
	iOpResult_e SensorFault;

	/* Call back methods to work with sensor (sort the interface) */

	/*
	 * Required and called upon initial startup of the sensor,
	 * this is called only once.
	 */
	iOpResult_e (*Init)(void);

	/*
	 * Method to be called when the system needs to restore
	 * the sensor to the original state.
	 */
	iOpResult_e (*ResetDefaults)(void);

	/*
	 * Methods to be called to control the state
	 * of the sensor, Pause/Stop must disable any
	 * IRQ's and filling of FiFo buffers.
	 *
	 * Upon a call to start and resume, the sensor
	 * must be ready to accept and process interrupts.
	 */
	iOpResult_e (*Start)(void);
	iOpResult_e (*Pause)(void);
	iOpResult_e (*Resume)(void);
	iOpResult_e (*Stop)(void);

	/*
	 * This can be called by one of two methods:
	 * - Sample Rate timing has expired, with a period set
	 *   for the sample rate
	 *
	 * - The IRQAsserted is set, and the sample rate is set
	 *   to key off of an IRQ
	 *
	 * Read will generally queue an I2C Job to be processed
	 * by the I2C Job Queue
	 *
	 * For Analog sensors, work will be done when ever necessary
	 * more work may or may not need to be done.
	 *
	 * Upon completion of this call, more work may or may
	 * not need to be done.  Timing on this should be very
	 * fast < 10 uSeconds.
	 */
	iOpResult_e (*Read)(float ticks);

	/*
	 * ProcessData is called when the DataReady field is set
	 * for this sensor.  This is usually done in some background
	 * task which is usually an IRQ handler from I2C or some
	 * other EOC value.  Key is that the background tasks
	 * just sets the flag, then this picks up the work
	 * that needs to be done and processes it on our
	 * main thread.
	 */
	iOpResult_e (*ProcessData)(void);

	/*
	 * Called whenever the IRQAsserted is set
	 */
	iOpResult_e (*HandleIRQ)(void);

	/*
	 * Next set of methods will be called upon demand from
	 * an external source, generally telemetry
	 * ---------------------------------------------------
	 */

	/*
	 * Implementation must return true if calibration
	 * completes synchronously, and false if not.
	 */
	bool (*StartZero)(uint8_t sampleCount, float pauseMS);

	/*
	 * This is called once calibration is completed, it must not do any
	 * work besides updating the state variables on the sensor and
	 * queueing up a write to the EEPROM.
	 */
	iOpResult_e (*CompleteZero)(u16_vector_t *cals);

	/*
	 * Update Setting is called upon an address and a value
	 * coming over telemetry into the sensor.  This will
	 * always be called when the sensors are in a paused state
	 * upon completion functionality will call "Resume" on the
	 * sensor so any new values can be written to the peripheral
	 * and immediately resetarted.
	 */
	iOpResult_e (*UpdateSetting)(uint16_t addr, uint8_t value);

	/*
	 * This method should be populated for each sensor to send a specific set
	 * of diagnostics information from the sensor through the telemetry.
	 */
	void (*SendDiag)(void);

	/*
	 * Processed X, Y and Z data for the sensor.
	 */
	float X;
	float Y;
	float Z;

	/*
	 * Some sensors will contain extended data, this is a pointer
	 * that can be used to hold a reference to a custom sensors
	 * data, an example of this is gryro data that contains
	 * information about gyro rate.
	 */
	void *Extended;

	/*
	 * The sensors are implemented as a LinkedList,
	 * this simply points to the next item in the list.
	 */
	void *Next;


} Sensor_t;


#define CALIBRATION_POINT_COUNT 20
#define CALIBRATION_WAIT_MS 100

extern Sensor_t *SensorsList;

extern Sensor_t *GyroITG3200Sensor;
extern Sensor_t *GyroL3GD20Sensor;

extern Sensor_t *GyroMPU60x0Sensor;
extern Sensor_t *AccMPU60x0Sensor;

extern Sensor_t *AccADXL345Sensor;
extern Sensor_t *AccLSM303Sensor;

extern Sensor_t *MagHMC5883Sensor;
extern Sensor_t *MagLSM303Sensor;

extern Sensor_t *SonarSensor;
extern Sensor_t *PressureBMP085Sensor;
extern Sensor_t *PressureMS5611Sensor;

extern Sensor_t *AltGPSSensor;
extern Sensor_t *HeadingGPSSensor;
extern Sensor_t *GeoGPSSensor;

extern Sensor_t *GyroMPU60x0Sensor;
extern Sensor_t *AccMPU60x0Sensor;

extern Sensor_t *ADCLipoSensor;

extern Sensor_t *ComplementarySnsrFusion;
extern Sensor_t *KalmanSnsrFusion;

extern Sensor_t *PIDControllerAction;
extern Sensor_t *AutoPilotAction;

I2CJob_t *I2CZeroJob;


#endif /* SNSR_MAIN_H_ */
