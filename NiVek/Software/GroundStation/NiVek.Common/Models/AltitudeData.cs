using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class AltitudeData
    {
        public static AltitudeData Create(byte[] buffer)
        {
            var data = new AltitudeData();

            var byteIndex = 0;

            data.Altitude = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0; /* Sent to one place */
            data.Sonar = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0; /* Sent to two places */
            data.BMP085 = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0; /* Sent to one place */
            data.MS5611 = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0;  /* Sent to one place */
            data.GPS = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0; /* Sent to once place */

            return data;
        }

        public double Altitude { get; private set; }
        public double Sonar { get; private set; }
        public double MS5611 { get; private set; }
        public double BMP085 { get; private set; }
        public double GPS { get; private set; }
    }
}
