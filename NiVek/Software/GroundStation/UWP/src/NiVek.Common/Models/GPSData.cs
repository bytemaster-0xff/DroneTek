using LagoVista.Core.ViewModels;
using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class GPSData : ViewModelBase
    {
        const int ALTITUDE_BYTE_START = 27;
        const int HEADING_BYTE_START = 25;

        private bool _validFix;
        public bool ValidFix { get { return _validFix; } set { Set(ref _validFix, value); } }

        private double _latitude;
        public double Latitude { get { return _latitude; } set { Set(ref _latitude, value); } }
        private double _longitude;
        public double Longitude { get { return _longitude; } set { Set(ref _longitude, value); } }
        private double _altitude;
        public double Altitude { get { return _altitude; } set { Set(ref _altitude, value); } }
        private double _heading;
        public double Heading { get { return _heading; } set { Set(ref _heading, value); } }
        private string _satellitesUsed;
        public String SatellitesUsed { get { return _satellitesUsed; } set { Set(ref _satellitesUsed, value); } }
        private String _fixType;
        public String FixType
        {
            get { return _fixType; }
            set
            {
                Set(ref _fixType, value);
                if (value != _fixType)
                {
                    RaisePropertyChanged(() => FixTypeDisplay);
                    RaisePropertyChanged(() => BackgroundColor);
                    RaisePropertyChanged(() => ForgroundColor);
                }
            }
        }

        private bool _isDataReady;
        public bool IsDataReady { get { return _isDataReady; } set { Set(ref _isDataReady, value); } }

        public static GPSData Create(byte[] buffer)
        {
            var data = new GPSData();
            data.Update(buffer);
            return data;

            //var snsrUpdate = new SensorUpdate();

        }

        public void Update(byte[] buffer)
        {
            try
            {
                var temp = System.Text.UTF8Encoding.UTF8.GetString(buffer, 0, 9);

                SatellitesUsed = System.Text.UTF8Encoding.UTF8.GetString(buffer, 22, 2);
                FixType = System.Text.UTF8Encoding.UTF8.GetString(buffer, 24, 1);
                ValidFix = FixType != "0";

                if (ValidFix)
                {
                    Latitude = Convert.ToDouble(temp.Substring(0, 2));
                    Latitude += (double.Parse(temp.Substring(2)) / 60.0);

                    temp = System.Text.UTF8Encoding.UTF8.GetString(buffer, 11, 10);

                    if (buffer[10] == 'S')
                        Latitude *= -1.0;

                    Longitude = Convert.ToDouble(temp.Substring(0, 3));
                    Longitude += (double.Parse(temp.Substring(3)) / 60.0);

                    if (buffer[21] == 'W')
                        Longitude *= -1.0;

                    Altitude = ((UInt16)(buffer[ALTITUDE_BYTE_START] << 8 | buffer[ALTITUDE_BYTE_START + 1])) / 10.0f;
                    Heading = (UInt16)(buffer[HEADING_BYTE_START] << 8 | buffer[HEADING_BYTE_START + 1]);
                }
                else
                {
                    Latitude = 0.0;
                    Longitude = 0.0;
                    Altitude = 0;
                    Heading = 0;
                }

                IsDataReady = true;
            }
            catch (Exception)
            {
                IsDataReady = false;
            }

        }

        public String FixTypeDisplay
        {
            get
            {
                switch (FixType)
                {
                    case "0": return "No Fix";
                    case "1": return "GPS";
                    case "2": return "DGPS";
                    case "3": return "PPS Fix";
                    case "4": return "RTK";
                    case "5": return "Flt RTK";
                    case "6": return "Est";
                    case "7": return "Man";
                    case "8": return "Sim";
                }

                return String.Empty;
            }
        }

        public UInt32 BackgroundColor
        {
            get
            {
                if (!IsDataReady)
                    return 0xFF800000;                    
            
                if (ValidFix)
                    return 0xFF000080;

                return 0xFFFFFF00;
            }

        }

        public UInt32 ForgroundColor
        {
            get
            {
                if (!IsDataReady)
                    return 0xFF000000;

                if (ValidFix)
                    return 0xFFFFFFFF;

                return 0xFF000000;
            }
        }
    }
}
