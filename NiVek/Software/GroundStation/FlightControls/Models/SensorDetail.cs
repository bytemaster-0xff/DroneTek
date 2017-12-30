using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class Sensors
    {
        public const byte None = 0x00;
        public const byte ITG3200 = 0x20;
        public const byte MPU60x0GYRO = 0x21;
        public const byte L3GD20 = 0x22;

        public const byte LSM303_ACC = 0x30;
        public const byte ADXL345 = 0x31;
        public const byte MPU60x0ACC = 0x32;

        public const byte LSM303_MAG = 0x40;
        public const byte HMC5883 = 0x41;
        public const byte MagGPS = 0x42;

        public const byte BMP085 = 0x50;
        public const byte HCSR04 = 0x51;        
        public const byte AltGPS = 0x52;

        public const byte LIPO_ADC = 0x60;

        public const byte GeoGPS = 0x70;

        public const byte SnsrFusionComplementary = 0x80;
        public const byte SnsrFusionKalman = 0x81;

        public const byte PIDController = 0x90;
    }

    public class SensorDetail
    {
        public static SensorDetail Create(byte[] buffer, bool round)
        {
            try
            {
                byte byteIndex = 0;
                var detail = new SensorDetail();
                detail.RawX = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                detail.RawY = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8);
                detail.RawZ = (Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8);

                detail.X = ((Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0);
                detail.Y = ((Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0);
                detail.Z = ((Int16)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0);

                if (round)
                {
                    detail.X = Math.Round(detail.X * 2, MidpointRounding.AwayFromZero) / 2.0;
                    detail.Y = Math.Round(detail.Y * 2, MidpointRounding.AwayFromZero) / 2.0;
                    detail.Z = Math.Round(detail.Z * 2, MidpointRounding.AwayFromZero) / 2.0;
                }

                detail.SensorId = buffer[byteIndex++];

                return detail;
            }
            catch (Exception)
            {
                Debug.WriteLine("Error pulling data.");
            }

            return null;

        }

        public static SensorDetail Create(byte[] buffer)
        {
            return SensorDetail.Create(buffer, true);
        }

        public Int16 RawX { get; set; }
        public Int16 RawY { get; set; }
        public Int16 RawZ { get; set; }

        public double X { get; set; }
        public double Y { get; set; }
        public double Z { get; set; }

        public byte SensorId { get; set; }
    }
}
