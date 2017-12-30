using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class ConfigBase
    {
        protected const ushort ITG3200_EEPROM_OFFSET = 0x0020;
        protected const ushort LSM303_ACC_EEPROM_OFFSET = 0x0060;
        protected const ushort LSM303_MAG_EEPROM_OFFSET = 0x00A0;
        protected const ushort L3GD_EEPROM_OFFSET = 0x00E0;
        protected const ushort BMP085_EEPROM_OFFSET = 0x0120;
        protected const ushort ADXL_EEPROM_OFFSET = 0x0160;
        protected const ushort HMC5883_EEPROM_OFFSET = 0x01A0;
        protected const ushort HCSR04_EEPROM_OFFSET = 0x01E0;
        protected const ushort LIPO_ADC_EEPROM_OFFSET = 0x0220;
        protected const ushort MPU60x0_GYRO_EEPROM_OFFSET = 0x0260;
        protected const ushort MPU60x0_ACC_EEPROM_OFFSET = 0x02A0;
        protected const ushort GPS_HEADING_EEPROM_OFFSET = 0x02E0;
        protected const ushort GPS_ALT_EEPROM_OFFSET = 0x0320;
        protected const ushort GPS_GEO_EEPROM_OFFSET = 0x0360;
        protected const ushort KALMAN_EEPROM_OFFSET = 0x03A0;
        protected const ushort COMPLEMENTARY_EEPROM_OFFSET = 0x03E0;

        protected const ushort SENSOR_CFG_EEPROM_OFFSET = 0x0600;
        protected const ushort DATA_TX_INTVL_EEPROM_OFFSET = 0x0800;

        public const byte MeasureTypeNone = 0;
        public const byte MeasureTypePitch = 10;
        public const byte MeasureTypeRoll = 11;
        public const byte MeasureTypeHeading = 12;
        public const byte MeasureTypeAltitude = 13;

        public class SettingsValue
        {
            public byte Value { get; set; }
            public String Display { get; set; }
        }

        public class SensorDefinition : SettingsValue
        {
            public enum SensorTypes
            {
                None,
                Gyro,
                Accelerometer,
                Magnetometer,
                Altitude,
                ADC,
                GeoLocation,
            }

            public SensorTypes SensorType { get; set; }
        }

        ObservableCollection<SettingsValue> _measureTypes = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() {Display="None", Value=MeasureTypeNone},
            new SettingsValue() {Display="Pitch", Value=MeasureTypePitch},
            new SettingsValue() {Display="Roll", Value=MeasureTypeRoll},
            new SettingsValue() {Display="Heading", Value=MeasureTypeHeading},
            new SettingsValue() {Display="Altitude", Value=MeasureTypeAltitude},
        };

        public ObservableCollection<SettingsValue> MeasureTypes { get { return _measureTypes; } }

        ObservableCollection<SensorDefinition> _allSensors = new ObservableCollection<SensorDefinition>()
            {
                new SensorDefinition() { Display ="None", Value=Sensors.None, SensorType = SensorDefinition.SensorTypes.Gyro },
                new SensorDefinition() { Display ="ITG3200", Value=Sensors.ITG3200, SensorType = SensorDefinition.SensorTypes.Gyro },
                new SensorDefinition() { Display ="L3GD20", Value=Sensors.L3GD20, SensorType = SensorDefinition.SensorTypes.Gyro },
                new SensorDefinition() { Display ="GyroMPU60x0", Value=Sensors.MPU60x0GYRO, SensorType = SensorDefinition.SensorTypes.Gyro },
                new SensorDefinition() { Display ="AccLSM303", Value=Sensors.LSM303_ACC, SensorType = SensorDefinition.SensorTypes.Accelerometer },
                new SensorDefinition() { Display ="AccADXL345", Value=Sensors.ADXL345, SensorType = SensorDefinition.SensorTypes.Accelerometer},
                new SensorDefinition() { Display ="AccMPU60x0", Value=Sensors.MPU60x0ACC, SensorType = SensorDefinition.SensorTypes.Accelerometer },
                new SensorDefinition() { Display ="MagLSM303", Value=Sensors.LSM303_MAG, SensorType = SensorDefinition.SensorTypes.Magnetometer },
                new SensorDefinition() { Display ="MagHMS5883", Value=Sensors.HMC5883, SensorType = SensorDefinition.SensorTypes.Magnetometer },
                new SensorDefinition() { Display ="MagGPS", Value=Sensors.MagGPS, SensorType = SensorDefinition.SensorTypes.Magnetometer },
                new SensorDefinition() { Display ="AltBMP085", Value=Sensors.BMP085,SensorType = SensorDefinition.SensorTypes.Altitude },
                new SensorDefinition() { Display ="AltHCSR04", Value=Sensors.HCSR04,SensorType = SensorDefinition.SensorTypes.Altitude },
                new SensorDefinition() { Display ="AltGPS", Value=Sensors.AltGPS, SensorType = SensorDefinition.SensorTypes.Altitude },
                new SensorDefinition() { Display ="ADCLipo", Value=Sensors.LIPO_ADC, SensorType = SensorDefinition.SensorTypes.ADC },
                new SensorDefinition() { Display ="GeoGPS", Value=Sensors.GeoGPS, SensorType = SensorDefinition.SensorTypes.GeoLocation },
            };

        public ObservableCollection<SensorDefinition> AllSensors { get { return _allSensors; } }

        ObservableCollection<SettingsValue> _compFilterCollection = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() {Display="1 Sec", Value=0},
            new SettingsValue() {Display="0.9 Sec", Value=1},
            new SettingsValue() {Display="0.8 Sec", Value=2},
            new SettingsValue() {Display="0.7 Sec", Value=3},
            new SettingsValue() {Display="0.6 Sec", Value=4},
            new SettingsValue() {Display="0.5 Sec", Value=5},
            new SettingsValue() {Display="0.4 Sec", Value=6},
            new SettingsValue() {Display="0.3 Sec", Value=7},
            new SettingsValue() {Display="0.2 Sec", Value=8},
            new SettingsValue() {Display="0.15 Sec", Value=9},
            new SettingsValue() {Display="0.1 Sec", Value=10},
            new SettingsValue() {Display="50ms", Value=11},            
            new SettingsValue() {Display="25ms", Value=12},
            new SettingsValue() {Display="20ms", Value=13},
            new SettingsValue() {Display="15ms", Value=14},
            new SettingsValue() {Display="10ms", Value=15},
            new SettingsValue() {Display="5ms", Value=16},
            new SettingsValue() {Display="2ms", Value=17},
            new SettingsValue() {Display="1ms", Value=18},
        };

        public ObservableCollection<SettingsValue> CompFilterCollection { get { return _compFilterCollection; } }


        ObservableCollection<SettingsValue> _sampleRateCollection = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() { Display = "SampleRate 1KHz", Value = 0},
            new SettingsValue() { Display = "SampleRate 500Hz", Value = 1},
            new SettingsValue() { Display = "SampleRate 200Hz", Value = 2},
            new SettingsValue() { Display = "SampleRate 150Hz", Value = 3},
            new SettingsValue() { Display = "SampleRate 100Hz", Value = 4},
            new SettingsValue() { Display = "SampleRate 50Hz", Value = 5 },
            new SettingsValue() { Display = "SampleRate 25Hz", Value = 6 },
            new SettingsValue() { Display = "SampleRate 20Hz", Value = 7 },
            new SettingsValue() { Display = "SampleRate 10Hz", Value = 8 },
            new SettingsValue() { Display = "SampleRate 5Hz", Value = 9 },
            new SettingsValue() { Display = "SampleRate 1Hz", Value = 10 },
            new SettingsValue() { Display = "IRQ Driven", Value = 0xFF },
        };

        public ObservableCollection<SettingsValue> SampleRateCollection { get { return _sampleRateCollection; } }

        #region Freq List
        ObservableCollection<SettingsValue> _medianFilterCollection = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() { Display ="None", Value=0 },
            new SettingsValue() { Display ="Median - 3 Samples", Value=1 },
            new SettingsValue() { Display ="Median - 5 Samples", Value=2 },
            new SettingsValue() { Display ="Median - 7 Samples", Value=3 },
            new SettingsValue() { Display ="Median - 9 Samples", Value=4 },
        };

        public ObservableCollection<SettingsValue> MedianFilterCollection { get { return _medianFilterCollection; } }

        ObservableCollection<SettingsValue> _movingAverageCollection = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() { Display ="None", Value=0 },
            new SettingsValue() { Display ="Mov. Avg. .01%", Value=0x10 },
            new SettingsValue() { Display ="Mov. Avg. .1%", Value=0x20 },
            new SettingsValue() { Display ="Mov. Avg. 1%", Value=0x30 },
            new SettingsValue() { Display ="Mov. Avg. 2%", Value=0x40 },
            new SettingsValue() { Display ="Mov. Avg. 5%", Value=0x50 },
            new SettingsValue() { Display ="Mov. Avg. 10%", Value=0x60 },
            new SettingsValue() { Display ="Mov. Avg. 15%", Value=0x70 },
            new SettingsValue() { Display ="Mov. Avg. 20%", Value=0x80 },
            new SettingsValue() { Display ="Mov. Avg. 30%", Value=0x90 },
            new SettingsValue() { Display ="Mov. Avg. 40%", Value=0xA0 },
            new SettingsValue() { Display ="Mov. Avg. 50%", Value=0xB0 },
            new SettingsValue() { Display ="Mov. Avg. 60%", Value=0xC0 },
            new SettingsValue() { Display ="Mov. Avg. 70%", Value=0xD0 },
            new SettingsValue() { Display ="Mov. Avg. 80%", Value=0xE0 },
            new SettingsValue() { Display ="Mov. Avg. 90%", Value=0xF0 },
        };

        public ObservableCollection<SettingsValue> MovingAverageCollection { get { return _movingAverageCollection; } }
        #endregion


        ObservableCollection<SettingsValue> _ctrlLoopRateCollection = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() { Display ="400 Hz", Value=0 },
            new SettingsValue() { Display ="300 Hz", Value=1 },
            new SettingsValue() { Display ="250 Hz", Value=2 },
            new SettingsValue() { Display ="200 Hz", Value=3 },
            new SettingsValue() { Display ="150 Hz", Value=4 },
            new SettingsValue() { Display ="125 Hz", Value=5 },
            new SettingsValue() { Display ="100 Hz", Value=6 },
            new SettingsValue() { Display ="75 Hz", Value=7 },
            new SettingsValue() { Display ="50 Hz", Value=9 },
            new SettingsValue() { Display ="40 Hz", Value=9 },
            new SettingsValue() { Display ="30 Hz", Value=10 },
            new SettingsValue() { Display ="25 Hz", Value=11 },
            new SettingsValue() { Display ="20 Hz", Value=12 },
            new SettingsValue() { Display ="15 Hz", Value=13 },
            new SettingsValue() { Display ="10 Hz", Value=14 },
            new SettingsValue() { Display ="5 Hz", Value=15 },
            new SettingsValue() { Display ="1 Hz", Value=16 },
        };
        public ObservableCollection<SettingsValue> CtrlLoopCollection { get { return _ctrlLoopRateCollection; } }

        public void UpdateApplied(int p)
        {
            Debug.WriteLine("Updated applied for address {0}", p);
        }

    }
}
