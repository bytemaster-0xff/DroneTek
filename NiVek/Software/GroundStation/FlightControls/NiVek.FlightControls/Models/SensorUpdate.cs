using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    [DataContract]
    public class SensorUpdate
    {
        public float X { get; set; }

        [DataMember(Name = "lat")]
        public String Latitude { get; set; }

        [DataMember(Name = "lon")]
        public String Longitude { get; set; }

        [DataMember(Name = "satellites")]
        public int Satellites { get; set; }

        [DataMember(Name = "validfix")]
        public int ValidFix { get; set; }

        [DataMember(Name = "dataType")]
        public string DataType { get; set; }

        [DataMember(Name = "angXAcc")]
        public float AccX { get; set; }

        [DataMember(Name = "angYAcc")]
        public float AccY { get; set; }

        [DataMember(Name = "message")]
        public String Message { get; set; }

        [DataMember(Name = "angX")]
        public double AngX { get; set; }

        public double AngXCopter { get { return AngX - 90.0; } }
        public double AngYCopter { get { return -AngY; } }

        public double AngXAOA { get { return AngX * 33/5; } }

        public int ZIndexTop { get { return Math.Abs(AngXCopter) > 90.0 ? 0 : 1; } }

        public int ZIndexBottom { get { return Math.Abs(AngXCopter) > 90.0 ? 1 : 0; } }
        



        [DataMember(Name = "angY")]
        public double AngY { get; set; }

        [DataMember(Name = "angXRate")]
        public double AngXRate { get; set; }

        [DataMember(Name = "angYRate")]
        public double AngYRate { get; set; }

        [DataMember(Name = "angZRate")]
        public double AngZRate { get; set; }

        [DataMember(Name = "int")]
        public double Interval { get; set; }

        [DataMember(Name = "rcin1")]
        public double RCIn1 { get; set; }

        [DataMember(Name = "rcin2")]
        public double RCIn2 { get; set; }

        [DataMember(Name = "rcin3")]
        public double RCIn3 { get; set; }

        [DataMember(Name = "rcin4")]
        public double RCIn4 { get; set; }

/*        public System.Device.Location.GeoCoordinate Location
        {
            get
            {
                if (ValidFix == 0)
                    return null;

                var parts = Latitude.Split(' ');
                var minutes = Convert.ToDouble(parts[1].TrimEnd(new char[] { 'N', 'E', 'S', 'W' }));


                var lat = Convert.ToDouble(parts[0]) + minutes / 60.0;

                parts = Longitude.Split(' ');

                minutes = Convert.ToDouble(parts[1].TrimEnd(new char[] { 'N', 'E', 'S', 'W' }));
                var lon = Convert.ToDouble(parts[0]) + minutes / 60.0;

                return new System.Device.Location.GeoCoordinate(lat, -lon);

            }
        }*/
    }



    public class GPSUpdatedEventArgs : EventArgs
    {
        public GPSUpdatedEventArgs(SensorUpdate data)
        {
            GPSData = data;
        }

        public SensorUpdate GPSData { get; private set; }
    }

}
