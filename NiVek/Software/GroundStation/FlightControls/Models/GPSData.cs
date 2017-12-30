using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class GPSData
    {
        public bool ValidFix { get; set; }
        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public String SatellitesUsed { get; set; }

        public static GPSData Create(byte[] buffer)
        {
            var snsrUpdate = new SensorUpdate();

            try
            {
                var data = new GPSData();

                var temp = System.Text.UTF8Encoding.UTF8.GetString(buffer, 0, 9);
                
                if(!temp.StartsWith("?"))
                {
                    data.ValidFix = true;
                    data.Latitude = Convert.ToDouble(temp.Substring(0, 2));
                    data.Latitude += (double.Parse(temp.Substring(2)) / 60.0);                    

                    temp = System.Text.UTF8Encoding.UTF8.GetString(buffer, 11, 10);


                    if(buffer[10] == 'S')
                        data.Latitude *= -1.0;

                    data.Longitude = Convert.ToDouble(temp.Substring(0, 3));
                    data.Longitude += (double.Parse(temp.Substring(3)) / 60.0);                    
                    
                    if(buffer[21] == 'W')
                        data.Longitude *= -1.0;

                    data.SatellitesUsed = System.Text.UTF8Encoding.UTF8.GetString(buffer, 22, 2);
                }
                else{
                    data.ValidFix = false;
                    data.Latitude =  0.0;
                    data.Longitude = 0.0;

                }
                return data;

            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
