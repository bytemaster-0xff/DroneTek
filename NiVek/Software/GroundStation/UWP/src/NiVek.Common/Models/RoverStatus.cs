using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class RoverStatus : ModelBase
    {
        public static RoverStatus Create(byte[] buffer)
        {
            var status = new RoverStatus();
            status.Update(buffer);
            return status;
        }

        public void Update(byte[] buffer)
        {
            var idx = 0;
            Pan = Convert.ToInt16(buffer[idx++] | buffer[idx++] << 8);
            Tilt = Convert.ToInt16(buffer[idx++] | buffer[idx++] << 8);

            FrontSonar = Convert.ToUInt16(buffer[idx++] | buffer[idx++] << 8);
            RearSonar = Convert.ToUInt16(buffer[idx++] | buffer[idx++] << 8);

            LeftIR = Convert.ToUInt16(buffer[idx++] | buffer[idx++] << 8);
            RightIR = Convert.ToUInt16(buffer[idx++] | buffer[idx++] << 8);

            Heading = Convert.ToUInt16(buffer[idx++] | buffer[idx++] << 8);

            RaisePropertyChanged(() => FrontSonarPixels);
            RaisePropertyChanged(() => RearSonarPixels);

            RaisePropertyChanged(() => RightIRPixels);
            RaisePropertyChanged(() => LeftIRPixels);
        }

        private short _pan;
        public short Pan
        {
            get { return _pan; }
            set { 
                Set(ref _pan, value);
                RaisePropertyChanged(() => PanAngle);
            }
        }

        public int PanAngle
        {
            get
            {
                var delta = Pan - 1400;
                return Convert.ToInt32((delta / 800.0f) * 85);
            }

        }

        private short _tilt;
        public short Tilt
        {
            get { return _tilt; }
            set { Set(ref _tilt, value); }
        }

        private UInt16 _heading;
        public UInt16 Heading
        {
            get { return _heading; }
            set { Set(ref _heading, value); }
        }

        private UInt16 _frontSonar;
        public UInt16 FrontSonar
        {
            get { return _frontSonar; }
            set { Set(ref _frontSonar, value); }
        }

        private UInt16 _rearSonar;
        public UInt16 RearSonar
        {
            get { return _rearSonar; }
            set { Set(ref _rearSonar, value); }
        }

        private UInt16 _leftIR;
        public UInt16 LeftIR
        {
            get { return _leftIR; }
            set { Set(ref _leftIR, value); }
        }

        private UInt16 _rightIR;
        public UInt16 RightIR
        {
            get { return _rightIR; }
            set { Set(ref _rightIR, value); }
        }

        public double FrontSonarPixels
        {
            get { return Convert.ToDouble(FrontSonar); }
        }
        public double RearSonarPixels
        {
            get { return Convert.ToDouble(RearSonar ); }
        }


        public double LeftIRPixels
        {
            get { return Convert.ToDouble(LeftIR ); }
        }


        public double RightIRPixels
        {
            get { return Convert.ToDouble(RightIR); }
        }

    }
}
