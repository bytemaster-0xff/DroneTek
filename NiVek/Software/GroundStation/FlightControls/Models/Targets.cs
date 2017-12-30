using System;
using System.Collections.Generic;
using System.Linq;

namespace NiVek.FlightControls.Models
{
    public class Targets
    {
        public static Targets Create(byte[] buffer)
        {
            try
            {
                var target = new Targets();

                var byteIndex = 0;
                target.TargetHeading = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                target.TargetAltitude = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);

                target.ThrottleIn = buffer[byteIndex++];
                target.PitchIn = (sbyte)buffer[byteIndex++];
                target.RollIn = (sbyte)buffer[byteIndex++];
                target.YawIn = (sbyte)buffer[byteIndex++];

                return target;
            }
            catch(Exception)
            {
                return null;
            }
        }

        public short TargetHeading { get; set; }
        public short TargetAltitude { get; set; }

        private double _yawIn;
        private double _rollIn;
        private double _pitchIn;
        private double _throttleIn;
        public double ThrottleIn
        {
            get
            {
                if (_throttleIn > 255) return 255;
                if (_throttleIn < 0) return 0;
                return _throttleIn;
            }
            set
            {
                _throttleIn = value;
            }
        }
        public double PitchIn
        {
            get
            {
                if (_pitchIn > 90) return 90;
                if (_pitchIn < -90) return -90;
                return _pitchIn;
            }
            set
            {
                _pitchIn = value;
            }
        }
        public double RollIn
        {
            get
            {
                if (_rollIn > 90) return 90;
                if (_rollIn < -90) return -90;
                return _rollIn;
            }
            set
            {
                _rollIn = value;
            }
        }
        public double YawIn
        {
            get
            {
                if (_yawIn > 90) return 90;
                if (_yawIn < -90) return -90;
                return _yawIn;
            }
            set
            {
                _yawIn = value;
            }
        }
    }
}
