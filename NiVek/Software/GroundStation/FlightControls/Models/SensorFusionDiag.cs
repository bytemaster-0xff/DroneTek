using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class SensorFusionDiag
    {
        public SensorFusionDiag(byte[] buffer)
        {
            var byteIndex = 0;

            CombinedValue = ((double)(Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8)) / 10.0;
            Sensor1Value = ((double)(Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8)) / 10.0;
            Sensor2Value = ((double)(Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8)) / 10.0;
        }

        public double CombinedValue { get; set; }
        public double Sensor1Value { get; set; }
        public double Sensor2Value { get; set; }
    }
}
