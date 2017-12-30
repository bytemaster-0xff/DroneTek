using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class BlackBoxRecord
    {
        public static BlackBoxRecord Create(String data)
        {
            var record = new BlackBoxRecord();

            var parts = data.Split(',');

            if (parts.Count() == 1)
                return null;
            try
            {
                record.TimeStamp = Convert.ToDouble(parts[1]);

                record.Pitch = Convert.ToDouble(parts[2]);
                record.Roll = Convert.ToDouble(parts[3]);
                record.Heading = Convert.ToDouble(parts[4]);
                record.Altitude = Convert.ToDouble(parts[5]);

                record.TargetPitch = Convert.ToDouble(parts[6]);
                record.TargetRoll = Convert.ToDouble(parts[7]);
                record.TargetAltitude = Convert.ToDouble(parts[8]);
                record.TargetHeading = Convert.ToDouble(parts[9]);

                record.Throttle = Convert.ToInt16(parts[10]);
                record.Motor1 = Convert.ToInt16(parts[11]);
                record.Motor2 = Convert.ToInt16(parts[12]);
                record.Motor3 = Convert.ToInt16(parts[13]);
                record.Motor4 = Convert.ToInt16(parts[14]);

                if(parts[15] != "?")
                    record.Latitude = Convert.ToDouble(parts[15]);

                if(parts[16] != "?")
                    record.Longitude = Convert.ToDouble(parts[16]);

                return record;
            }
            catch (Exception)
            {
                return null;
            }
        }

        public Double TimeStamp { get; set; }

        public double Altitude { get; set; }
        public double Pitch { get; set; }
        public double Roll { get; set; }
        public double Heading { get; set; }

        public double TargetAltitude { get; set; }
        public double TargetHeading { get; set; }
        public double TargetPitch { get; set; }
        public double TargetRoll { get; set; }

        public short Motor1 { get; set; }
        public short Motor2 { get; set; }
        public short Motor3 { get; set; }
        public short Motor4 { get; set; }
        public short Throttle { get; set; }

        public double Latitude { get; set; }
        public double Longitude { get; set; }

    }
}
