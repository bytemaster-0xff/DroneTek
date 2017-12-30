using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class MultiWiiPID
    {
        public class PID : ModelBase
        {
            private byte _p;
            public byte P
            {
                get { return _p;  }
                set
                {
                    if(_p != value)
                    {
                        _p = value;
                        OnPropertyChanged(() => P);
                    }
                }
            }

            private byte _i;
            public byte I
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

            private byte _d;
            public byte D
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

        public PID Pitch { get; set; }
        public PID Roll { get; set; }
        public PID Yaw { get; set; }

        public PID Level { get; set; }

        public void Update(BufferReader rdr)
        {
            Pitch.Update(rdr);
            Roll.Update(rdr);
            Yaw.Update(rdr);
            Level.Update(rdr);
        }
    
        public static MultiWiiPID Create(BufferReader rdr)
        {
            var pid = new MultiWiiPID();
            pid.Pitch = new PID();
            pid.Roll = new PID();
            pid.Yaw = new PID();
            pid.Level = new PID();

            pid.Update(rdr);

            return pid;
        }
    
    }
}
