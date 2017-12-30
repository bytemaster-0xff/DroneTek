using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI.ApplicationSettings;
using Windows.UI.Xaml;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
using NiVek.WinCommon.Services;

namespace NiVek.FlightControls
{
    public sealed partial class MainPage : Views.NiVekPage
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedFrom(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            Roll.Stop();
            Pitch.Stop();
        }

        protected override void OnNavigatedTo(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            DataContext = DroneHub.Instance.Active;

            Roll.Start();
            Pitch.Start();
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.SensorData:
                    {
                        var data = NiVek.Common.Models.SensorUpdate.Create(msg.Payload);
                        if (data != null)
                        {
                            Pitch.AddDataPoint(data.PitchAngle);
                            Roll.AddDataPoint(data.RollAngle);
                            Attitutde.SensorData = data;
                        }
                    }

                    break;
                case IncomingMessage.SensorGps:
                    var gpsData = NiVek.Common.Models.GPSData.Create(msg.Payload);
                    //if(gpsData.ValidFix)
                      //  NavMap.Center = new Bing.Maps.Location(gpsData.Latitude, gpsData.Longitude);
                    break;
            }
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            
        }
    }
}
