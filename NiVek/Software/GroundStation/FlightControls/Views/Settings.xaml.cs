using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System.Threading;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    public sealed partial class Settings : Views.NiVekPage
    {
        public Settings()
        {
            this.InitializeComponent();
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if (status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                var sensorConfig = await Drone.GetAsync<NiVek.Common.Models.SensorConfig>(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_ReadCfgValues, IncomingMessage.SensorConfigSettings);
                DataContext = sensorConfig;
            }             
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {                
                case IncomingMessage.SensorConfigUpdated:
                    var snsrConfig = (NiVek.Common.Models.SensorConfig)ConfigSettings.DataContext;
                    snsrConfig.UpdateApplied(msg.Payload[0] << 8 | msg.Payload[1]);
                    break;
            }
        }

        private async void Restart_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_SensorRestart);
        }

        private async void Reset_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_SensorResetDefaults);
        }

    }
}
