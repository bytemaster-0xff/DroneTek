using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class CompassConfig : NiVekPage
    {
        public CompassConfig()
        {
            this.InitializeComponent();

            this.Unloaded += CompassConfig_Unloaded;
        }

        async void CompassConfig_Unloaded(object sender, RoutedEventArgs e)
        {
            if (Drone.Channel.ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected)
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_EndSensorDiagnostics);
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            RawXChart.Start();
            RawYChart.Start();
            RawZChart.Start();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            RawXChart.Stop();
            RawYChart.Stop();
            RawZChart.Stop();
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if (status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                var sensorConfig = await Drone.GetAsync<NiVek.Common.Models.SensorConfig>(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_ReadCfgValues, IncomingMessage.SensorConfigSettings);
                DataContext = sensorConfig;
                
                var msg = new OutgoingMessage() {ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorDiagnostics, ExpectACK = true };
                msg.AddByte(NiVek.Common.Models.Sensors.LSM303_MAG);
                await Drone.SendMessageAsync(msg);
            }            
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.SensorConfigUpdated:
                    var snsrConfig = (NiVek.Common.Models.SensorConfig)CompassSettings.DataContext;
                    snsrConfig.UpdateApplied(msg.Payload[0] << 8 | msg.Payload[1]);
                    break;


                case IncomingMessage.SensorDiagnostics:
                    var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload, false);

                    try
                    {
                        RawXChart.AddDataPoint(snsrData.X / 10.0f, snsrData.RawX);
                        RawYChart.AddDataPoint(snsrData.Y / 10.0f, snsrData.RawY);
                        RawZChart.AddDataPoint(snsrData.Z / 10.0f, snsrData.RawZ);
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine("EX-> " + ex.Message);
                        Debug.WriteLine(ex.StackTrace);                        
                    }
                    break;
            }
        }

        private async void CancelCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_CancelMagCalibration);

            BeginCalibrationXY.Visibility = Windows.UI.Xaml.Visibility.Visible;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            await Task.Delay(50);

            var msg = new OutgoingMessage() {ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorDiagnostics, ExpectACK = true };
            msg.AddByte(NiVek.Common.Models.Sensors.LSM303_MAG);
            await Drone.SendMessageAsync(msg);
        }

        private async void EndCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            BeginCalibrationXY.Visibility = Windows.UI.Xaml.Visibility.Visible;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_EndMagCalibration);

            await Task.Delay(50);

            var msg = new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorDiagnostics, ExpectACK = true };
            msg.AddByte(NiVek.Common.Models.Sensors.LSM303_MAG);
            await Drone.SendMessageAsync(msg);

        }

        private async void BeginCalibrationXY_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_BeginMagCalibrationXY);

            BeginCalibrationXY.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
        }
    }
}
