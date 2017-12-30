using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class PIDSensorData
    {
        public static PIDSensorData Create(byte[] buffer)
        {
            try
            {
                var pid = new PIDSensorData();
                var byteIndex = 0;
                pid.PitchAngle = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                pid.RollAngle = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                pid.Heading = (UInt16)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                pid.AltitudeCM = (UInt16)(buffer[byteIndex++] | buffer[byteIndex++] << 8);

                pid.TargetRoll = (sbyte)buffer[byteIndex++];
                pid.TargetPitch = (sbyte)buffer[byteIndex++];
                pid.TargetAltitude = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                pid.TargetHeading = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8);

                pid.PowerPortFront = buffer[byteIndex++];
                pid.PowerPortRear = buffer[byteIndex++];
                pid.PowerStarboardFront = buffer[byteIndex++];
                pid.PowerStarboardRear = buffer[byteIndex++];

                pid.ErrorPitch = pid.TargetPitch - pid.PitchAngle;
                pid.ErrorRoll = pid.TargetRoll - pid.RollAngle;

                pid.ErrorAltitude = pid.TargetAltitude - pid.AltitudeCM;
                pid.ErrorHeading = pid.TargetHeading - pid.Heading;


                return pid;
            }
            catch (Exception)
            {
                return null;
            }
        }


        public double X { get; set; }

        public double PitchAngle { get; set; }
        public double RollAngle { get; set; }
        public double Heading { get; set; }
        public double AltitudeCM { get; set; }

        public short TargetHeading { get; set; }
        public short TargetAltitude { get; set; }
        public short TargetPitch { get; set; }
        public short TargetRoll { get; set; }

        public short PowerFront { get { return PowerPortFront; } }
        public short PowerPort { get { return PowerPortRear; } }
        public short PowerStarboard { get { return PowerStarboardFront; } }
        public short PowerRear { get { return PowerStarboardFront; } }

        public string PowerFrontPct { get { return PowerPortFrontPct; } }
        public string PowerPortPct { get { return PowerPortRearPct; } }
        public string PowerStarboardPct { get { return PowerStarboardFrontPct; } }
        public string PowerRearPct { get { return PowerStarboardFrontPct; } }


        public short PowerPortFront { get; set; }
        public string PowerPortFrontPct { get { return String.Format("{0}%", PowerPortFront * 100 / 255); } }
        public short PowerPortRear { get; set; }
        public string PowerPortRearPct { get { return String.Format("{0}%", PowerPortRear * 100 / 255); } }
        public short PowerStarboardRear { get; set; }
        public string PowerStarboardRearPct { get { return String.Format("{0}%", PowerStarboardRear * 100 / 255); } }
        public short PowerStarboardFront { get; set; }
        public string PowerStarboardFrontPct { get { return String.Format("{0}%", PowerStarboardFront * 100 / 255); } }

        public double ErrorPitch { get; set; }
        public double ErrorRoll { get; set; }
        public double ErrorHeading { get; set; }
        public double ErrorAltitude { get; set; }
    }
}
