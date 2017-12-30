using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;

namespace NiVek.Common.Models
{
    [DataContract]
    public class SensorUpdate : GalaSoft.MvvmLight.ViewModelBase
    {
        public enum CalibrationStatus
        {
            Calibrated = 0xFF,
            NotCalibrated = 0x00
        };

        public static SensorUpdate Create(byte[] buffer)
        {
            var snsrUpdate = new SensorUpdate();
            snsrUpdate.Update(buffer);

            return snsrUpdate;
        }

        public void Update(byte[] buffer)
        {
            try
            {
                var byteIndex = 0;

                PitchAngle = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                RollAngle = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                Heading = (UInt16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                AltitudeM = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;

                PitchAngle = Math.Round(PitchAngle * 2, MidpointRounding.AwayFromZero) / 2.0;
                RollAngle = Math.Round(RollAngle * 2, MidpointRounding.AwayFromZero) / 2.0;
                IsReady = true;
            }
            catch (Exception)
            {
                IsReady = false;
            }

        }

        private bool _isReady;
        public bool IsReady { get { return _isReady; } set { Set(ref _isReady, value); } }


        public CalibrationStatus PWMCalibrated { get; private set; }

        public float AltitudeM { get; set; }

        public float X { get; set; }

        public String Latitude { get; set; }

        public String Longitude { get; set; }

        public int Satellites { get; set; }

        public int ValidFix { get; set; }

        public string DataType { get; set; }

        public String Message { get; set; }

        public double RollAngle { get; set; }

        public double AngXCopter { get { return Math.Round(-RollAngle, 0); } }
        public double AngYCopter { get { return Math.Round(PitchAngle - 90.0, 0); } }

        static double _priorAttitudeRollAngle;
        public double AttitudeRollAngle
        {
            get
            {
                _priorAttitudeRollAngle = (RollAngle * 0.1f) + (_priorAttitudeRollAngle * 0.9f);

                return _priorAttitudeRollAngle;
            }
        }

        static double _priorAttitudePitchAngle;
        public double AttitudePitchAngle
        {
            get
            {
                _priorAttitudePitchAngle = (PitchAngle * 0.1f) + (_priorAttitudePitchAngle * 0.9f);

                return _priorAttitudePitchAngle;
            }
        }

        static double _priorPitchPixels;
        public double PitchPixelOffset
        {
            get
            {
                var currentValue = PitchAngle * 33.0f / 5.0f;
                _priorPitchPixels = _priorPitchPixels * 0.9 + currentValue * 0.1;
                return _priorPitchPixels;
            }
        }

        public int ZIndexTop { get { return Math.Abs(AngXCopter) > 90.0 ? 0 : 1; } }

        public int ZIndexBottom { get { return Math.Abs(AngXCopter) > 90.0 ? 1 : 0; } }

        public double PitchAngle { get; set; }
        public double YawAngle { get; set; }


        public double Interval { get; set; }




        public double Heading { get; set; }
        public double CompassRotation { get { return (double)-Heading; } }

        //public Thickness CompassOffset { get { return new Thickness(-this.Heading * 2000 / 360, 0, 0, 0); } }
    }



    public class SensorsUpdatedEventArgs : EventArgs
    {
        public SensorsUpdatedEventArgs(SensorUpdate data)
        {
            SensorData = data;
        }

        public SensorUpdate SensorData { get; private set; }
    }

}
