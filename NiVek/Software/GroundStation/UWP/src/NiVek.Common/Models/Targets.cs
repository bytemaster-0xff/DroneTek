using LagoVista.Core.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;

namespace NiVek.Common.Models
{
    public class Targets : ViewModelBase
    {
        public static Targets Create(byte[] buffer)
        {
            var target = new Targets();
            target.Update(buffer);
            return target;
        }

        public void Update(byte[] buffer)
        {
            try
            {
                var byteIndex = 0;
                TargetHeading = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                TargetAltitude = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);

                ThrottleIn = buffer[byteIndex++];
                PitchIn = (sbyte)buffer[byteIndex++];
                RollIn = (sbyte)buffer[byteIndex++];
                YawIn = (sbyte)buffer[byteIndex++];
                IsDataReady = true;
            }
            catch (Exception)
            {
                IsDataReady = false;
            }
        }

        private bool _isDataReady;
        public bool IsDataReady
        {
            get { return _isDataReady; }
            set { Set(ref _isDataReady, value); }
        }

        private short _targetHeading;
        public short TargetHeading { get { return _targetHeading; } set { Set(ref _targetHeading, value); } }
        private short _targetAltitude;
        public short TargetAltitude { get { return _targetHeading; } set { Set(ref _targetAltitude, value); } }

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
                if(_throttleIn != value)
                {
                    _throttleIn = value;
                    RaisePropertyChanged(() => ThrottlePct);
                }
            }
        }

        public int ThrottlePct { get { return (int)Math.Round((_throttleIn / 255.0f) * 100.0f); } }
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
                Set(ref _pitchIn, value);
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
                Set(ref _rollIn, value);
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
                Set(ref _yawIn, value);
            }
        }
    }
}
