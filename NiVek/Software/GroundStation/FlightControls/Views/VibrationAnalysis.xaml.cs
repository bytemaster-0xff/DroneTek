using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
using NiVek.FlightControls.NiVekMath;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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
    public sealed partial class VibrationAnalysis : NiVekPage
    {
        List<Controls.Chart> _charts;

        private bool _paused = false;

        public VibrationAnalysis()
        {
            this.InitializeComponent();

            _charts = new List<Controls.Chart>();
            _charts.Add(XChart);
            _charts.Add(YChart);
            _charts.Add(ZChart);
            _charts.Add(RawXChart);
            _charts.Add(RawYChart);
            _charts.Add(RawZChart);

            this.Unloaded += VibrationAnalysis_Unloaded;
        }

        async void VibrationAnalysis_Unloaded(object sender, RoutedEventArgs e)
        {
            if (Drone.Channel.ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected)
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_EndSensorDiagnostics);
            
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
            if (_paused)
                return;

            switch (msg.MessageId)
            {
                case IncomingMessage.SensorDiagnostics:
                    var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload);
                    if (snsrData == null)
                        return;

                    switch (SensorType)
                    {
                        case "MPU60x0ACC":
                        case "LSM303ACC":
                        case "ADXL345":
                        case "L3GD20":
                            if(RawXChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                RawXChart.AddDataPoint(snsrData.RawX);

                            if(RawYChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                RawYChart.AddDataPoint(snsrData.RawY);

                            if(RawZChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                RawZChart.AddDataPoint(snsrData.RawZ);

                            if (XChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                XChart.AddDataPoint(snsrData.X);

                            if (YChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                YChart.AddDataPoint(snsrData.Y);

                            if (ZChart.Visibility == Windows.UI.Xaml.Visibility.Visible)
                                ZChart.AddDataPoint(snsrData.Z);
                            break;
                    }

                    break;

            }
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            RawXChart.Start();
            RawYChart.Start();
            RawZChart.Start();
            XChart.Start();
            YChart.Start();
            ZChart.Start();

            Header.DataContext = Drone;

            base.OnNavigatedTo(e);
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            RawXChart.Stop();
            RawYChart.Stop();
            RawZChart.Stop();
            XChart.Stop();
            YChart.Stop();
            ZChart.Stop();
        }

        public string SensorType
        {
            get
            {
                var selectedItem = ((ComboBox)SensorSelection).SelectedItem as ComboBoxItem;
                if (selectedItem != null)
                    return selectedItem.Content.ToString();
                else
                    return String.Empty;

            }
        }

        private async void SensorSelection_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (LSM303ACCSensor != null)
            {
                LSM303ACCSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                ADXL345Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                MPU60x0AccSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                L3GD20Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }

            var selectedItem = ((ComboBox)sender).SelectedItem as ComboBoxItem;
            if (selectedItem != null && ((ComboBox)sender).SelectedIndex > 0)
            {
                foreach (var chart in _charts)
                    chart.Visibility = Windows.UI.Xaml.Visibility.Visible;

                byte sensorId = 0;

                switch (selectedItem.Content.ToString())
                {
                    case "L3GD20":
                        RawYChart.ChartAxis[0].Scale = 200;
                        RawZChart.ChartAxis[0].Scale = 200;
                        RawXChart.ChartAxis[0].Scale = 200;

                        YChart.ChartAxis[0].Scale = 45.0;
                        ZChart.ChartAxis[0].Scale = 45.0;
                        XChart.ChartAxis[0].Scale = 45.0;
                        L3GD20Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.L3GD20;
                        break;

                    case "ADXL345":
                        YChart.ChartAxis[0].Scale = 45.0;
                        ZChart.ChartAxis[0].Scale = 45.0;
                        XChart.ChartAxis[0].Scale = 45.0;
                        ADXL345Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.ADXL345;
                        break;
                    case "LSM303ACC":
                        RawYChart.ChartAxis[0].Scale = 34000;
                        RawZChart.ChartAxis[0].Scale = 34000;
                        RawXChart.ChartAxis[0].Scale = 34000;

                        YChart.ChartAxis[0].Scale = 200.0;
                        ZChart.ChartAxis[0].Scale = 200.0;
                        XChart.ChartAxis[0].Scale = 200.0;
                        LSM303ACCSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.LSM303_ACC;
                        break;
                    case "MPU60x0ACC":
                        YChart.ChartAxis[0].Scale = 5.0;
                        ZChart.ChartAxis[0].Scale = 5.0;
                        XChart.ChartAxis[0].Scale = 5.0;
                        MPU60x0AccSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.MPU60x0ACC;
                        break;
                }

                /*var visibleCharts = _charts.Where(chrt => chrt.Visibility == Windows.UI.Xaml.Visibility.Visible);

                var chartHeight = ChartGrid.ActualHeight / visibleCharts.Count();
                foreach (var chart in visibleCharts)
                    chart.Height = chartHeight;*/

                if (sensorId > 0)
                {
                    var msg = new OutgoingMessage()
                    {
                        DestinationAddress = Drone.DroneAddress,
                        ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor,
                        MessageId = SensorModule.CMD_BeginSensorDiagnostics,
                        ExpectACK = true
                    };

                    msg.AddByte(sensorId);
                    await Drone.SendMessageAsync(msg);
                }


                XChart.Refresh();
                YChart.Refresh();
                ZChart.Refresh();
                RawYChart.Refresh();
                RawZChart.Refresh();
                RawXChart.Refresh();
            }
        }

        private void ResetStdDev_Click_1(object sender, RoutedEventArgs e)
        {
            XChart.Refresh();
            YChart.Refresh();
            ZChart.Refresh();
            RawXChart.Refresh();
            RawYChart.Refresh();
            RawZChart.Refresh();
        }

        private void AxisSelection_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (XChart != null)
            {
                XChart.Refresh();
                YChart.Refresh();
                ZChart.Refresh();
                RawXChart.Refresh();
                RawYChart.Refresh();
                RawZChart.Refresh();
                
                XChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                YChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                ZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                RawXChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                RawYChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                RawZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                switch (AxisSelection.SelectedIndex)
                {
                    case 0:
                        RawXChart.DrawChartBackground();
                        XChart.DrawChartBackground();
                        RawXChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        XChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;
                    case 1:
                        RawYChart.DrawChartBackground();
                        YChart.DrawChartBackground();
                        RawYChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        YChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;
                    case 2:
                        RawZChart.DrawChartBackground();
                        ZChart.DrawChartBackground();
                        RawZChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
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

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            _paused = !_paused;
            PauseButton.Content = _paused ? "Continue" : "Pause";

        }
    }
}
