using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class SensorMinimal : ModelBase
    {
        public DateTime DateStamp { get; private set; }

        private int _pitch;
        public int Pitch
        {
            get { return _pitch; }
            set
            {
                if (_pitch != value)
                {
                    _pitch = value;
                    OnPropertyChanged(() => Pitch);
                    OnPropertyChanged(() => PitchAngle);
                }
            }
        }

        public double PitchAngle
        {
            get { return _pitch / 20.0f;  }
        }

        private int _roll;
        public int Roll
        {
            get { return _roll; }
            set
            {
                if (_roll != value)
                {
                    _roll = value;
                    OnPropertyChanged(() => Roll);
                    OnPropertyChanged(() => RollAngle);
                }
            }
        }

        private int _motor1;
        public int Motor1
        {
            get { return _motor1; }
            set
            {
                if (_motor1 != value)
                {
                    _motor1 = value;
                    OnPropertyChanged(() => Motor1);
                }
            }
        }

        private int _motor2;
        public int Motor2
        {
            get { return _motor2; }
            set
            {
                if (_motor2 != value)
                {
                    _motor2 = value;
                    OnPropertyChanged(() => Motor2);
                }
            }
        }
        private int _motor3;
        public int Motor3
        {
            get { return _motor3; }
            set
            {
                if (_motor3 != value)
                {
                    _motor3 = value;
                    OnPropertyChanged(() => Motor3);
                }
            }
        }
        
        private int _motor4;
        public int Motor4
        {
            get { return _motor4; }
            set
            {
                if (_motor4 != value)
                {
                    _motor4 = value;
                    OnPropertyChanged(() => Motor4);
                }
            }
        }


        private int _rc1;
        public int RC1
        {
            get { return _rc1; }
            set
            {
                if (_rc1 != value)
                {
                    _rc1 = value;
                    OnPropertyChanged(() => RC1);
                }
            }
        }
        private int _rc2;
        public int RC2
        {
            get { return _rc2; }
            set
            {
                if (_rc2 != value)
                {
                    _rc2 = value;
                    OnPropertyChanged(() => RC2);
                }
            }
        }
        private int _rc3;
        public int RC3
        {
            get { return _rc3; }
            set
            {
                if (_rc3 != value)
                {
                    _rc3 = value;
                    OnPropertyChanged(() => RC3);
                }
            }
        }
        private int _rc4;
        public int RC4
        {
            get { return _rc4; }
            set
            {
                if (_rc4 != value)
                {
                    _rc4 = value;
                    OnPropertyChanged(() => RC4);
                }
            }
        }

        private int _rc5;
        public int RC5
        {
            get { return _rc5; }
            set
            {
                if (_rc5 != value)
                {
                    _rc5 = value;
                    OnPropertyChanged(() => RC5);
                }
            }
        }
        private int _rc6;
        public int RC6
        {
            get { return _rc6; }
            set
            {
                if (_rc6 != value)
                {
                    _rc6 = value;
                    OnPropertyChanged(() => RC6);
                }
            }
        }

        public double RollAngle
        {
            get { return _roll / 20.0f;  }
        }

        private bool? _isArmed;
        public bool IsArmed
        {
            get { return !_isArmed.HasValue ? false : _isArmed.Value; }
            set
            {
                if (!_isArmed.HasValue || _isArmed.Value != value)
                {
                    _isArmed = value;
                    OnPropertyChanged(() => IsArmed);
                }
            }
        }

        private bool? _stableMode;
        public bool StableMode
        {
            get { return !_stableMode.HasValue ? false : _stableMode.Value; }
            set
            {
                if (!_stableMode.HasValue || _stableMode.Value != value)
                {
                    _stableMode = value;
                    OnPropertyChanged(() => StableMode);
                    OnPropertyChanged(() => StabilityMode);
                }
            }
        }


        public String StabilityMode
        {
            get { return _stableMode.HasValue && _stableMode.Value ? "Stable" : "Acro";  }
        }

        public void Update(BufferReader rdr)
        {
            var statusByte = rdr.ReadByte();
            IsArmed = ((statusByte & 0x01) == 0x01);
            StableMode = ((statusByte & 0x02) == 0x02);

            var copterState = rdr.ReadByte();
            var battery = rdr.ReadByte();

            Roll = rdr.ReadShort();
            Pitch = rdr.ReadShort();

            Motor1 = rdr.ReadShort();
            Motor2 = rdr.ReadShort();
            Motor3 = rdr.ReadShort();
            Motor4 = rdr.ReadShort();

            RC1 = rdr.ReadShort();
            RC2 = rdr.ReadShort();
            RC3 = rdr.ReadShort();
            RC4 = rdr.ReadShort();
            RC5 = rdr.ReadShort();
            RC6 = rdr.ReadShort();

        }

        public static SensorMinimal Create(BufferReader rdr)
        {
            var snsr = new SensorMinimal();
            snsr.Update(rdr);
            return snsr;
        }
    }
}
