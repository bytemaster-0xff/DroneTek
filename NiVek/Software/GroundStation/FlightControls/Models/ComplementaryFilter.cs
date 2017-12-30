using NiVek.FlightControls.Commo;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class ComplementaryFilter : ConfigBase, INotifyPropertyChanged
    {
        #region Fields
        bool _enabled;
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

        public event PropertyChangedEventHandler PropertyChanged;
      

        public ComplementaryFilter(byte[] buffer)
        {
            var idx = 0;

            _enabled = buffer[idx++] != 0;
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

        public bool IsBound
        {
            get;
            set;
        }

        void UploadSetting(ushort addr, byte value)
        {
            if (IsBound)
            {
                var msg = new OutgoingMessage() { MessageId = SensorModule.CMD_SetCfg_Value, ModuleType = OutgoingMessage.ModuleTypes.Sensor };
                addr |= COMPLEMENTARY_EEPROM_OFFSET;

                msg.Add(addr);
                msg.AddByte(value);
                msg.AddByte(0x00);
                msg.ExpectACK = true;
                App.Commo.Send(msg);

                Debug.WriteLine("Sending Value {0} {1}", addr, value);
            }
        }


        public bool Enabled
        {
            get
            {
                return _enabled;
            }
            set
            {
                
                if (_enabled != value)
                {
                    _enabled = value;
                    UploadSetting(0, (byte)(value ? 0xFF : 0x00));
                    PropertyChanged(this, new PropertyChangedEventArgs("Enabled"));
                }
            }
        }
        public byte UpdateRate
        {
            get
            {
                return _updateRate;
            }
            set
            {
                if (_updateRate != value)
                {
                    _updateRate = value;
                    UploadSetting(1, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("UpdateRate"));
                }
            }
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

        public byte PitchFilter
        {
            get
            {
                return _pitchFilter;
            }
            set
            {
                if (_pitchFilter != value)
                {
                    _pitchFilter = value;
                    UploadSetting(2, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchFilter"));
                }
            }
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

        public byte RollFilter
        {
            get
            {
                return _rollFilter;
            }
            set
            {
                if (_rollFilter != value)
                {
                    _rollFilter = value;
                    UploadSetting(3, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollFilter"));
                }
            }
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

        public byte HeadingFilter
        {
            get
            {
                return _headingFilter;
            }
            set
            {
                if (_headingFilter != value)
                {
                    _headingFilter = value;
                    UploadSetting(4, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingFilter"));
                }
            }
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

        public byte AltitudeFilter
        {
            get
            {
                return _altitudeFilter;
            }
            set
            {
                if (_altitudeFilter != value)
                {
                    _altitudeFilter = value;
                    UploadSetting(5, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeFilter"));
                }
            }
        }
        public byte PitchTimeConstant
        {
            get
            {
                return _pitchTimeConstant;
            }
            set
            {
                if (_pitchTimeConstant != value)
                {
                    _pitchTimeConstant = value;
                    UploadSetting(6, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchTimeConstant"));
                }
            }
        }
        public byte RollTimeConstant
        {
            get
            {
                return _rollTimeConstant;
            }
            set
            {
                if (_rollTimeConstant != value)
                {
                    _rollTimeConstant = value;
                    UploadSetting(7, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollTimeConstant"));
                }
            }
        }

        public byte HeadingTimeConstant
        {
            get
            {
                return _headingTimeConstant;
            }
            set
            {
                if (_headingTimeConstant != value)
                {
                    _headingTimeConstant = value;
                    UploadSetting(8, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingTimeConstant"));
                }
            }
        }
        public byte AltitudeTimeConstant
        {
            get
            {
                return _altitudeTimeConstant;
            }
            set
            {
                if (_altitudeTimeConstant != value)
                {
                    _altitudeTimeConstant = value;
                    UploadSetting(9, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeTimeConstant"));
                }
            }
        }
        public byte PitchSensor1
        {
            get
            {
                return _pitchSensor1;
            }
            set
            {
                if (_pitchSensor1 != value)
                {
                    _pitchSensor1 = value;
                    UploadSetting(10, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchSensor1"));
                }
            }
        }
        public byte PitchSensor2
        {
            get
            {
                return _pitchSensor2;
            }
            set
            {
                if (_pitchSensor2 != value)
                {
                    _pitchSensor2 = value;
                    UploadSetting(11, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchSensor2"));
                }
            }
        }
        public byte PitchSensor3
        {
            get
            {
                return _pitchSensor3;
            }
            set
            {
                if (_pitchSensor3 != value)
                {
                    _pitchSensor3 = value;
                    UploadSetting(12, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchSensor3"));
                }

            }
        }
        public byte RollSensor1
        {
            get
            {
                return _rollSensor1;
            }
            set
            {
                if (_rollSensor1 != value)
                {
                    _rollSensor1 = value;
                    UploadSetting(13, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollSensor1"));
                }
            }
        }
        public byte RollSensor2
        {
            get
            {
                return _rollSensor2;
            }
            set
            {
                if (_rollSensor2 != value)
                {
                    _rollSensor2 = value;
                    UploadSetting(14, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollSensor2"));
                }
            }
        }
        public byte RollSensor3
        {
            get
            {
                return _rollSensor3;
            }
            set
            {
                if (_rollSensor3 != value)
                {
                    _rollSensor3 = value;
                    UploadSetting(15, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollSensor3"));
                }
            }
        }
        public byte HeadingSensor1
        {
            get
            {
                return _headingSensor1;
            }
            set
            {
                if (_headingSensor1 != value)
                {
                    _headingSensor1 = value;
                    UploadSetting(16, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingSensor1"));
                }
            }
        }
        public byte HeadingSensor2
        {
            get
            {
                return _headingSensor2;
            }
            set
            {
                if (_headingSensor2 != value)
                {
                    _headingSensor2 = value;
                    UploadSetting(17, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingSensor2"));
                }
            }
        }
        public byte HeadingSensor3
        {
            get
            {
                return _headingSensor3;
            }
            set
            {
                if (_headingSensor3 != value)
                {
                    _headingSensor3 = value;
                    UploadSetting(18, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingSensor3"));
                }
            }
        }
        public byte AltitudeSensor1
        {
            get
            {
                return _altitudeSensor1;
            }
            set
            {
                if (_altitudeSensor1 != value)
                {
                    _altitudeSensor1 = value;
                    UploadSetting(19, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeSensor1"));
                }
            }
        }
        public byte AltitudeSensor2
        {
            get
            {
                return _altitudeSensor2;
            }
            set
            {
                if (_altitudeSensor2 != value)
                {
                    _altitudeSensor2 = value;
                    UploadSetting(20, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeSensor2"));
                }
            }
        }
        public byte AltitudeSensor3
        {
            get
            {
                return _altitudeSensor3;
            }
            set
            {
                if (_altitudeSensor3 != value)
                {
                    _altitudeSensor3 = value;
                    UploadSetting(21, value);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeSensor3"));
                }
            }
        }



    }
}
