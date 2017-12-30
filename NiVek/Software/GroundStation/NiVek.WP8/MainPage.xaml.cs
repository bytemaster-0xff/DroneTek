using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using NiVek.WP8.Resources;
using System.Windows.Threading;
using System.Diagnostics;
using System.Windows.Media;
using NiVek.Common.Comms;

namespace NiVek.WP8
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }
        #region Standard Event Handler Stuff
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            App.TheApp.CommChannel.MessageReady += CommChannel_MessageReady;
            App.TheApp.CommChannel.OnConnect += CommChannel_OnConnect;
            App.TheApp.CommChannel.OnDisconnect += CommChannel_OnDisconnect;

            switch (App.TheApp.CommChannel.ConnectionState)
            {
                case NiVek.Common.Comms.Common.ConnectionStates.Connected: ConnectionStatus.Fill = new SolidColorBrush(Colors.Green); break;
                case NiVek.Common.Comms.Common.ConnectionStates.Disconnected: ConnectionStatus.Fill = new SolidColorBrush(Colors.Red); break;
                case NiVek.Common.Comms.Common.ConnectionStates.Disconnecting: ConnectionStatus.Fill = new SolidColorBrush(Colors.Yellow); break;
                //case Common.Comms.Common.ConnectionStates.ConnectionEstablished: ConnectionStatus.Fill = new SolidColorBrush(Colors.Green); break;
                case NiVek.Common.Comms.Common.ConnectionStates.Connecting: ConnectionStatus.Fill = new SolidColorBrush(Colors.Yellow); break;
            }
        }

        void CommChannel_OnDisconnect(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke(() =>
            {
                ConnectionStatus.Fill = new SolidColorBrush(Colors.Red);
            });
        }

        void CommChannel_OnConnecting(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke(() =>
            {
                ConnectionStatus.Fill = new SolidColorBrush(Colors.Yellow);
            });
        }

        void CommChannel_OnFailedToConnected(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke(() =>
            {
                ConnectionStatus.Fill = new SolidColorBrush(Colors.Red);
            });
        }

        void CommChannel_OnConnect(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke(() =>
            {
                ConnectionStatus.Fill = new SolidColorBrush(Colors.Green);

            });
        }
        #endregion

        void CommChannel_MessageReady(object sender, IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.SensorData:
                    {
                        var data = NiVek.Common.Models.SensorUpdate.Create(msg.Payload);
                        if (data != null)
                        {
                            Debug.WriteLine("HEADING " + data.Heading);

/*                            Pitch.AddDataPoint(data.PitchAngle);
                            Roll.AddDataPoint(data.RollAngle);
                            Attitutde.SensorData = data;
                            //CopterOrientation.SensorData = data;
                            CompassDisplay.DataContext = data;*/
                            //CopterOrientation.DataContext = data;
                        }
                    }

                    break;
                case IncomingMessage.SensorGps:
                    {
                        /*var data = GPSData.Create(msg.Payload);

                        if (data != null && data.ValidFix)
                        {
                            NavMap.Center = new Bing.Maps.Location(data.Latitude, data.Longitude);
                            NavMap.ZoomLevel = 24;
                            NavMap.Visibility = Visibility.Visible;
                        }
                        else
                        {
                            NavMap.Visibility = Visibility.Collapsed;
                        }*/
                    }

                    break;
            } 
        }


        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            App.TheApp.CommChannel.MessageReady -= CommChannel_MessageReady;
            App.TheApp.CommChannel.OnConnect -= CommChannel_OnConnect;
            App.TheApp.CommChannel.OnDisconnect -= CommChannel_OnDisconnect;
        }

        private void Connect_Click(object sender, RoutedEventArgs e)
        {
            App.TheApp.CommChannel.Connect("1.2.3.4", 9050);
        }
    }
}