using NiVek.Common.Comms;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace NiVek.Common.Models
{
    [ModuleConfig(MessageId = SensorModule.CMD_SetCfg_Value, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor)]
    public class SensorConfig : ConfigBase, IModel, INotifyPropertyChanged
    {
        public SensorConfig()
        {

        }

        public enum CalibrationStatus
        {
            Calibrated = 0xFF,
            NotCalibrated = 0x00
        };

        #region Constants to define EEPROM memory location

        const ushort ADXL_ENABLED = ADXL_EEPROM_OFFSET + 6;
        const ushort ADXL_BW_RATE = ADXL_EEPROM_OFFSET + 7;
        const ushort ADXL_DATA_FORMAT = ADXL_EEPROM_OFFSET + 8;
        const ushort ADXL_FIFO_ENABLE = ADXL_EEPROM_OFFSET + 9;
        const ushort ADXL_FILTER_OPTION = ADXL_EEPROM_OFFSET + 10;
        const ushort ADXL_SAMPLE_RATE = ADXL_EEPROM_OFFSET + 11;

        const ushort BMP085_ENABLED = BMP085_EEPROM_OFFSET + 0;
        const ushort BMP085_NUMBER_SAMPLES = BMP085_EEPROM_OFFSET + 1;
        const ushort BMP085_OVERSAMPLING_RATE = BMP085_EEPROM_OFFSET + 2;
        const ushort BMP085_FILTER_OPTION = BMP085_EEPROM_OFFSET + 3;
        const ushort BMP085_SAMPLE_RATE = BMP085_EEPROM_OFFSET + 4;

        const ushort MS5611_ENABLED = MS5611_EEPROM_OFFSET + 0;
        const ushort MS5611_OVERSAMPLING_RATE = MS5611_EEPROM_OFFSET + 1;
        const ushort MS5611_FILTER_OPTION = MS5611_EEPROM_OFFSET + 2;
        const ushort MS5611_SAMPLE_RATE = MS5611_EEPROM_OFFSET + 3;

        const ushort SONAR_ENABLE = SONAR_EEPROM_OFFSET + 0;
        const ushort SONAR_SENSOR_TYPE = SONAR_EEPROM_OFFSET + 1;
        const ushort SONAR_FILTER_TYPE = SONAR_EEPROM_OFFSET + 2;
        const ushort SONAR_SAMPLE_RATE = SONAR_EEPROM_OFFSET + 3;

        const ushort HMC5883_ENABLED = HMC5883_EEPROM_OFFSET + 13;
        const ushort HMC5883_CRA = HMC5883_EEPROM_OFFSET + 14;
        const ushort HMC5883_CRB = HMC5883_EEPROM_OFFSET + 15;
        const ushort HMC5883_MODE = HMC5883_EEPROM_OFFSET + 16;
        const ushort HMC5883_FILTER_OPTION = HMC5883_EEPROM_OFFSET + 17;
        const ushort HMC5883_SAMPLE_RATE = HMC5883_EEPROM_OFFSET + 18;

        const ushort ITG3200_ENABLED = ITG3200_EEPROM_OFFSET + 6;
        const ushort ITG3200_SMPL_DLPF_FS = ITG3200_EEPROM_OFFSET + 7;
        const ushort ITG3200_SMPL_RT_DIV = ITG3200_EEPROM_OFFSET + 8;
        const ushort ITG3200_FILTER_OPTION = ITG3200_EEPROM_OFFSET + 9;
        const ushort ITG3200_SAMPLE_RATE = ITG3200_EEPROM_OFFSET + 10;

        const ushort MPU60x0_GYRO_ENABLED = MPU60x0_GYRO_EEPROM_OFFSET + 6;
        const ushort MPU60x0_SMPLRT_DIV = MPU60x0_GYRO_EEPROM_OFFSET + 7;
        const ushort MPU60x0_GYRO_FS = MPU60x0_GYRO_EEPROM_OFFSET + 8;
        const ushort MPU60x0_FIFO_ENABLE = MPU60x0_GYRO_EEPROM_OFFSET + 9;
        const ushort MPU60x0_GYRO_FILTER_TYPE = MPU60x0_GYRO_EEPROM_OFFSET + 10;
        const ushort MPU60x0_GYRO_SAMPLE_RATE = MPU60x0_GYRO_EEPROM_OFFSET + 11;

        const ushort MPU60x0_ACC_ENABLED = MPU60x0_ACC_EEPROM_OFFSET + 6;
        const ushort MPU60x0_ACC_FS = MPU60x0_ACC_EEPROM_OFFSET + 7;
        const ushort MPU60x0_DLPF = MPU60x0_ACC_EEPROM_OFFSET + 8;
        const ushort MPU60x0_ACC_FILTER_TYPE = MPU60x0_ACC_EEPROM_OFFSET + 9;
        const ushort MPU60x0_ACC_SAMPLE_RATE = MPU60x0_ACC_EEPROM_OFFSET + 10;

        const ushort L3GD20_ENABLED = L3GD_EEPROM_OFFSET + 6;
        const ushort L3GD20_CTRL_REG1 = L3GD_EEPROM_OFFSET + 7;
        const ushort L3GD20_CTRL_REG2 = L3GD_EEPROM_OFFSET + 8;
        const ushort L3GD20_CTRL_REG3 = L3GD_EEPROM_OFFSET + 9;
        const ushort L3GD20_CTRL_REG4 = L3GD_EEPROM_OFFSET + 10;
        const ushort L3GD20_CTRL_REG5 = L3GD_EEPROM_OFFSET + 11;
        const ushort L3GD20_FIFO_ENABLE = L3GD_EEPROM_OFFSET + 12;
        const ushort L3GD20_FILTER_OPTION = L3GD_EEPROM_OFFSET + 13;
        const ushort L3GD20_SAMPLE_RATE = L3GD_EEPROM_OFFSET + 14;

        const ushort LSM303_ACC_Enabled = LSM303_ACC_EEPROM_OFFSET + 6;
        const ushort LSM303_ACC_REG1 = LSM303_ACC_EEPROM_OFFSET + 7;
        const ushort LSM303_ACC_REG2 = LSM303_ACC_EEPROM_OFFSET + 8;
        const ushort LSM303_ACC_REG3 = LSM303_ACC_EEPROM_OFFSET + 9;
        const ushort LSM303_ACC_REG4 = LSM303_ACC_EEPROM_OFFSET + 10;
        const ushort LSM303_ACC_FIFO_ENABLE = LSM303_ACC_EEPROM_OFFSET + 11;
        const ushort LSM303_ACC_FILTER_OPTION = LSM303_ACC_EEPROM_OFFSET + 12;
        const ushort LSM303_ACC_SAMPLE_RATE = LSM303_ACC_EEPROM_OFFSET + 13;

        const ushort LSM303_MAG_Enabled = LSM303_MAG_EEPROM_OFFSET + 13;
        const ushort LSM303_MAG_CRA_REG = LSM303_MAG_EEPROM_OFFSET + 14;
        const ushort LSM303_MAG_CRB_REG = LSM303_MAG_EEPROM_OFFSET + 15;
        const ushort LSM303_MAG_MR_REG = LSM303_MAG_EEPROM_OFFSET + 16;
        const ushort LSM303_MAG_FILTER_OPTION = LSM303_MAG_EEPROM_OFFSET + 17;
        const ushort LSM303_MAG_SAMPLE_RATE = LSM303_MAG_EEPROM_OFFSET + 18;

        const ushort SNSR_SAMPLE_RATE = SENSOR_CFG_EEPROM_OFFSET;
        const ushort SNSR_ACC_CMPLT_TIME_BASE = SENSOR_CFG_EEPROM_OFFSET + 1;
        const ushort SNSR_GYRO_TYPE = SENSOR_CFG_EEPROM_OFFSET + 2;
        const ushort SNSR_ACC_TYPE = SENSOR_CFG_EEPROM_OFFSET + 3;
        const ushort SNSR_MAG_TYPE = SENSOR_CFG_EEPROM_OFFSET + 4;

        const ushort LIPO_ADC_FILTER_ENABLED = LIPO_ADC_EEPROM_OFFSET + 0;
        const ushort LIPO_ADC_FILTER_TYPE = LIPO_ADC_EEPROM_OFFSET + 1;
        const ushort LIPO_ADC_SAMPLE_RATE = LIPO_ADC_EEPROM_OFFSET + 2;

        const ushort GPS_DGPS_Type = GPS_GEO_EEPROM_OFFSET + 0;

        const ushort DATA_UPDATE_RATE_SENSOR = DATA_TX_INTVL_EEPROM_OFFSET + 0;
        const ushort DATA_UPDATE_RATE_GPS = DATA_TX_INTVL_EEPROM_OFFSET + 1;
        const ushort DATA_UPDATE_RATE_BATTERY = DATA_TX_INTVL_EEPROM_OFFSET + 2;
        const ushort DATA_UPDATE_RATE_SYSTEM = DATA_TX_INTVL_EEPROM_OFFSET + 3;
        const ushort DATA_UPDATE_RATE_TARGET = DATA_TX_INTVL_EEPROM_OFFSET + 4;
        const ushort DATA_UPDATE_RATE_MOTORS = DATA_TX_INTVL_EEPROM_OFFSET + 5;
        const ushort DATA_UPDATE_RATE_ALTITUDE = DATA_TX_INTVL_EEPROM_OFFSET + 6;

        #endregion

        #region Create an instance from a buffer

        private void Populate(byte[] buffer)
        {
            int idx = 0;
            _ctrlLoopRate = buffer[idx++];

            _statusSendDataRate = buffer[idx++];
            _gpsSendDataRate = buffer[idx++];
            _batterySendDataRate = buffer[idx++];
            _sensorSendDataRate = buffer[idx++];
            _targetSendDataRate = buffer[idx++];
            _motorSendDataRate = buffer[idx++];
            _altitudeSendDataRate = buffer[idx++];

            _itg3200Enabled = buffer[idx++];
            _itg3200DlpFs = buffer[idx++];
            _itg3200SmplRtDiv = buffer[idx++];
            _itg3200FilterOption = buffer[idx++];
            _itg3200SampleRate = buffer[idx++];

            _mpu60x0GyroEnabled = buffer[idx++];
            _mpu60x0AccEnabled = buffer[idx++];

            _mpu60x0Dlpf = buffer[idx++];
            _mpu60x0SmplRtDiv = buffer[idx++];
            _mpu60x0GyroFS = buffer[idx++];
            _mpu60x0AccFS = buffer[idx++];
            _mpu60x0FifoEnabled = buffer[idx++];

            _mpu60x0GyroFilterType = buffer[idx++];
            _mpu60x0AccFilterType = buffer[idx++];
            _mpu60x0GyroSmplRt = buffer[idx++];
            _mpu60x0AccSmplRt = buffer[idx++];

            _lsm303AccEnabled = buffer[idx++];
            _lsm303AccReg1 = buffer[idx++];
            _lsm303AccReg2 = buffer[idx++];
            _lsm303AccReg3 = buffer[idx++];
            _lsm303AccReg4 = buffer[idx++];
            _lsm303AccFiFoEnable = buffer[idx++];
            _lsm303AccFilterOption = buffer[idx++];
            _lsm303AccSampleRate = buffer[idx++];

            _lsm303MagEnabled = buffer[idx++];
            _lsm303MagCraReg = buffer[idx++];
            _lsm303MagCrbReg = buffer[idx++];
            _lsm303MagMRReg = buffer[idx++];
            _lsm303MagFilterOption = buffer[idx++];
            _lsm303MagSampleRate = buffer[idx++];
            _lsm303MagCalibrated = (CalibrationStatus)buffer[idx++];

            _l3gd20Enabled = buffer[idx++];
            _l3gd20CtrlReg1 = buffer[idx++];
            _l3gd20CtrlReg2 = buffer[idx++];
            _l3gd20CtrlReg3 = buffer[idx++];
            _l3gd20CtrlReg4 = buffer[idx++];
            _l3gd20CtrlReg5 = buffer[idx++];
            _l3gd20FifoEnable = buffer[idx++];
            _l3gd20FilterOption = buffer[idx++];
            _l3gd20SampleRate = buffer[idx++];

            _adxl345Enabled = buffer[idx++];
            _adxl345Rate = buffer[idx++];
            _adxl345DataFormat = buffer[idx++];
            _adxl345FiFoEnabled = buffer[idx++];
            _adxl345FilterOption = buffer[idx++];
            _adxl345SampleRate = buffer[idx++];

            _hmc5883Enabled = buffer[idx++];
            _hmc5883CRA = buffer[idx++];
            _hmc5883CRB = buffer[idx++];
            _hmc5883Mode = buffer[idx++];
            _hmc5883FilterOption = buffer[idx++];
            _hmc5883SampleRate = buffer[idx++];
            _hmc5883Calibrated = (CalibrationStatus)buffer[idx++];

            _sonarEnabled = buffer[idx++];
            _sonarSensorType = buffer[idx++];
            _sonarFilterOption = buffer[idx++];
            _sonarSampleRate = buffer[idx++];

            _bmp085Enabled = buffer[idx++];
            _bmp085NumberSamples = buffer[idx++];
            _bmp085OversamplingRate = buffer[idx++];
            _bmp085FilterOption = buffer[idx++];
            _bmp085SampleRate = buffer[idx++];

            _ms5611Enabled = buffer[idx++];
            _ms5611OversamplingRate = buffer[idx++];
            _ms5611FilterOption = buffer[idx++];
            _ms5611SampleRate = buffer[idx++];

            _gpsDGPSType = buffer[idx];

            _lipoAdcEnabled = buffer[idx++];
            _lipoAdcFilterType = buffer[idx++];
            _lipoAdcSampleRate = buffer[idx++];
        }

        public SensorConfig(Drone drone, byte[] buffer)
        {
            this.Drone = drone;
            Populate(buffer);
        }
        public void Update(BufferReader reader)
        {
            var buffer = reader.GetRawBuffer();
            Populate(buffer);
        }
        #endregion

        #region Update Rates
        byte _statusSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_SYSTEM)]
        public byte StatusSendDataRate
        {
            get { return _statusSendDataRate; }
            set { Set(value, ref _statusSendDataRate, () => StatusSendDataRate); }
        }

        byte _sensorSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_SENSOR)]
        public byte SensorSendDataRate
        {
            get { return _sensorSendDataRate; }
            set { Set(value, ref _sensorSendDataRate, () => SensorSendDataRate); }
        }

        byte _gpsSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_GPS)]
        public byte GPSSendDataRate
        {
            get { return _gpsSendDataRate; }
            set { Set(value, ref _gpsSendDataRate, () => GPSSendDataRate); }
        }


        byte _batterySendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_BATTERY)]
        public byte BatterySendDataRate
        {
            get { return _batterySendDataRate; }
            set { Set(value, ref _batterySendDataRate, () => BatterySendDataRate); }
        }

        byte _targetSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_TARGET)]

        public byte TargetSendDataRate
        {
            get { return _targetSendDataRate; }
            set { Set(value, ref _targetSendDataRate, () => TargetSendDataRate); }
        }

        byte _motorSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_MOTORS)]
        public byte MotorSendDataRate
        {
            get { return _motorSendDataRate; }
            set { Set(value, ref _motorSendDataRate, () => MotorSendDataRate); }
        }

        byte _altitudeSendDataRate;
        [DeviceSetting(SettingAddress = DATA_UPDATE_RATE_ALTITUDE)]
        public byte AltitudeSendDataRate
        {
            get { return _altitudeSendDataRate; }
            set { Set(value, ref _altitudeSendDataRate, () => AltitudeSendDataRate); }
        }
        #endregion

        #region Master Configuration
        public byte _ctrlLoopRate;
        [DeviceSetting(SettingAddress = SNSR_SAMPLE_RATE)]
        public byte ControlLoopRate
        {
            get { return _ctrlLoopRate; }
            set { Set(value, ref _ctrlLoopRate, () => ControlLoopRate); }
        }
        #endregion

        #region ADXL 345
        byte _adxl345Enabled;
        [DeviceSetting(SettingAddress = ADXL_ENABLED)]
        public bool ADXL345Enabled
        {
            get { return _adxl345Enabled != 0; }
            set { Set(value, ref _adxl345Enabled, () => ADXL345Enabled); }
        }

        byte _adxl345FiFoEnabled;
        [DeviceSetting(SettingAddress = ADXL_FIFO_ENABLE)]
        public bool ADXL345FiFoEnabled
        {
            get { return _adxl345FiFoEnabled != 0; }
            set { Set(value, ref _adxl345FiFoEnabled, () => ADXL345FiFoEnabled); }
        }


        byte _adxl345SampleRate;
        [DeviceSetting(SettingAddress = ADXL_SAMPLE_RATE)]
        public byte ADXL345SampleRate
        {
            get { return _adxl345SampleRate; }
            set { Set(value, ref _adxl345SampleRate, () => ADXL345SampleRate); }
        }

        byte _adxl345DataFormat;
        [DeviceSetting(SettingAddress = ADXL_DATA_FORMAT)]
        public byte ADXL345DataFormat
        {
            get { return _adxl345DataFormat; }
            set { Set(value, ref _adxl345DataFormat, () => ADXL345DataFormat); }
        }

        ObservableCollection<SettingsValue> _adxl345GainCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 2g", Value = 0}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="+/- 4g", Value = 0x01},
                new SettingsValue() { Display ="+/- 8g", Value = 0x2},
                new SettingsValue() { Display ="+/- 16g", Value = 0x3},
            };
        public ObservableCollection<SettingsValue> ADXL345GainCollection { get { return _adxl345GainCollection; } }

        ObservableCollection<SettingsValue> _adxl345ResolutionCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="Full Range", Value = 0}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="10 Bit", Value = 0x01 << 3},
            };
        public ObservableCollection<SettingsValue> ADXLResolutionCollection { get { return _adxl345ResolutionCollection; } }


        ObservableCollection<SettingsValue> _adxl345DataRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="3200 Hz", Value = 0xF}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="1600 Hz", Value = 0xE},
                new SettingsValue() { Display ="800 Hz", Value = 0xD},
                new SettingsValue() { Display ="400 Hz", Value = 0xC},
                new SettingsValue() { Display ="200 Hz", Value = 0xB},
                new SettingsValue() { Display ="100 Hz", Value = 0xA},
                new SettingsValue() { Display ="50 Hz", Value = 0x9},
                new SettingsValue() { Display ="25 Hz", Value = 0x8},
                new SettingsValue() { Display ="12.5 Hz", Value = 0x7},
                new SettingsValue() { Display ="6.25 Hz", Value = 0x6},
            };

        public ObservableCollection<SettingsValue> ADXL345DataRateCollection { get { return _adxl345DataRateCollection; } }

        byte _adxl345Rate;
        [DeviceSetting(SettingAddress = ADXL_BW_RATE)]
        public byte ADXL345BWRate
        {
            get { return _adxl345Rate; }
            set { Set(value, ref _adxl345Rate, () => ADXL345BWRate); }
        }

        byte _adxl345FilterOption;
        [DeviceSetting(SettingAddress = ADXL_FILTER_OPTION)]
        public byte ADXL345FilterOption
        {
            get { return _adxl345FilterOption; }
            set { Set(value, ref _adxl345FilterOption, () => ADXL345FilterOption); }
        }

        public byte ADXL345MedianFilterOption
        {
            get { return (byte)(_adxl345FilterOption & 0x0F); }
            set { ADXL345FilterOption = (byte)((_adxl345FilterOption & 0xf0) | value); }
        }

        public byte ADXL345MovingAverageFilterOption
        {
            get { return (byte)(_adxl345FilterOption & 0xF0); }
            set { ADXL345FilterOption = (byte)((_adxl345FilterOption & 0x0F) | value); }
        }

        public byte ADXLResolution
        {
            get { return (byte)(_adxl345DataFormat & 0x04); }
            set
            {
                byte tmp = (byte)(~(0x01 << 3) & ADXL345DataFormat);
                ADXL345DataFormat = (byte)(tmp | value);
            }
        }

        public byte ADXLRange
        {
            get { return (byte)(ADXL345DataFormat & 0x03); }
            set
            {
                byte tmp = (byte)(~0x03 & ADXL345DataFormat);
                ADXL345DataFormat = (byte)(tmp | value);
            }
        }

        #endregion

        #region BMP085
        byte _bmp085Enabled;
        [DeviceSetting(SettingAddress = BMP085_ENABLED)]
        public bool BMP085Enabled
        {
            get { return _bmp085Enabled != 0; }
            set { Set(value, ref _bmp085Enabled, () => BMP085Enabled); }
        }

        byte _bmp085SampleRate;
        [DeviceSetting(SettingAddress = BMP085_SAMPLE_RATE)]
        public byte BMP085SampleRate
        {
            get { return _bmp085SampleRate; }
            set { Set(value, ref _bmp085SampleRate, () => BMP085SampleRate); }
        }

        private byte _bmp085NumberSamples;
        [DeviceSetting(SettingAddress = BMP085_NUMBER_SAMPLES)]
        public byte BMP085NumberSamples
        {
            get { return _bmp085NumberSamples; }
            set { Set(value, ref _bmp085NumberSamples, () => BMP085NumberSamples); }
        }

        private byte _bmp085OversamplingRate;
        [DeviceSetting(SettingAddress = BMP085_OVERSAMPLING_RATE)]
        public byte BMP085OverSamplingRate
        {
            get { return _bmp085OversamplingRate; }
            set { Set(value, ref _bmp085OversamplingRate, () => BMP085OverSamplingRate); }
        }

        private byte _bmp085FilterOption;
        [DeviceSetting(SettingAddress = BMP085_FILTER_OPTION)]
        public byte BMP085FilterOption
        {
            get { return _bmp085FilterOption; }
            set { Set(value, ref _bmp085FilterOption, () => BMP085FilterOption); }
        }

        public byte BMP085MedianFilterOption
        {
            get { return (byte)(_bmp085FilterOption & 0x0F); }
            set { BMP085FilterOption = (byte)((_bmp085FilterOption & 0xf0) | value); }
        }

        public byte BMP085MovingAverageFilterOption
        {
            get { return (byte)(_bmp085FilterOption & 0xF0); }
            set { BMP085FilterOption = (byte)((_bmp085FilterOption & 0x0F) | value); }
        }
        #endregion

        #region MS5611 (Pressure)
        byte _ms5611Enabled;
        [DeviceSetting(SettingAddress = MS5611_ENABLED)]
        public bool MS5611Enabled
        {
            get { return _ms5611Enabled != 0; }
            set { Set(value, ref _ms5611Enabled, () => MS5611Enabled); }
        }

        byte _ms5611SampleRate;
        [DeviceSetting(SettingAddress = MS5611_SAMPLE_RATE)]
        public byte MS5611SampleRate
        {
            get { return _ms5611SampleRate; }
            set { Set(value, ref _ms5611SampleRate, () => MS5611SampleRate); }
        }

        private byte _ms5611OversamplingRate;
        [DeviceSetting(SettingAddress = MS5611_OVERSAMPLING_RATE)]
        public byte MS5611OverSamplingRate
        {
            get { return _ms5611OversamplingRate; }
            set { Set(value, ref _ms5611OversamplingRate, () => MS5611OverSamplingRate); }
        }

        private byte _ms5611FilterOption;
        [DeviceSetting(SettingAddress = MS5611_FILTER_OPTION)]
        public byte MS5611FilterOption
        {
            get { return _ms5611FilterOption; }
            set { Set(value, ref _ms5611FilterOption, () => MS5611FilterOption); }
        }

        public byte MS5611MedianFilterOption
        {
            get { return (byte)(_ms5611FilterOption & 0x0F); }
            set { MS5611FilterOption = (byte)((_ms5611FilterOption & 0xf0) | value); }
        }

        public byte MS5611MovingAverageFilterOption
        {
            get { return (byte)(_ms5611FilterOption & 0xF0); }
            set { MS5611FilterOption = (byte)((_ms5611FilterOption & 0x0F) | value); }
        }
        #endregion

        #region HCSR04
        ObservableCollection<SettingsValue> _sonarTypeCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="None", Value = 0}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="HC-SR04", Value = 0x01},
                new SettingsValue() { Display ="MB1030 EZ", Value = 0x2},
            };
        public ObservableCollection<SettingsValue> SonarTypeCollection { get { return _sonarTypeCollection; } }

        byte _sonarEnabled;
        [DeviceSetting(SettingAddress = SONAR_ENABLE)]
        public bool SonarEnabled
        {
            get { return _sonarEnabled != 0; }
            set { Set(value, ref _sonarEnabled, () => SonarEnabled); }
        }

        byte _sonarSensorType;
        [DeviceSetting(SettingAddress = SONAR_SENSOR_TYPE)]
        public byte SonarSensorType
        {
            get { return _sonarSensorType; }
            set { Set(value, ref _sonarSensorType, () => SonarSensorType); }
        }

        byte _sonarSampleRate;
        [DeviceSetting(SettingAddress = SONAR_SAMPLE_RATE)]
        public byte SonarSampleRate
        {
            get { return _sonarSampleRate; }
            set { Set(value, ref _sonarSampleRate, () => SonarSampleRate); }
        }

        private byte _sonarFilterOption;
        [DeviceSetting(SettingAddress = SONAR_FILTER_TYPE)]
        public byte SonarFilterOption
        {
            get { return _sonarFilterOption; }
            set { Set(value, ref _sonarFilterOption, () => SonarFilterOption); }
        }

        public byte SonarMedianFilterOption
        {
            get { return (byte)(_sonarFilterOption & 0x0F); }
            set { SonarFilterOption = (byte)((_sonarFilterOption & 0xf0) | value); }
        }

        public byte SonarMovingAverageFilterOption
        {
            get { return (byte)(_sonarFilterOption & 0xF0); }
            set { SonarFilterOption = (byte)((_sonarFilterOption & 0x0F) | value); }
        }
        #endregion

        #region HMC5883
        #region Raw Register
        byte _hmc5883Enabled;
        [DeviceSetting(SettingAddress = HMC5883_ENABLED)]
        public bool HMC5883Enabled
        {
            get { return _hmc5883Enabled != 0; }
            set { Set(value, ref _hmc5883Enabled, () => HMC5883Enabled); }
        }

        CalibrationStatus _hmc5883Calibrated;
        public CalibrationStatus HMC5883Calibrated
        {
            get { return _hmc5883Calibrated; }
        }

        byte _hmc5883SampleRate;
        [DeviceSetting(SettingAddress = HMC5883_SAMPLE_RATE)]
        public byte HMC5883SampleRate
        {
            get { return _hmc5883SampleRate; }
            set { Set(value, ref _hmc5883SampleRate, () => HMC5883SampleRate); }
        }

        byte _hmc5883CRA;
        [DeviceSetting(SettingAddress = HMC5883_CRA)]
        public byte HMC5883CRA
        {
            get { return _hmc5883CRA; }
            set { Set(value, ref _hmc5883CRA, () => HMC5883CRA); }
        }

        public byte _hmc5883CRB;
        [DeviceSetting(SettingAddress = HMC5883_CRB)]
        public byte HMC5883CRB
        {
            get { return _hmc5883CRB; }
            set { Set(value, ref _hmc5883CRB, () => HMC5883CRB); }
        }

        public byte _hmc5883Mode;
        [DeviceSetting(SettingAddress = HMC5883_MODE)]
        public byte HMC5883Mode
        {
            get { return _hmc5883Mode; }
            set { Set(value, ref _hmc5883Mode, () => HMC5883Mode); }
        }

        public byte _hmc5883FilterOption;
        [DeviceSetting(SettingAddress = HMC5883_FILTER_OPTION)]
        public byte HMC5883FilterOption
        {
            get { return _hmc5883FilterOption; }
            set { Set(value, ref _hmc5883FilterOption, () => HMC5883FilterOption); }
        }

        public byte HMC5883MedianFilterOption
        {
            get { return (byte)(_hmc5883FilterOption & 0x0F); }
            set { HMC5883FilterOption = (byte)((_hmc5883FilterOption & 0xf0) | value); }
        }

        public byte HMC5883MovingAverageFilterOption
        {
            get { return (byte)(_hmc5883FilterOption & 0xF0); }
            set { HMC5883FilterOption = (byte)((_hmc5883FilterOption & 0x0F) | value); }
        }
        #endregion

        ObservableCollection<SettingsValue> _hmc5883GainCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 0.88 Ga", Value=0x00 << 5 }, //Shifted 5 is DDDxxxxx
                new SettingsValue() { Display ="+/- 1.3 Ga", Value=0x01 << 5 },
                new SettingsValue() { Display ="+/- 1.9 Ga", Value=0x02 << 5 },
                new SettingsValue() { Display ="+/- 2.5 Ga", Value=0x03 << 5 },
                new SettingsValue() { Display ="+/- 4.0 Ga", Value=0x04 << 5 },
                new SettingsValue() { Display ="+/- 4.7 Ga", Value=0x05 << 5 },
                new SettingsValue() { Display ="+/- 5.6 Ga", Value=0x06 << 5 },
                new SettingsValue() { Display ="+/- 8.1 Ga", Value=0x07 << 5 }

            };

        public ObservableCollection<SettingsValue> HMC5883GainCollection { get { return _hmc5883GainCollection; } }

        public byte HMC5883Gain
        {
            get { return (byte)(HMC5883CRB); }
            set { HMC5883CRB = (byte)(value & (0x07 << 5)); }
        }

        ObservableCollection<SettingsValue> _hmc5883DataRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="0.75 Hz", Value=0x00 << 2 },
                new SettingsValue() { Display ="1.5 Hz", Value=0x01 << 2 },
                new SettingsValue() { Display ="3 Hz", Value=0x02 << 2 },
                new SettingsValue() { Display ="7.5 Hz", Value=0x03 << 2 },
                new SettingsValue() { Display ="15 Hz", Value=0x04 << 2 },
                new SettingsValue() { Display ="30 Hz", Value=0x05 << 2 },
                new SettingsValue() { Display ="75 Hz", Value=0x06 << 2 },
            };

        public ObservableCollection<SettingsValue> HMC5883DataRateCollection { get { return _hmc5883DataRateCollection; } }

        public byte HMC5883DataRate
        {
            get { return (byte)(HMC5883CRA & 0x1C); }
            set
            {
                byte tmp = (byte)(HMC5883CRA & ~(0x7 << 2));
                HMC5883CRA = (byte)(tmp | value);
            }
        }

        ObservableCollection<SettingsValue> _hmc5883SamplesAveragedCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="1", Value=0x00 << 5 },
                new SettingsValue() { Display ="2", Value=0x01 << 5 },
                new SettingsValue() { Display ="4", Value=0x02 << 5 },
                new SettingsValue() { Display ="8", Value=0x03 << 5 },
            };

        public ObservableCollection<SettingsValue> HMC5883SamplesAveragedCollection { get { return _hmc5883SamplesAveragedCollection; } }

        public byte HMC5883AverageSamples
        {
            get { return (byte)(HMC5883CRA & 0x60); }
            set
            {
                byte tmp = (byte)(HMC5883CRA & ~(0x02 << 5));
                HMC5883CRA = (byte)(tmp | value);
            }
        }
        #endregion

        #region ITG3200 Gyro
        byte _itg3200Enabled;
        [DeviceSetting(SettingAddress = ITG3200_ENABLED)]
        public bool ITG3200Enabled
        {
            get { return _itg3200Enabled != 0; }
            set { Set(value, ref _itg3200Enabled, () => ITG3200Enabled); }
        }

        byte _itg3200SampleRate;
        [DeviceSetting(SettingAddress = ITG3200_SAMPLE_RATE)]
        public byte ITG3200SampleRate
        {
            get { return _itg3200SampleRate; }
            set { Set(value, ref _itg3200SampleRate, () => ITG3200SampleRate); }
        }


        ObservableCollection<SettingsValue> _itgSampleRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue(){Display ="1 KHz", Value = 0 },
                new SettingsValue(){Display ="500 Hz", Value = 1 },
                new SettingsValue(){Display ="333 Hz", Value = 2 },
                new SettingsValue(){Display ="250 Hz", Value = 3 },
                new SettingsValue(){Display ="200 Hz", Value = 4 },
                new SettingsValue(){Display ="166 Hz", Value = 5},
                new SettingsValue(){Display ="142 Hz", Value = 6},
                new SettingsValue(){Display ="125 Hz", Value = 7},
                new SettingsValue(){Display ="111 Hz", Value = 8},
                new SettingsValue(){Display ="100 Hz", Value = 9},
                new SettingsValue(){Display ="50 Hz", Value = 19},
                new SettingsValue(){Display ="25 Hz", Value = 39},
                new SettingsValue(){Display ="10 Hz", Value = 99},                
            };

        public ObservableCollection<SettingsValue> ITGSampleRateCollection { get { return _itgSampleRateCollection; } }

        private byte _itg3200SmplRtDiv;
        [DeviceSetting(SettingAddress = ITG3200_SMPL_RT_DIV)]
        public byte ITG3200SmplRtDiv
        {
            get { return _itg3200SmplRtDiv; }
            set { Set(value, ref _itg3200SmplRtDiv, () => ITG3200SmplRtDiv); }
        }

        ObservableCollection<SettingsValue> _itg3200LPFCollection = new ObservableCollection<SettingsValue>()
            {
                //new SettingsValue(){Display ="256 Hz", Value = 0x00 | 0x10 },
                new SettingsValue(){Display ="188 Hz", Value = 0x01 | 0x18 }, //If we OR a 0x18 (or 0bXXX11XXXX) into D4, D3, we set the proper full scale value (3 => +/- 2000 d/s
                new SettingsValue(){Display ="98 Hz", Value = 0x02 | 0x18 },
                new SettingsValue(){Display ="42 Hz", Value = 0x03 | 0x18 },
                new SettingsValue(){Display ="20 Hz", Value = 0x04 | 0x18 },
                new SettingsValue(){Display ="10 Hz", Value = 0x05 | 0x18 },
                new SettingsValue(){Display ="5 Hz", Value = 0x06 | 0x18 },
            };

        public ObservableCollection<SettingsValue> ITG3200LPFCollection { get { return _itg3200LPFCollection; } }


        private byte _itg3200DlpFs;
        [DeviceSetting(SettingAddress = ITG3200_SMPL_DLPF_FS)]
        public byte ITG3200DlpFs
        {
            get { return _itg3200DlpFs; }
            set { Set(value, ref _itg3200DlpFs, () => ITG3200DlpFs); }
        }

        private byte _itg3200FilterOption;
        [DeviceSetting(SettingAddress = ITG3200_FILTER_OPTION)]
        public byte ITG3200FilterOption
        {
            get { return _itg3200FilterOption; }
            set { Set(value, ref _itg3200FilterOption, () => ITG3200FilterOption); }
        }

        public byte ITG3200MedianFilterOption
        {
            get { return (byte)(_itg3200FilterOption & 0x0F); }
            set { ITG3200FilterOption = (byte)((_itg3200FilterOption & 0xf0) | value); }
        }

        public byte ITG3200MovingAverageFilterOption
        {
            get { return (byte)(_itg3200FilterOption & 0xF0); }
            set { ITG3200FilterOption = (byte)((_itg3200FilterOption & 0x0F) | value); }
        }
        #endregion

        #region L3GD20 Gyro
        #region Raw Registers
        byte _l3gd20Enabled;

        [DeviceSetting(SettingAddress = L3GD20_ENABLED)]
        public bool L3GD20Enabled
        {
            get { return _l3gd20Enabled != 0; }
            set { Set(value, ref _l3gd20Enabled, () => L3GD20Enabled); }
        }

        byte _l3gd20FifoEnable;
        [DeviceSetting(SettingAddress = L3GD20_FIFO_ENABLE)]
        public bool L3GD20FifoEnable
        {
            get { return _l3gd20FifoEnable != 0; }
            set { Set(value, ref _l3gd20FifoEnable, () => L3GD20FifoEnable); }
        }


        byte _l3gd20SampleRate;
        [DeviceSetting(SettingAddress = L3GD20_SAMPLE_RATE)]
        public byte L3GD20SampleRate
        {
            get { return _l3gd20SampleRate; }
            set { Set(value, ref _l3gd20SampleRate, () => L3GD20SampleRate); }
        }


        byte _l3gd20CtrlReg1;
        [DeviceSetting(SettingAddress = L3GD20_CTRL_REG1)]
        public byte L3GD20CtrlReg1
        {
            get { return _l3gd20CtrlReg1; }
            set { Set(value, ref _l3gd20CtrlReg1, () => L3GD20CtrlReg1); }
        }

        byte _l3gd20CtrlReg2;
        [DeviceSetting(SettingAddress = L3GD20_CTRL_REG2)]
        public byte L3GD20CtrlReg2
        {
            get { return _l3gd20CtrlReg2; }
            set { Set(value, ref _l3gd20CtrlReg2, () => L3GD20CtrlReg2); }
        }

        byte _l3gd20CtrlReg3;
        [DeviceSetting(SettingAddress = L3GD20_CTRL_REG3)]
        public byte L3GD20CtrlReg3
        {
            get { return _l3gd20CtrlReg3; }
            set { Set(value, ref _l3gd20CtrlReg3, () => L3GD20CtrlReg3); }
        }

        byte _l3gd20CtrlReg4;
        [DeviceSetting(SettingAddress = L3GD20_CTRL_REG4)]
        public byte L3GD20CtrlReg4
        {
            get { return _l3gd20CtrlReg4; }
            set { Set(value, ref _l3gd20CtrlReg4, () => L3GD20CtrlReg4); }
        }

        byte _l3gd20CtrlReg5;
        [DeviceSetting(SettingAddress = L3GD20_CTRL_REG5)]
        public byte L3GD20CtrlReg5
        {
            get { return _l3gd20CtrlReg5; }
            set { Set(value, ref _l3gd20CtrlReg5, () => L3GD20CtrlReg5); }
        }

        byte _l3gd20FilterOption;
        [DeviceSetting(SettingAddress = L3GD20_FILTER_OPTION)]
        public byte L3GD20FilterOption
        {
            get { return _l3gd20FilterOption; }
            set { Set(value, ref _l3gd20FilterOption, () => L3GD20FilterOption); }
        }

        public byte L3GD20MedianFilterOption
        {
            get { return (byte)(_l3gd20FilterOption & 0x0F); }
            set { L3GD20FilterOption = (byte)((_l3gd20FilterOption & 0xf0) | value); }
        }

        public byte L3GD20MovingAverageFilterOption
        {
            get { return (byte)(_l3gd20FilterOption & 0xF0); }
            set { L3GD20FilterOption = (byte)((_l3gd20FilterOption & 0x0F) | value); }
        }
        #endregion
        ObservableCollection<SettingsValue> _l3gd20HighPassFilterCuttoffCollection = new ObservableCollection<SettingsValue>()
        {
                new SettingsValue() { Display ="7.2/13.5/27/51.4", Value=0x00},
                new SettingsValue() { Display ="3.5/7.2/13.5/27", Value=0x01},
                new SettingsValue() { Display ="1.8/3.5/7.2/13.5", Value=0x02},
                new SettingsValue() { Display ="0.9/1.8/3.5/7.2", Value=0x03},
                new SettingsValue() { Display ="0.45/0.9/1.8/3.5", Value=0x04},
                new SettingsValue() { Display ="0.18/0.45/0.9/1.8", Value=0x05},
                new SettingsValue() { Display ="0.09/0.18/0.45/0.9", Value=0x06},
                new SettingsValue() { Display ="0.045/0.45/0.9/0.45", Value=0x07},
                new SettingsValue() { Display ="0.018/0.045/0.09/0.18", Value=0x08},
                new SettingsValue() { Display ="0.009/0.018/0.045/0.09", Value=0x09},
        };

        public ObservableCollection<SettingsValue> L3G20HighPassFilterCuttoffCollection { get { return _l3gd20HighPassFilterCuttoffCollection; } }


        public byte L3GD20HPFCutoff
        {
            get { return (byte)(L3GD20CtrlReg2 & 0x0F); }
            set
            {
                L3GD20CtrlReg2 = (byte)(0x20 | value);
            }
        }

        ObservableCollection<SettingsValue> _l3gd20DataRateCuttoffCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="95 Hz/12.5", Value=(0x00 << 6) | (0x00 << 4)},
                new SettingsValue() { Display ="95 Hz/25", Value=(0x00 << 6) | (0x01 << 4)},
                new SettingsValue() { Display ="190 Hz/12.5", Value=(0x01 << 6) | (0x00 << 4) },
                new SettingsValue() { Display ="190 Hz/25", Value=(0x01 << 6) | (0x01 << 4) },
                new SettingsValue() { Display ="190 Hz/50", Value=(0x01 << 6) | (0x02 << 4) },
                new SettingsValue() { Display ="190 Hz/70", Value=(0x01 << 6) | (0x03 << 4) },
                new SettingsValue() { Display ="380 Hz/20", Value=(0x02 << 6) | (0x00 << 4) },
                new SettingsValue() { Display ="380 Hz/25", Value=(0x02 << 6) | (0x01 << 4) },
                new SettingsValue() { Display ="380 Hz/50", Value=(0x02 << 6) | (0x02 << 4) },
                new SettingsValue() { Display ="380 Hz/100", Value=(0x02 << 6) | (0x03 << 4) },
                new SettingsValue() { Display ="760 Hz/30", Value=(0x03 << 6) | (0x00 << 4) },
                new SettingsValue() { Display ="760 Hz/35", Value=(0x03 << 6) | (0x01 << 4) },
                new SettingsValue() { Display ="760 Hz/50", Value=(0x03 << 6) | (0x02 << 4) },
                new SettingsValue() { Display ="760 Hz/100", Value=(0x03 << 6) | (0x03 << 4) },
            };


        public ObservableCollection<SettingsValue> L3GD20DataRateCuttoffCollection { get { return _l3gd20DataRateCuttoffCollection; } }

        public byte L3GD20DataRateCuttoff
        {
            get { return (byte)(L3GD20CtrlReg1 & 0xF0); }
            set
            {
                byte tmp = (byte)(L3GD20CtrlReg1 & 0x0F); //Clear teh top 4 bytes, so we can assign the data rate.
                L3GD20CtrlReg1 = (byte)(tmp | value);
            }
        }



        #endregion

        #region MPU60x0
        byte _mpu60x0GyroEnabled;
        [DeviceSetting(SettingAddress = MPU60x0_GYRO_ENABLED)]
        public bool MPU60x0GryoEnabled
        {
            get { return _mpu60x0GyroEnabled != 0; }
            set { Set(value, ref _mpu60x0GyroEnabled, () => MPU60x0GryoEnabled); }
        }

        byte _mpu60x0AccEnabled;
        [DeviceSetting(SettingAddress = MPU60x0_ACC_ENABLED)]
        public bool MPU60x0AccEnabled
        {
            get { return _mpu60x0AccEnabled != 0; }
            set { Set(value, ref _mpu60x0AccEnabled, () => MPU60x0AccEnabled); }
        }

        byte _mpu60x0FifoEnabled;
        [DeviceSetting(SettingAddress = MPU60x0_FIFO_ENABLE)]
        public bool MPU60x0FifoEnabled
        {
            get { return _mpu60x0FifoEnabled != 0; }
            set { Set(value, ref _mpu60x0FifoEnabled, () => MPU60x0FifoEnabled); }
        }

        ObservableCollection<SettingsValue> _mpu60x0DLPFCollection = new ObservableCollection<SettingsValue>()
            {
                //new SettingsValue(){Display ="256 Hz", Value = 0x00 | 0x10 },
                new SettingsValue(){Display ="184Acc/188Gyr Hz", Value = 0x01 }, //If we OR a 0x18 (or 0bXXX11XXXX) into D4, D3, we set the proper full scale value (3 => +/- 2000 d/s
                new SettingsValue(){Display ="94Acc/98Gyr Hz", Value = 0x02 },
                new SettingsValue(){Display ="44Acc/42Gyr Hz", Value = 0x03 },
                new SettingsValue(){Display ="21Acc/20Gyr Hz", Value = 0x04 },
                new SettingsValue(){Display ="10Acc/10Gyr Hz", Value = 0x05 },
                new SettingsValue(){Display ="5Acc/5Gyr Hz", Value = 0x06 },
            };

        public ObservableCollection<SettingsValue> MPU60x0DLPFCollection { get { return _mpu60x0DLPFCollection; } }

        byte _mpu60x0Dlpf;
        [DeviceSetting(SettingAddress = MPU60x0_DLPF)]
        public byte MPU60x0Dlpf
        {
            get { return _mpu60x0Dlpf; }
            set { Set(value, ref _mpu60x0Dlpf, () => MPU60x0Dlpf); }
        }

        ObservableCollection<SettingsValue> _mpu60x0SampleRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue(){Display ="1 KHz", Value = 0 },
                new SettingsValue(){Display ="500 Hz", Value = 1 },
                new SettingsValue(){Display ="333 Hz", Value = 2 },
                new SettingsValue(){Display ="250 Hz", Value = 3 },
                new SettingsValue(){Display ="200 Hz", Value = 4 },
                new SettingsValue(){Display ="166 Hz", Value = 5},
                new SettingsValue(){Display ="142 Hz", Value = 6},
                new SettingsValue(){Display ="125 Hz", Value = 7},
                new SettingsValue(){Display ="111 Hz", Value = 8},
                new SettingsValue(){Display ="100 Hz", Value = 9},
                new SettingsValue(){Display ="50 Hz", Value = 19},
                new SettingsValue(){Display ="25 Hz", Value = 39},
                new SettingsValue(){Display ="10 Hz", Value = 99},                
            };
        public ObservableCollection<SettingsValue> MPU60x0SampleRateCollection { get { return _mpu60x0SampleRateCollection; } }

        byte _mpu60x0SmplRtDiv;
        [DeviceSetting(SettingAddress = MPU60x0_SMPLRT_DIV)]
        public byte MPU60x0SmpRtDiv
        {
            get { return _mpu60x0SmplRtDiv; }
            set { Set(value, ref _mpu60x0SmplRtDiv, () => MPU60x0SmpRtDiv); }
        }

        ObservableCollection<SettingsValue> _mpu60x0GyroFSCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 250 °/s", Value = 0 << 3}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="+/- 500 °/s", Value = 0x01 << 3},
                new SettingsValue() { Display ="+/- 1000 °/s", Value = 0x2 << 3},
                new SettingsValue() { Display ="+/- 2000 °/s", Value = 0x3 << 3},
            };
        public ObservableCollection<SettingsValue> MPU60x0GyroFSCollection { get { return _mpu60x0GyroFSCollection; } }

        byte _mpu60x0GyroFS;
        [DeviceSetting(SettingAddress = MPU60x0_GYRO_FS)]
        public byte MPU60x0GyroFS
        {
            get { return _mpu60x0GyroFS; }
            set { Set(value, ref _mpu60x0GyroFS, () => MPU60x0GyroFS); }
        }

        ObservableCollection<SettingsValue> _mpu60x0AccFSCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 2g", Value = 0}, //Just shove this value into BW_RATE (always use normal power), OR'ing 0x10 will switch to low power
                new SettingsValue() { Display ="+/- 4g", Value = 0x01 << 3},
                new SettingsValue() { Display ="+/- 8g", Value = 0x02 << 3},
                new SettingsValue() { Display ="+/- 16g", Value = 0x03 << 3},
            };
        public ObservableCollection<SettingsValue> MPU60x0AccFSGainCollection { get { return _mpu60x0AccFSCollection; } }

        byte _mpu60x0AccFS;
        [DeviceSetting(SettingAddress = MPU60x0_ACC_FS)]
        public byte MPU60x0AccFS
        {
            get { return _mpu60x0AccFS; }
            set { Set(value, ref _mpu60x0AccFS, () => MPU60x0AccFS); }
        }

        byte _mpu60x0GyroFilterType;
        [DeviceSetting(SettingAddress = MPU60x0_GYRO_FILTER_TYPE)]
        public byte MPU60x0GyroFilterType
        {
            get { return _mpu60x0GyroFilterType; }
            set { Set(value, ref _mpu60x0GyroFilterType, () => MPU60x0GyroFilterType); }
        }

        public byte MPU60x0GyroMedianFilterOption
        {
            get { return (byte)(_mpu60x0GyroFilterType & 0x0F); }
            set { MPU60x0GyroFilterType = (byte)((_mpu60x0GyroFilterType & 0xf0) | value); }
        }

        public byte MPU60x0GyroMovingAverageFilterOption
        {
            get { return (byte)(_mpu60x0GyroFilterType & 0xF0); }
            set { MPU60x0GyroFilterType = (byte)((_mpu60x0GyroFilterType & 0x0F) | value); }
        }

        byte _mpu60x0GyroSmplRt;
        [DeviceSetting(SettingAddress = MPU60x0_GYRO_SAMPLE_RATE)]
        public byte MPU60x0GyroSmplRt
        {
            get { return _mpu60x0GyroSmplRt; }
            set { Set(value, ref _mpu60x0GyroSmplRt, () => MPU60x0GyroSmplRt); }
        }

        byte _mpu60x0AccFilterType;
        [DeviceSetting(SettingAddress = MPU60x0_ACC_FILTER_TYPE)]
        public byte MPU60x0AccFilterType
        {
            get { return _mpu60x0AccFilterType; }
            set { Set(value, ref _mpu60x0AccFilterType, () => MPU60x0AccFilterType); }
        }

        public byte MPU60x0AccMedianFilterOption
        {
            get { return (byte)(_mpu60x0AccFilterType & 0x0F); }
            set { MPU60x0AccFilterType = (byte)((_mpu60x0AccFilterType & 0xf0) | value); }
        }

        public byte MPU60x0AccMovingAverageFilterOption
        {
            get { return (byte)(_mpu60x0AccFilterType & 0xF0); }
            set { MPU60x0AccFilterType = (byte)((_mpu60x0AccFilterType & 0x0F) | value); }
        }

        byte _mpu60x0AccSmplRt;
        [DeviceSetting(SettingAddress = MPU60x0_ACC_SAMPLE_RATE)]
        public byte MPU60x0AccSmplRt
        {
            get { return _mpu60x0AccSmplRt; }
            set { Set(value, ref _mpu60x0AccSmplRt, () => MPU60x0AccSmplRt); }
        }
        #endregion

        #region GPS Configuration

        ObservableCollection<SettingsValue> _gpsDGPSTypes = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="None", Value = 0 },
                new SettingsValue() { Display ="RTCM", Value = 1 },
                new SettingsValue() { Display ="WAAS", Value = 2 },
            };

        public ObservableCollection<SettingsValue> GPSDGPSTypes { get { return _gpsDGPSTypes; } }

        byte _gpsDGPSType;
        [DeviceSetting(SettingAddress = GPS_DGPS_Type)]
        public byte GPSDGPSType
        {
            get { return _gpsDGPSType; }
            set { Set(value, ref _gpsDGPSType, () => GPSDGPSType); }
        }

        #endregion

        #region Lipo ADC
        byte _lipoAdcEnabled;
        [DeviceSetting(SettingAddress = LIPO_ADC_FILTER_ENABLED)]
        public bool LipoAdcEnabled
        {
            get { return _lipoAdcEnabled != 0; }
            set { Set(value, ref _lipoAdcEnabled, () => LipoAdcEnabled); }
        }


        byte _lipoAdcSampleRate;
        [DeviceSetting(SettingAddress = LIPO_ADC_SAMPLE_RATE)]
        public byte LipoADCSampleRate
        {
            get { return _lipoAdcSampleRate; }
            set { Set(value, ref _lipoAdcSampleRate, () => LipoADCSampleRate); }
        }

        byte _lipoAdcFilterType;
        [DeviceSetting(SettingAddress = LIPO_ADC_FILTER_TYPE)]
        public byte LipoADCFilterType
        {
            get { return _lipoAdcFilterType; }
            set { Set(value, ref _lipoAdcFilterType, () => LipoADCFilterType); }
        }

        public byte LipoADCMedianFilterOption
        {
            get { return (byte)(_lipoAdcFilterType & 0x0F); }
            set { LipoADCFilterType = (byte)((_lipoAdcFilterType & 0xf0) | value); }
        }

        public byte LipoADCMovingAverageFilterOption
        {
            get { return (byte)(_lipoAdcFilterType & 0xF0); }
            set { LipoADCFilterType = (byte)((_lipoAdcFilterType & 0x0F) | value); }
        }
        #endregion

        #region LSM303 Accelerometer
        #region Actual Registers/EEPROM Values
        byte _lsm303AccEnabled;
        [DeviceSetting(SettingAddress = LSM303_ACC_Enabled)]
        public bool LSM303AccEnabled
        {
            get { return _lsm303AccEnabled != 0; }
            set { Set(value, ref _lsm303AccEnabled, () => LSM303AccEnabled); }
        }

        byte _lsm303AccFiFoEnable;
        [DeviceSetting(SettingAddress = LSM303_ACC_FIFO_ENABLE)]
        public bool LSM303FiFoEnable
        {
            get { return _lsm303AccFiFoEnable != 0; }
            set { Set(value, ref _lsm303AccFiFoEnable, () => LSM303FiFoEnable); }
        }


        byte _lsm303AccSampleRate;
        [DeviceSetting(SettingAddress = LSM303_ACC_SAMPLE_RATE)]
        public byte LSM303AccSampleRate
        {
            get { return _lsm303AccSampleRate; }
            set { Set(value, ref _lsm303AccSampleRate, () => LSM303AccSampleRate); }
        }


        byte _lsm303AccReg1;
        [DeviceSetting(SettingAddress = LSM303_ACC_REG1)]
        public byte LSM303AccReg1
        {
            get { return _lsm303AccReg1; }
            set { Set(value, ref _lsm303AccReg1, () => LSM303AccReg1); }
        }

        byte _lsm303AccReg2;
        [DeviceSetting(SettingAddress = LSM303_ACC_REG2)]
        public byte LSM303AccReg2
        {
            get { return _lsm303AccReg2; }
            set { Set(value, ref _lsm303AccReg2, () => LSM303AccReg2); }
        }

        byte _lsm303AccReg3;
        [DeviceSetting(SettingAddress = LSM303_ACC_REG3)]
        public byte LSM303AccReg3
        {
            get { return _lsm303AccReg3; }
            set { Set(value, ref _lsm303AccReg3, () => LSM303AccReg3); }
        }

        byte _lsm303AccReg4;
        [DeviceSetting(SettingAddress = LSM303_ACC_REG4)]
        public byte LSM303AccReg4
        {
            get { return _lsm303AccReg4; }
            set { Set(value, ref _lsm303AccReg4, () => LSM303AccReg4); }
        }

        byte _lsm303AccFilterOption;
        [DeviceSetting(SettingAddress = LSM303_ACC_FILTER_OPTION)]
        public byte LSM303AccFilterOption
        {
            get { return _lsm303AccFilterOption; }
            set { Set(value, ref _lsm303AccFilterOption, () => LSM303AccFilterOption); }
        }

        public byte LSM303AccMedianFilterOption
        {
            get { return (byte)(_lsm303AccFilterOption & 0x0F); }
            set { LSM303AccFilterOption = (byte)((_lsm303AccFilterOption & 0xf0) | value); }
        }

        public byte LSM303AccMovingAverageFilterOption
        {
            get { return (byte)(_lsm303AccFilterOption & 0xF0); }
            set { LSM303AccFilterOption = (byte)((_lsm303AccFilterOption & 0x0F) | value); }
        }
        #endregion

        ObservableCollection<SettingsValue> _lsm303AccFullScale = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 2G", Value=0x00 << 4 },
                new SettingsValue() { Display ="+/- 4G", Value=0x01 << 4 },
                new SettingsValue() { Display ="+/- 8G", Value=0x02 << 4 },
                new SettingsValue() { Display ="+/- 16G", Value=0x03 << 4 },
            };

        public ObservableCollection<SettingsValue> LSM303AccFullScaleCollection { get { return _lsm303AccFullScale; } }

        public byte LSM303AccFullScale
        {
            get { return (byte)(_lsm303AccReg4 & (0x03 << 4)); }
            set
            {
                byte tmp = (byte)(~(0x03 << 4) & LSM303AccReg4);
                LSM303AccReg4 = (byte)(value | tmp);
            }
        }

        ObservableCollection<SettingsValue> _lsm303AccPowerCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="Low Power", Value= (0x01 << 3) },
                new SettingsValue() { Display ="Normal", Value=0x0 },
            };

        public ObservableCollection<SettingsValue> LSMAccPowerModeCollection { get { return _lsm303AccPowerCollection; } }

        public byte LSM303PowerMode
        {
            get { return (byte)(LSM303AccReg1 & (0x01 << 3)); }
            set
            {
                byte tmp = (byte)(LSM303AccReg1 & ~(0x01 << 3));
                LSM303AccReg1 = (byte)(tmp | value);
            }
        }

        ObservableCollection<SettingsValue> _lsm303AccHighPassFilterCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="Disabled", Value=0x80 },
                new SettingsValue() { Display ="Cuttoff 1", Value=0x90 },
                new SettingsValue() { Display ="Cuttoff 2", Value=0xB0 },
                new SettingsValue() { Display ="Cuttoff 3", Value=0xD0 },
                new SettingsValue() { Display ="Cuttoff 4", Value=0xF0 },
            };

        public ObservableCollection<SettingsValue> LSM303AccHighPassFilterCollection { get { return _lsm303AccHighPassFilterCollection; } }


        public byte LSM303AccHighPassFilter
        {
            get { return (byte)(LSM303AccReg2); }
            set { LSM303AccReg2 = (byte)(value); }
        }

        ObservableCollection<SettingsValue> _lsm303AccDataRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="1 Hz", Value=0x01 << 4 },
                new SettingsValue() { Display ="10 Hz", Value=0x02 << 4 },
                new SettingsValue() { Display ="25 Hz", Value=0x03 << 4 },
                new SettingsValue() { Display ="50 Hz", Value=0x04 << 4 },
                new SettingsValue() { Display ="100 Hz", Value=0x05 << 4 },
                new SettingsValue() { Display ="200 Hz", Value=0x06 << 4 },
                new SettingsValue() { Display ="400 Hz", Value=0x07 << 4 },
                new SettingsValue() { Display ="1.62 KHz", Value=0x08 << 4 },
                new SettingsValue() { Display ="1.34/5.376 KHz", Value=0x09 << 4 },
            };

        public ObservableCollection<SettingsValue> LSM303AccDataRateCollection { get { return _lsm303AccDataRateCollection; } }

        public byte LSM303AccDataRate
        {
            get { return (byte)(LSM303AccReg1 & 0xF0); }
            set
            {
                byte tmp = (byte)(LSM303AccReg1 & 0x0F);
                LSM303AccReg1 = (byte)(tmp | value);
            }
        }
        #endregion

        #region LSM303 Magnometer
        CalibrationStatus _lsm303MagCalibrated;
        public CalibrationStatus Lsm303Calibrated
        {
            get { return _lsm303MagCalibrated; }

        }

        byte _lsm303MagEnabled;
        [DeviceSetting(SettingAddress = LSM303_MAG_Enabled)]
        public bool LSM303MagEnabled
        {
            get { return _lsm303MagEnabled != 0; }
            set { Set(value, ref _lsm303MagEnabled, () => LSM303MagEnabled); }
        }

        byte _lsm303MagSampleRate;
        [DeviceSetting(SettingAddress = LSM303_MAG_SAMPLE_RATE)]
        public byte LSM303MagSampleRate
        {
            get { return _lsm303MagSampleRate; }
            set { Set(value, ref _lsm303MagSampleRate, () => LSM303MagSampleRate); }
        }

        byte _lsm303MagCraReg;
        [DeviceSetting(SettingAddress = LSM303_MAG_CRA_REG)]
        public byte LSM303MagCraReg
        {
            get { return _lsm303MagCraReg; }
            set { Set(value, ref _lsm303MagCraReg, () => LSM303MagCraReg); }
        }

        byte _lsm303MagCrbReg;
        [DeviceSetting(SettingAddress = LSM303_MAG_CRB_REG)]
        public byte LSM303MagCrbReg
        {
            get { return _lsm303MagCrbReg; }
            set { Set(value, ref _lsm303MagCrbReg, () => LSM303MagCrbReg); }
        }

        byte _lsm303MagMRReg;
        [DeviceSetting(SettingAddress = LSM303_MAG_MR_REG)]
        public byte LSM303MagMRReg
        {
            get { return _lsm303MagMRReg; }
            set { Set(value, ref _lsm303MagMRReg, () => LSM303MagMRReg); }
        }

        byte _lsm303MagFilterOption;
        [DeviceSetting(SettingAddress = LSM303_MAG_FILTER_OPTION)]
        public byte LSM303MagFilterOption
        {
            get { return _lsm303MagFilterOption; }
            set { Set(value, ref _lsm303MagFilterOption, () => LSM303MagFilterOption); }
        }

        public byte LSM303MagMedianFilterOption
        {
            get { return (byte)(_lsm303MagFilterOption & 0x0F); }
            set { LSM303MagFilterOption = (byte)((_lsm303MagFilterOption & 0xf0) | value); }
        }

        public byte LSM303MagMovingAverageFilterOption
        {
            get { return (byte)(_lsm303MagFilterOption & 0xF0); }
            set { LSM303MagFilterOption = (byte)((_lsm303MagFilterOption & 0x0F) | value); }
        }

        ObservableCollection<SettingsValue> _lsm303MagGain = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="+/- 1.3 ga", Value=0x01 << 5 },
                new SettingsValue() { Display ="+/- 1.9 ga", Value=0x02 << 5 },
                new SettingsValue() { Display ="+/- 2.5 ga", Value=0x03 << 5 },
                new SettingsValue() { Display ="+/- 4.0 ga", Value=0x04 << 5 },
                new SettingsValue() { Display ="+/- 4.7 ga", Value=0x05 << 5 },
                new SettingsValue() { Display ="+/- 5.6 ga", Value=0x06 << 5 },
                new SettingsValue() { Display ="+/- 8.1 ga", Value=0x07 << 5 }
            };

        public ObservableCollection<SettingsValue> LSM303MagGainCollection { get { return _lsm303MagGain; } }

        public byte LSM303MagGain
        {
            get { return (byte)(LSM303MagCrbReg & (byte)(0x07 << 5)); }
            set
            {
                byte tmp = (byte)(LSM303MagCrbReg & ~(byte)(0x07 << 5));
                LSM303MagCrbReg = (byte)(tmp | value);
            }
        }

        ObservableCollection<SettingsValue> _lsm303MagDataRateCollection = new ObservableCollection<SettingsValue>()
            {
                new SettingsValue() { Display ="0.75 Hz", Value=0x00 << 2 },
                new SettingsValue() { Display ="1.5 Hz", Value=0x01 << 2 },
                new SettingsValue() { Display ="3.0 Hz", Value=0x02 << 2 },
                new SettingsValue() { Display ="7.5 Hz", Value=0x03 << 2 },
                new SettingsValue() { Display ="15 Hz", Value=0x04 << 2 },
                new SettingsValue() { Display ="30 Hz", Value=0x05 << 2 },
                new SettingsValue() { Display ="75 Hz", Value=0x06 << 2 },
                new SettingsValue() { Display ="220 Hz", Value=0x07 << 2 },
            };

        public ObservableCollection<SettingsValue> LSM303MagDataRateCollection { get { return _lsm303MagDataRateCollection; } }

        public byte LSM303MagDataRate
        {
            get { return (byte)(LSM303MagCraReg & (0x07 << 2)); }
            set
            {
                byte tmp = (byte)(LSM303MagCraReg & ~(0x07 << 2));
                LSM303MagCraReg = (byte)(tmp | value);
            }
        }

        #endregion LSM303 Magnometer
    }
}
