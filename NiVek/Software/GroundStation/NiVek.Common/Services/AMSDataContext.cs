using Microsoft.WindowsAzure.MobileServices;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public class AMSDataContext
    {
        private static AMSDataContext _instance = null;
        private const string ACCOUNT_NAME = "nivek";
        private const string AMS_KEY = "tpfUjlETAyAGrnGriaDyKnYTewtQCE50";

        private MobileServiceClient _mobileServiceClient;
        public MobileServiceClient AMSClient { get { return _mobileServiceClient; } }


        private AMSDataContext() { }

        public static AMSDataContext Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new AMSDataContext();
                    _instance._mobileServiceClient = new MobileServiceClient(String.Format("https://{0}.azure-mobile.net/", ACCOUNT_NAME), AMS_KEY);
                    _instance._droneComms = _instance._mobileServiceClient.GetTable<Models.DroneComms>();
                    _instance._drones = _instance._mobileServiceClient.GetTable<Comms.Drone>();
                    _instance._flights = _instance._mobileServiceClient.GetTable<Models.Flight>();
                }

                return _instance;
            }
        }

        private IMobileServiceTable<Models.DroneComms> _droneComms;
        public IMobileServiceTable<Models.DroneComms> DroneComms { get { return _droneComms; } }

        private IMobileServiceTable<Comms.Drone> _drones;
        public IMobileServiceTable<Comms.Drone> Drones { get { return _drones; } }

        private IMobileServiceTable<Models.Flight> _flights;
        public IMobileServiceTable<Models.Flight> Flights { get { return _flights; } }
    }
}
