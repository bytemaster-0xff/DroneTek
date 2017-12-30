using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class SensorFusion : NiVekPage
    {
        public SensorFusion()
        {
            this.InitializeComponent();
            this.Unloaded += SensorFusion_Unloaded;

            MeasureType = ConfigBase.MeasureTypeNone;
        }

        public byte MeasureType { get; set; }

        async  void SensorFusion_Unloaded(object sender, RoutedEventArgs e)
        {
            if (Drone.Channel.ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected)
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_EndSensorFusionDiag);
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if (status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_BeginSensorFusionDiag);
                var sensorConfig = await Drone.GetAsync<NiVek.Common.Models.ComplementaryFilter>(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_ReadSensorFusionConfig, IncomingMessage.SensorFusionConfig);
                SensorFusionConfig.DataContext = sensorConfig;
                MeasureTypeSelection.SelectedIndex = 0;
                sensorConfig.IsBound = true;
            }                
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {                
                case IncomingMessage.SensorConfigUpdated:
                    var snsrConfig = (NiVek.Common.Models.ComplementaryFilter)SensorFusionConfig.DataContext;
                    snsrConfig.UpdateApplied(msg.Payload[0] << 8 | msg.Payload[1]);
                    break;
                case IncomingMessage.SensorFusionDiag:
                    var dataPoint = new NiVek.Common.Models.SensorFusionDiag(msg.Payload);
                    var selectedItem = MeasureTypeSelection.SelectedItem as NiVek.Common.Models.ConfigBase.SettingsValue;
                    if(selectedItem != null && selectedItem.Value == ConfigBase.MeasureTypeAltitude)
                        Chart.AddDataPoint(dataPoint.CombinedValue + 5, dataPoint.Sensor1Value + 5, dataPoint.Sensor2Value + 5);
                    else
                        Chart.AddDataPoint(dataPoint.CombinedValue, dataPoint.Sensor1Value, dataPoint.Sensor2Value);

                    break;
            }
        }


        private async void MeasureType_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                PitchConfig.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                RollConfig.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                HeadingConfig.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                AltitudeConfig.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                var item = e.AddedItems[0] as NiVek.Common.Models.ConfigBase.SettingsValue;
                switch (item.Value)
                {
                    case ConfigBase.MeasureTypeNone:
                        MeasureType = ConfigBase.MeasureTypeNone;
                        await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_EndSensorFusionDiag, ExpectACK = true });
                        break;
                    case ConfigBase.MeasureTypePitch:
                        Chart.ChartAxis[0].Scale = 90;
                        Chart.ChartAxis[1].Scale = 90;
                        Chart.ChartAxis[2].Scale = 90;

                        Chart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                        Chart.ChartAxis[1].Origin = Controls.LineOrigin.Center;
                        Chart.ChartAxis[2].Origin = Controls.LineOrigin.Center;

                        MeasureType = ConfigBase.MeasureTypePitch;
                        PitchConfig.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                        break;
                    case ConfigBase.MeasureTypeRoll:
                        Chart.ChartAxis[0].Scale = 90;
                        Chart.ChartAxis[1].Scale = 90;
                        Chart.ChartAxis[2].Scale = 90;
                        Chart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                        Chart.ChartAxis[1].Origin = Controls.LineOrigin.Center;
                        Chart.ChartAxis[2].Origin = Controls.LineOrigin.Center;

                        MeasureType = ConfigBase.MeasureTypeRoll;
                        RollConfig.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                        break;
                    case ConfigBase.MeasureTypeHeading:
                        Chart.ChartAxis[0].Scale = 360;
                        Chart.ChartAxis[1].Scale = 360;
                        Chart.ChartAxis[2].Scale = 360;

                        Chart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;
                        Chart.ChartAxis[1].Origin = Controls.LineOrigin.Bottom;
                        Chart.ChartAxis[2].Origin = Controls.LineOrigin.Bottom;

                        MeasureType = ConfigBase.MeasureTypeHeading;
                        HeadingConfig.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                        break;
                    case ConfigBase.MeasureTypeAltitude:
                        Chart.ChartAxis[0].Scale = 20;
                        Chart.ChartAxis[1].Scale = 20;
                        Chart.ChartAxis[2].Scale = 20;

                        Chart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;
                        Chart.ChartAxis[1].Origin = Controls.LineOrigin.Bottom;
                        Chart.ChartAxis[2].Origin = Controls.LineOrigin.Bottom;

                        MeasureType = ConfigBase.MeasureTypeAltitude;
                        AltitudeConfig.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                        break;

                }
            }
        }

        private void PowerOff_Click(object sender, RoutedEventArgs e)
        {
            Drone.ThrottleInput = 0;
            Drone.Safe();
        }

        private void Power20_Click(object sender, RoutedEventArgs e)
        {
            Drone.Arm();
            Drone.ThrottleInput = Convert.ToByte(0.20 * 255.0);
        }

        private void Power30_Click(object sender, RoutedEventArgs e)
        {
            Drone.Arm();
            Drone.ThrottleInput = Convert.ToByte(0.30 * 255.0); ;
        }

        private void Power40_Click(object sender, RoutedEventArgs e)
        {
            Drone.Arm();
            Drone.ThrottleInput = Convert.ToByte(0.40 * 255.0); ;
        }

        protected override void OnNavigatedTo(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {            
            base.OnNavigatedTo(e);
            Chart.Start();

            Header.DataContext = Drone;
        }

        protected override void OnNavigatedFrom(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            Chart.Stop();
        }
    }
}
