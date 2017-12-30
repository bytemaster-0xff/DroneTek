using NiVek.Common.Comms;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class MicroConfiguration : ModelBase, IModel
    {        
        public enum PIDTypes
        {
            Roll = 0,
            Pitch = 1,
            Yaw = 2,
            Altitude = 3,
            Stable = 4
        }

        public enum ParameterIndex
        {
            DeviceAddress = 100,

            GyroAccCompFilter = 110,
            GyroMagCompFilter = 111,

            RCRate = 120,
            RCExpo = 121,

            RollPitchRate = 130,
            YawRate = 131,
            DynamicThrottleAdjustment = 132,

            RollTrim = 140,
            PitchTrim = 141
        }

        public class PID : ModelBase
        {
            PIDTypes _pidTypes;


            private PID() {  }

            public PID(PIDTypes pidType)
            {
                _pidTypes = pidType;
            }

            public PIDTypes PIDType
            {
                get{return _pidTypes;}
                set { _pidTypes = value; }
            }

            private Int16 _p;
            public Int16 P
            {
                get { return _p; }
                set
                {
                    if (_p != value)
                    {
                        _p = value;
                        OnPropertyChanged(() => P);
                    }
                }
            }

            private Int16 _i;
            public Int16 I
            {
                get { return _i; }
                set
                {
                    if (_i != value)
                    {
                        _i = value;
                        OnPropertyChanged(() => I);
                    }
                }
            }

            private Int16 _d;
            public Int16 D
            {
                get { return _d; }
                set
                {
                    if (_d != value)
                    {
                        _d = value;
                        OnPropertyChanged(() => P);
                    }
                }
            }

            public void Update(BufferReader rdr)
            {
                P = rdr.ReadByte();
                I = rdr.ReadByte();
                D = rdr.ReadByte();
            }
        }

        public MicroConfiguration()
        {
            _pidRoll = new PID(PIDTypes.Roll);
            _pidPitch = new PID(PIDTypes.Pitch);
            _pidYaw = new PID(PIDTypes.Yaw);
            _pidAlt = new PID(PIDTypes.Altitude);
            _pidStable = new PID(PIDTypes.Stable);            
        }

        public MicroConfiguration(Drone drone)
        {
            Drone = Drone;
            _pidRoll = new PID(PIDTypes.Roll);
            _pidPitch = new PID(PIDTypes.Pitch);                        
            _pidYaw = new PID(PIDTypes.Yaw);
            _pidAlt = new PID(PIDTypes.Altitude);
            _pidStable = new PID(PIDTypes.Stable);            
        }

        void _pid_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            SendPID(sender as PID);
        }

        private bool _isReady = false;
        public bool IsReady
        {
            get { return _isReady; }
        }


        public void Update(BufferReader rdr)
        {
            _sysConfigVersion = rdr.ReadByte();
            _deviceAddress = rdr.ReadByte();

            _pidRoll.P = rdr.ReadByte();
            _pidPitch.P = rdr.ReadByte();
            _pidYaw.P = rdr.ReadByte();
            _pidAlt.P = rdr.ReadByte();
            _pidStable.P = rdr.ReadByte();

            _pidRoll.I = rdr.ReadByte();
            _pidPitch.I = rdr.ReadByte();
            _pidYaw.I = rdr.ReadByte();
            _pidAlt.I = rdr.ReadByte();
            _pidStable.I = rdr.ReadByte();

            _pidRoll.D = rdr.ReadByte();
            _pidPitch.D = rdr.ReadByte();
            _pidYaw.D = rdr.ReadByte();
            _pidAlt.D = rdr.ReadByte();
            _pidStable.D = rdr.ReadByte();
            
            _pidPitch.PropertyChanged += _pid_PropertyChanged;
            _pidRoll.PropertyChanged += _pid_PropertyChanged;
            _pidYaw.PropertyChanged += _pid_PropertyChanged;
            _pidAlt.PropertyChanged += _pid_PropertyChanged;
            _pidStable.PropertyChanged += _pid_PropertyChanged;

            _rcRate = rdr.ReadByte();
            _rcExpo = rdr.ReadByte();

            _rollPitchRate = rdr.ReadByte();
            _yawRate = rdr.ReadByte();

            _dynamicThrottle = rdr.ReadByte();

            _accTrimRoll = rdr.ReadShort();
            _accTrimPitch = rdr.ReadShort();

            _gyroAccCompFilter = rdr.ReadShort();
            _gyroMagCompFilter = rdr.ReadShort();

            _isReady = true;
        }

        private byte _sysConfigVersion;
        public byte SysConfigVersion
        {
            get { return _sysConfigVersion; }
            set { }
        }

        private byte _deviceAddress;
        public byte DeviceAddress
        {
            get { return _deviceAddress; }
            set { 
                if(_deviceAddress != value)
                {
                    _deviceAddress = value;
                    SendParameter(ParameterIndex.DeviceAddress, (short)value);
                }
            }
        }

        public static MicroConfiguration Create(BufferReader rdr, Drone drone)
        {
            var config = new MicroConfiguration(drone);
            try
            {
                config.Update(rdr);
                return config;
            }
            catch(Exception)
            {
                return null;
            }
        }

        #region Motor Power
        byte _motorPower = 0;
        public byte MotorPower
        {
            get { return _motorPower; }
            set
            {
                if (_motorPower != value)
                {
                    _motorPower = value;
                    SendMotorPower(_motorPower);
                    OnPropertyChanged(() => MotorPower);
                }
            }
        }

        byte _motor = 0xFF;
        public byte Motor
        {
            get { return _motor; }
            set
            {
                if (_motor != value)
                {
                    _motor = value;
                    SendMotorPower(_motorPower);
                    OnPropertyChanged(() => Motor);
                }
            }
        }

        private PID _pidRoll;
        public PID PIDRoll
        {
            get { return _pidRoll;  }
        }

        private PID _pidPitch;
        public PID PIDPitch
        {
            get { return _pidPitch; }
        }

        private PID _pidYaw;
        public PID PIDYaw
        {
            get { return _pidYaw; }
        }

        private PID _pidAlt;
        public PID PIDAlt
        {
            get { return _pidAlt; }
        }

        public PID _pidStable;
        public PID PIDStable
        {
            get { return _pidStable;  }
        }

        private short _gyroAccCompFilter;
        public short GyroAccCompFilter
        {
            get { return _gyroAccCompFilter; }
            set {
                if(_gyroAccCompFilter != value)
                {
                    _gyroAccCompFilter = value;
                    SendParameter(ParameterIndex.GyroAccCompFilter, value);
                    OnPropertyChanged(() => GyroAccCompFilter);
                }
            }
        }

        private short _gyroMagCompFilter;
        public short GyroMagCompFilter
        {
            get { return _gyroMagCompFilter; }
            set
            {
                if (_gyroMagCompFilter != value)
                {
                    _gyroMagCompFilter = value;
                    SendParameter(ParameterIndex.GyroMagCompFilter, value);
                    OnPropertyChanged(() => GyroMagCompFilter);
                }
            }
        }

        private byte _rcRate;
        public byte RCRate
        {
            get { return _rcRate; }
            set
            {
                if (_rcRate != value)
                {
                    _rcRate = value;
                    SendParameter(ParameterIndex.RCRate, (short)value);
                    OnPropertyChanged(() => RCRate);
                }
            }
        }

        private byte _rcExpo;
        public byte RCExpo
        {
            get { return _rcExpo; }
            set
            {
                if (_rcExpo != value)
                {
                    _rcExpo = value;
                    SendParameter(ParameterIndex.RCExpo, (byte)value);
                    OnPropertyChanged(() => RCExpo);
                }
            }
        }

        private byte _rollPitchRate;
        public byte RollPitchRate
        {
            get { return _rollPitchRate;  }
            set
            {
                if(_rollPitchRate != value)
                {
                    _rollPitchRate = value;
                    SendParameter(ParameterIndex.RollPitchRate, (short)value);
                }
            }
        }

        private byte _yawRate;
        public byte YawRate
        {
            get { return _yawRate; }
            set
            {
                if (_yawRate != value)
                {
                    _yawRate = value;
                    SendParameter(ParameterIndex.YawRate, (short)value);
                }
            }
        }

        private byte _dynamicThrottle;
        public byte DynamicThrottle
        {
            get { return _dynamicThrottle; }
            set
            {
                if (_dynamicThrottle != value)
                {
                    _dynamicThrottle = value;
                    SendParameter(ParameterIndex.DynamicThrottleAdjustment, (short)value);
                }
            }
        }

        private short _accTrimRoll;
        public short AccTrimRoll
        {
            get { return _accTrimRoll; }
            set
            {
                if(_accTrimRoll != value)
                {
                    _accTrimRoll = value;
                    SendParameter(ParameterIndex.RollTrim, value);
                    OnPropertyChanged(() => AccTrimRoll);
                }
            }
        }

        private short _accTrimPitch;
        public short AccTrimPitch
        {
            get { return _accTrimPitch; }
            set
            {
                if (_accTrimPitch != value)
                {
                    _accTrimPitch = value;
                    SendParameter(ParameterIndex.PitchTrim, value);
                    OnPropertyChanged(() => AccTrimPitch);
                }
            }
        }

        public async void SendParameter(ParameterIndex power, short value)
        {
            Debug.WriteLine("Sending motor power {0} to {1}", power, _motor);

            var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = GPIOModule.CMD_SetConfigValue, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte((byte)power);
            msg.Add(value);
            await Drone.Channel.SendMessageAsync(Drone, msg);
        }

        public async void SendPID(PID pid)
        {
            var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = GPIOModule.CMD_SetPIDConstant, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte((byte)pid.PIDType);
            
            msg.AddByte((byte)pid.P);
            msg.AddByte((byte)pid.I);
            msg.AddByte((byte)pid.D);

            await  Drone.SendMessageAsync(msg);
        }


        public async void SendMotorPower(byte power)
        {
            Debug.WriteLine("Sending motor power {0} to {1}", power, _motor);

            var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte(_motor);
            msg.AddByte(power);
            OnPropertyChanged(() => Motor);
            Drone.ThrottleInput = power;

            await Drone.SendMessageAsync(msg);
        }
        #endregion

        ObservableCollection<KeyDisplay> _frameConfigCollection = new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "+", Key =  (byte)GPIOConfig.FrameConfigTypes.Cross },
             new KeyDisplay{Display = "X", Key =  (byte)GPIOConfig.FrameConfigTypes.X },
        };

        public ObservableCollection<KeyDisplay> FrameConfigCollection { get { return _frameConfigCollection; } }


        ObservableCollection<KeyDisplay> _escUpdateRateCollection = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "400Hz", Key =  8 },
                new KeyDisplay{Display = "350Hz", Key =  7 },
                new KeyDisplay{Display = "300Hz", Key =  6},
                new KeyDisplay{Display = "250Hz", Key =  5},
                new KeyDisplay{Display = "200Hz", Key =  4},
                new KeyDisplay{Display = "150Hz", Key =  3},
                new KeyDisplay{Display = "100Hz", Key =  2},
                new KeyDisplay{Display = "50Hz", Key = 1 }
            };
        public ObservableCollection<KeyDisplay> ESCUpdateRateCollection { get { return _escUpdateRateCollection; } }

        ObservableCollection<KeyDisplay> _motorPowerCollection = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "Off", Key = 0x00 },
                new KeyDisplay{Display = "5%", Key = (byte)(0xFF * 0.05f) },
                new KeyDisplay{Display = "10%", Key = (byte)(0xFF * 0.1f) },
                new KeyDisplay{Display = "15%", Key = (byte)(0xFF * 0.15f) },
                new KeyDisplay{Display = "20%", Key = (byte)(0xFF * 0.2f) },
                new KeyDisplay{Display = "23%", Key = (byte)(0xFF * 0.23f) },
                new KeyDisplay{Display = "26%", Key = (byte)(0xFF * 0.26f) },
                new KeyDisplay{Display = "30%", Key = (byte)(0xFF * 0.3f) },
                new KeyDisplay{Display = "40%", Key = (byte)(0xFF * 0.4f) },
                new KeyDisplay{Display = "50%", Key = (byte)(0xFF * 0.5f) },
                new KeyDisplay{Display = "60%", Key = (byte)(0xFF * 0.6f) },
                new KeyDisplay{Display = "70%", Key = (byte)(0xFF * 0.7f) },
                new KeyDisplay{Display = "80%", Key = (byte)(0xFF * 0.8f) },
                new KeyDisplay{Display = "90%", Key = (byte)(0xFF * 0.9f) },
                new KeyDisplay{Display = "100%", Key = 0xFF }                
            };

        public ObservableCollection<KeyDisplay> MotorPowerCollection { get { return _motorPowerCollection; } }

        ObservableCollection<KeyDisplay> _motorCollectionX = new ObservableCollection<KeyDisplay>(){
                new KeyDisplay{Display = "All", Key = 0xFF },
                new KeyDisplay{Display = "Port Front", Key = 0x01 },
                new KeyDisplay{Display = "Port Rear", Key = 0x02 },
                new KeyDisplay{Display = "Starboard Front", Key = 0x03 },
                new KeyDisplay{Display = "Starboard Rear", Key = 0x04 },                
            };
        public ObservableCollection<KeyDisplay> MotorCollectionX { get { return _motorCollectionX; } }

        ObservableCollection<KeyDisplay> _motorCollectionCross = new ObservableCollection<KeyDisplay>(){
                new KeyDisplay{Display = "All", Key = 0xFF },
                new KeyDisplay{Display = "Front", Key = 0x01 },
                new KeyDisplay{Display = "Port", Key = 0x02 },
                new KeyDisplay{Display = "Starboard", Key = 0x03 },
                new KeyDisplay{Display = "Rear", Key = 0x04 },                
            };
        public ObservableCollection<KeyDisplay> MotorCollectionCross { get { return _motorCollectionCross; } }
    }
}
