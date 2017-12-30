using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.ApplicationSettings;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    public sealed partial class AltutidePID : NiVekPage
    {
        public AltutidePID()
        {
            this.InitializeComponent();
        }

        protected override void MessageArrived(NiVek.Common.Comms.IncomingMessage msg)
        {
     
            switch (msg.MessageId)
            {
                case IncomingMessage.AltitudeData:
                    {
                        var sensorData = NiVek.Common.Models.AltitudeData.Create(msg.Payload);
                        AltitudeChart.AddDataPoint(sensorData.MS5611 + 5);
                    }
                    break;
            }
        }

        void _pid_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if(status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                DataContext = await Drone.GetAsync<NiVek.Common.Models.GPIOConfig>(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.GPIOPidConstants);
            }
        }

        private void TargetAltitude_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }
    }
}
