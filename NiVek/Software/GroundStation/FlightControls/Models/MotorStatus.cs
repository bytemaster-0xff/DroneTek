using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class MotorStatus
    {
        public static MotorStatus Create(byte[] buffer)
        {

            try
            {
                var motorStatus = new MotorStatus();

                var byteIndex = 0;

                motorStatus.PowerPortFront = buffer[byteIndex++];
                motorStatus.PowerPortRear = buffer[byteIndex++];
                motorStatus.PowerStarboardFront = buffer[byteIndex++];
                motorStatus.PowerStarboardRear = buffer[byteIndex++];

                return motorStatus;
            }
            catch (Exception)
            {
                return null;
            }
        }

        public short PowerPortFront { get; set; }
        public string PowerPortFrontPct { get { return String.Format("{0}%", PowerPortFront * 100 / 255); } }
        public short PowerPortRear { get; set; }
        public string PowerPortRearPct { get { return String.Format("{0}%", PowerPortRear * 100 / 255); } }
        public short PowerStarboardRear { get; set; }
        public string PowerStarboardRearPct { get { return String.Format("{0}%", PowerStarboardRear * 100 / 255); } }
        public short PowerStarboardFront { get; set; }
        public string PowerStarboardFrontPct { get { return String.Format("{0}%", PowerStarboardFront * 100 / 255); } }
    }
}
