using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MicroSensorFusion : NiVekPage
    {
        public MicroSensorFusion()
        {
            this.InitializeComponent();
        }

        protected override void MessageArrived(NiVek.Common.Comms.IncomingMessage msg)
        {
            
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            var sensorConfig = await Drone.GetAsync<NiVek.Common.Models.MicroComplementaryFilter>(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_ReadSensorFusionConfig, IncomingMessage.SensorFusionConfig);
            SensorFusionConfig.DataContext = sensorConfig;
            MeasureTypeSelection.SelectedIndex = 0;        
        }

        protected async override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_EndSensorFusionDiag, ExpectACK = true });
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            
        }

        byte MeasureType = ModelBase.MeasureTypeNone;

        private async void MeasureTypeSelection_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {                
                var item = e.AddedItems[0] as NiVek.Common.Models.ModelBase.SettingsValue;
                if (item.Value != MeasureType)
                {
                    switch (item.Value)
                    {
                        case ModelBase.MeasureTypeNone:
                            MeasureType = ModelBase.MeasureTypeNone;
                            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_EndSensorFusionDiag, ExpectACK = true });
                            break;
                        case ModelBase.MeasureTypePitch:
                            Chart.ChartAxis[0].Scale = 90;
                            Chart.ChartAxis[1].Scale = 90;
                            Chart.ChartAxis[2].Scale = 90;

                            Chart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                            Chart.ChartAxis[1].Origin = Controls.LineOrigin.Center;
                            Chart.ChartAxis[2].Origin = Controls.LineOrigin.Center;

                            MeasureType = ConfigBase.MeasureTypePitch;
                            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                            break;
                        case ModelBase.MeasureTypeRoll:
                            Chart.ChartAxis[0].Scale = 90;
                            Chart.ChartAxis[1].Scale = 90;
                            Chart.ChartAxis[2].Scale = 90;
                            Chart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                            Chart.ChartAxis[1].Origin = Controls.LineOrigin.Center;
                            Chart.ChartAxis[2].Origin = Controls.LineOrigin.Center;

                            MeasureType = ConfigBase.MeasureTypeRoll;
                            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                            break;
                        case ModelBase.MeasureTypeHeading:
                            Chart.ChartAxis[0].Scale = 360;
                            Chart.ChartAxis[1].Scale = 360;
                            Chart.ChartAxis[2].Scale = 360;

                            Chart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;
                            Chart.ChartAxis[1].Origin = Controls.LineOrigin.Bottom;
                            Chart.ChartAxis[2].Origin = Controls.LineOrigin.Bottom;

                            MeasureType = ConfigBase.MeasureTypeHeading;
                            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                            break;
                        case ModelBase.MeasureTypeAltitude:
                            Chart.ChartAxis[0].Scale = 20;
                            Chart.ChartAxis[1].Scale = 20;
                            Chart.ChartAxis[2].Scale = 20;

                            Chart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;
                            Chart.ChartAxis[1].Origin = Controls.LineOrigin.Bottom;
                            Chart.ChartAxis[2].Origin = Controls.LineOrigin.Bottom;

                            MeasureType = ConfigBase.MeasureTypeAltitude;
                            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginSensorFusionDiag, Payload = new byte[] { item.Value }, PayloadSize = 1, ExpectACK = true });
                            break;

                    }
                }
            }

        }
    }
}
