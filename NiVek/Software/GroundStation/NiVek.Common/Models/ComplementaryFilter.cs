using NiVek.Common.Comms;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    [ModuleConfig(MessageId = SensorModule.CMD_SetCfg_Value, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor)]
    public class ComplementaryFilter : ConfigBase, IModel, INotifyPropertyChanged
    {
        #region Fields
        byte _enabled;
        byte _updateRate;

        byte _pitchFilter;
        byte _rollFilter;
        byte _headingFilter;
        byte _altitudeFilter;

        byte _pitchTimeConstant;
        byte _rollTimeConstant;
        byte _headingTimeConstant;
        byte _altitudeTimeConstant;

        byte _pitchSensor1;
        byte _pitchSensor2;
        byte _pitchSensor3;

        byte _rollSensor1;
        byte _rollSensor2;
        byte _rollSensor3;

        byte _headingSensor1;
        byte _headingSensor2;
        byte _headingSensor3;

        byte _altitudeSensor1;
        byte _altitudeSensor2;
        byte _altitudeSensor3;
        #endregion

        public ComplementaryFilter() { }

        public void Update(BufferReader reader)
        {
            var idx = 0;

            var buffer = reader.GetRawBuffer();

            _enabled = buffer[idx++];
            _updateRate = buffer[idx++];

            _pitchFilter = buffer[idx++];
            _rollFilter = buffer[idx++];
            _headingFilter = buffer[idx++];
            _altitudeFilter = buffer[idx++];

            _pitchTimeConstant = buffer[idx++];
            _rollTimeConstant = buffer[idx++];
            _headingTimeConstant = buffer[idx++];
            _altitudeTimeConstant = buffer[idx++];

            _pitchSensor1 = buffer[idx++];
            _pitchSensor2 = buffer[idx++];
            _pitchSensor3 = buffer[idx++];

            _rollSensor1 = buffer[idx++];
            _rollSensor2 = buffer[idx++];
            _rollSensor3 = buffer[idx++];

            _headingSensor1 = buffer[idx++];
            _headingSensor2 = buffer[idx++];
            _headingSensor3 = buffer[idx++];

            _altitudeSensor1 = buffer[idx++];
            _altitudeSensor2 = buffer[idx++];
            _altitudeSensor3 = buffer[idx++];

            _isReady = true;
        }

        public ComplementaryFilter(Drone drone, byte[] buffer)
        {
            this.Drone = drone;

            var idx = 0;

            _enabled = buffer[idx++];
            _updateRate = buffer[idx++];

            _pitchFilter = buffer[idx++];
            _rollFilter = buffer[idx++];
            _headingFilter = buffer[idx++];
            _altitudeFilter = buffer[idx++];

            _pitchTimeConstant = buffer[idx++];
            _rollTimeConstant = buffer[idx++];
            _headingTimeConstant = buffer[idx++];
            _altitudeTimeConstant = buffer[idx++];

            _pitchSensor1 = buffer[idx++];
            _pitchSensor2 = buffer[idx++];
            _pitchSensor3 = buffer[idx++];

            _rollSensor1 = buffer[idx++];
            _rollSensor2 = buffer[idx++];
            _rollSensor3 = buffer[idx++];

            _headingSensor1 = buffer[idx++];
            _headingSensor2 = buffer[idx++];
            _headingSensor3 = buffer[idx++];

            _altitudeSensor1 = buffer[idx++];
            _altitudeSensor2 = buffer[idx++];
            _altitudeSensor3 = buffer[idx++];
        }


        const ushort K_ENABLED = 0;
        const ushort K_UPDATE_RATE = 1;
        const ushort K_PITCH_FILTER = 2;
        const ushort K_ROLL_FILTER = 3;
        const ushort K_YAW_FILTER = 4;
        const ushort K_ALTITUDE_FILTER = 5;
        const ushort K_PITCH_TIME_CONSTANT = 6;
        const ushort K_ROLL_TIME_CONSTANT = 7;
        const ushort K_HEADING_TIME_CONSTANT = 8;
        const ushort K_ALTITUDE_TIME_CONSTANT = 9;

        const ushort K_PITCH_SENSOR_1 = 10;
        const ushort K_PITCH_SENSOR_2 = 11;
        const ushort K_PITCH_SENSOR_3 = 12;

        const ushort K_ROLL_SENSOR_1 = 13;
        const ushort K_ROLL_SENSOR_2 = 14;
        const ushort K_ROLL_SENSOR_3 = 15;

        const ushort K_HEADING_SENSOR_1 = 16;
        const ushort K_HEADING_SENSOR_2 = 17;
        const ushort K_HEADING_SENSOR_3 = 18;

        const ushort K_ALTITUDE_SENSOR_1 = 19;
        const ushort K_ALTITUDE_SENSOR_2 = 20;
        const ushort K_ALTITUDE_SENSOR_3 = 21;

        [DeviceSetting(SettingAddress = K_ENABLED)]
        public bool Enabled
        {
            get { return _enabled == 0x00; }
            set { Set(value, ref _enabled, () => Enabled); }
        }

        [DeviceSetting(SettingAddress = K_UPDATE_RATE)]
        public byte UpdateRate
        {
            get { return _updateRate; }
            set { Set(value, ref _updateRate, () => UpdateRate); }
        }

        public byte PitchMedianFilterOption
        {
            get { return (byte)(_pitchFilter & 0x0F); }
            set { PitchFilter = (byte)((_pitchFilter & 0xf0) | value); }
        }

        public byte PitchMovingAverageFilterOption
        {
            get { return (byte)(_pitchFilter & 0xF0); }
            set { PitchFilter = (byte)((_pitchFilter & 0x0F) | value); }
        }

        [DeviceSetting(SettingAddress = K_PITCH_FILTER)]
        public byte PitchFilter
        {
            get { return _pitchFilter; }
            set { Set(value, ref _pitchFilter, () => PitchFilter); }
        }

        public byte RollMedianFilterOption
        {
            get { return (byte)(_rollFilter & 0x0F); }
            set { RollFilter = (byte)((_rollFilter & 0xf0) | value); }
        }

        public byte RollMovingAverageFilterOption
        {
            get { return (byte)(_rollFilter & 0xF0); }
            set { RollFilter = (byte)((_rollFilter & 0x0F) | value); }
        }

        [DeviceSetting(SettingAddress = K_ROLL_FILTER)]
        public byte RollFilter
        {
            get { return _rollFilter; }
            set { Set(value, ref _rollFilter, () => RollFilter); }
        }

        public byte HeadingMedianFilterOption
        {
            get { return (byte)(_headingFilter & 0x0F); }
            set { HeadingFilter = (byte)((_headingFilter & 0xf0) | value); }
        }

        public byte HeadingMovingAverageFilterOption
        {
            get { return (byte)(_headingFilter & 0xF0); }
            set { HeadingFilter = (byte)((_headingFilter & 0x0F) | value); }
        }

        [DeviceSetting(SettingAddress = K_YAW_FILTER)]
        public byte HeadingFilter
        {
            get { return _headingFilter; }
            set { Set(value, ref _headingFilter, () => HeadingFilter); }
        }

        public byte AltitudeMedianFilterOption
        {
            get { return (byte)(_altitudeFilter & 0x0F); }
            set { AltitudeFilter = (byte)((_altitudeFilter & 0xf0) | value); }
        }

        public byte AltitudeMovingAverageFilterOption
        {
            get { return (byte)(_altitudeFilter & 0xF0); }
            set { AltitudeFilter = (byte)((_altitudeFilter & 0x0F) | value); }
        }

        [DeviceSetting(SettingAddress = K_ALTITUDE_FILTER)]
        public byte AltitudeFilter
        {
            get { return _altitudeFilter; }
            set { Set(value, ref _altitudeFilter, () => AltitudeFilter); }
        }

        [DeviceSetting(SettingAddress = K_PITCH_TIME_CONSTANT)]
        public byte PitchTimeConstant
        {
            get { return _pitchTimeConstant; }
            set { Set(value, ref _pitchTimeConstant, () => PitchTimeConstant); }
        }

        [DeviceSetting(SettingAddress = K_ROLL_TIME_CONSTANT)]
        public byte RollTimeConstant
        {
            get { return _rollTimeConstant; }
            set { Set(value, ref _rollTimeConstant, () => RollTimeConstant); }
        }

        [DeviceSetting(SettingAddress = K_HEADING_TIME_CONSTANT)]
        public byte HeadingTimeConstant
        {
            get { return _headingTimeConstant; }
            set { Set(value, ref _headingTimeConstant, () => HeadingTimeConstant); }
        }

        [DeviceSetting(SettingAddress = K_ALTITUDE_TIME_CONSTANT)]
        public byte AltitudeTimeConstant
        {
            get { return _altitudeTimeConstant; }
            set { Set(value, ref _altitudeTimeConstant, () => AltitudeTimeConstant); }
        }

        [DeviceSetting(SettingAddress = K_PITCH_SENSOR_1)]
        public byte PitchSensor1
        {
            get { return _pitchSensor1; }
            set { Set(value, ref _pitchSensor1, () => PitchSensor1); }
        }

        [DeviceSetting(SettingAddress = K_PITCH_SENSOR_2)]
        public byte PitchSensor2
        {
            get { return _pitchSensor2; }
            set { Set(value, ref _pitchSensor2, () => PitchSensor2); }
        }

        [DeviceSetting(SettingAddress = K_PITCH_SENSOR_3)]
        public byte PitchSensor3
        {
            get { return _pitchSensor3; }
            set { Set(value, ref _pitchSensor3, () => PitchSensor3); }
        }

        [DeviceSetting(SettingAddress = K_ROLL_SENSOR_1)]
        public byte RollSensor1
        {
            get { return _rollSensor1; }
            set { Set(value, ref _rollSensor1, () => RollSensor1); }
        }

        [DeviceSetting(SettingAddress = K_ROLL_SENSOR_2)]
        public byte RollSensor2
        {
            get { return _rollSensor2; }
            set { Set(value, ref _rollSensor2, () => RollSensor2); }
        }

        [DeviceSetting(SettingAddress = K_ROLL_SENSOR_3)]
        public byte RollSensor3
        {
            get { return _rollSensor3; }
            set { Set(value, ref _rollSensor3, () => RollSensor3); }
        }

        [DeviceSetting(SettingAddress = K_HEADING_SENSOR_1)]
        public byte HeadingSensor1
        {
            get { return _headingSensor1; }
            set { Set(value, ref _headingSensor1, () => HeadingSensor1); }
        }

        [DeviceSetting(SettingAddress = K_HEADING_SENSOR_2)]
        public byte HeadingSensor2
        {
            get { return _headingSensor2; }
            set { Set(value, ref _headingSensor2, () => HeadingSensor2); }
        }

        [DeviceSetting(SettingAddress = K_HEADING_SENSOR_3)]
        public byte HeadingSensor3
        {
            get { return _headingSensor3; }
            set { Set(value, ref _headingSensor3, () => HeadingSensor3); }
        }

        [DeviceSetting(SettingAddress = K_ALTITUDE_SENSOR_1)]
        public byte AltitudeSensor1
        {
            get { return _altitudeSensor1; }
            set { Set(value, ref _altitudeSensor1, () => AltitudeSensor1); }
        }

        [DeviceSetting(SettingAddress = K_ALTITUDE_SENSOR_2)]
        public byte AltitudeSensor2
        {
            get { return _altitudeSensor2; }
            set { Set(value, ref _altitudeSensor2, () => AltitudeSensor2); }
        }

        [DeviceSetting(SettingAddress = K_ALTITUDE_SENSOR_2)]
        public byte AltitudeSensor3
        {
            get { return _altitudeSensor3; }
            set { Set(value, ref _altitudeSensor3, () => AltitudeSensor3); }
        }
    }
}
