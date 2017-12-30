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
    public sealed partial class SensorConfiguration : NiVekPage
    {
        List<Controls.Chart> _charts;

        public SensorConfiguration()
        {
            this.InitializeComponent();

            _charts = new List<Controls.Chart>();
            _charts.Add(XChart);
            _charts.Add(YChart);
            _charts.Add(ZChart);
            _charts.Add(RawXChart);
            _charts.Add(RawYChart);
            _charts.Add(RawZChart);

            this.Unloaded += SensorConfiguration_Unloaded;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            Header.DataContext = Drone;
        }

        async void SensorConfiguration_Unloaded(object sender, RoutedEventArgs e)
        {
            if (Drone.Channel.ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected)
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_EndSensorDiagnostics);
        }

        private async void SensorSelection_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (LSM303ACCSensor != null)
            {
                LSM303ACCSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                ADXL345Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                LSM303MagSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                HMC5883Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                ITG3200Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                L3GD20Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                MPU60x0AccSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                MPU60x0GyroSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                LIPOAdcSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                BMP085Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                HCSR04Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                MS5611Sensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                GPSSensor.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }

            var selectedItem = ((ComboBox)sender).SelectedItem as ComboBoxItem;
            if (selectedItem != null && ((ComboBox)sender).SelectedIndex > 0)
            {
                foreach (var chart in _charts)
                    chart.Visibility = Windows.UI.Xaml.Visibility.Visible;

                byte sensorId = 0;

                XChart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                YChart.ChartAxis[0].Origin = Controls.LineOrigin.Center;
                ZChart.ChartAxis[0].Origin = Controls.LineOrigin.Center;

                switch (selectedItem.Content.ToString())
                {
                    case "ADXL345":
                        YChart.ChartAxis[0].Scale = 45.0;
                        ZChart.ChartAxis[0].Scale = 45.0;
                        XChart.ChartAxis[0].Scale = 45.0;
                        ADXL345Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.ADXL345;
                        break;
                    case "LSM303ACC":
                        YChart.ChartAxis[0].Scale = 2.0;
                        ZChart.ChartAxis[0].Scale = 2.0;
                        XChart.ChartAxis[0].Scale = 2.0;
                        LSM303ACCSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.LSM303_ACC;
                        break;
                    case "LSM303MAG":
                        YChart.ChartAxis[0].Scale = 1.0;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        XChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        LSM303MagSensor.Visibility = Windows.UI.Xaml.Visibility.Visible; ;
                        sensorId = NiVek.Common.Models.Sensors.LSM303_MAG;

                        break;
                    case "HMS5883":
                        YChart.ChartAxis[0].Scale = 90.0;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        XChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        HMC5883Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.HMC5883;
                        break;
                    case "L3GD20":
                        YChart.ChartAxis[0].Scale = 5.0;
                        ZChart.ChartAxis[0].Scale = 5.0;
                        XChart.ChartAxis[0].Scale = 5.0;
                        L3GD20Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.L3GD20;
                        break;
                    case "HCSR04":
                        YChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawYChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawXChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        XChart.ChartAxis[0].Scale = 1;
                        XChart.ChartAxis[0].FormatString = "0";
                        XChart.ChartAxis[0].Label = "Meters";
                        sensorId = NiVek.Common.Models.Sensors.HCSR04;
                        HCSR04Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;
                    case "GPS":

                        XChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        YChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawYChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawXChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                        GPSSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;


                    case "BMP085":
                        XChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        YChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Visible;

                        XChart.ChartAxis[0].Scale = 30000;
                        XChart.ChartAxis[0].Label = "Alt (M)";
                        XChart.ChartAxis[0].FormatString = "0.0";
                        XChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        YChart.ChartAxis[0].Scale = 50;
                        YChart.ChartAxis[0].Label = "hPa";
                        YChart.ChartAxis[0].FormatString = "0.000";
                        YChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        ZChart.ChartAxis[0].Scale = 30;
                        ZChart.ChartAxis[0].FormatString = "0.0";
                        ZChart.ChartAxis[0].Label = "DegC";
                        ZChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        sensorId = NiVek.Common.Models.Sensors.BMP085;

                        RawZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawYChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawXChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                        BMP085Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;
                    case "MS5611":
                        XChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        YChart.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        ZChart.Visibility = Windows.UI.Xaml.Visibility.Visible;

                        XChart.ChartAxis[0].Scale = 30000;
                        XChart.ChartAxis[0].Label = "Alt (M)";
                        XChart.ChartAxis[0].FormatString = "0.0";
                        XChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        YChart.ChartAxis[0].Scale = 50;
                        YChart.ChartAxis[0].Label = "hPa";
                        YChart.ChartAxis[0].FormatString = "0.000";
                        YChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        ZChart.ChartAxis[0].Scale = 30;
                        ZChart.ChartAxis[0].FormatString = "0.0";
                        ZChart.ChartAxis[0].Label = "DegC";
                        ZChart.ChartAxis[0].Origin = Controls.LineOrigin.Bottom;

                        sensorId = NiVek.Common.Models.Sensors.MS5611;

                        RawZChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawYChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                        RawXChart.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                        MS5611Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        break;


                    case "LIPOADC":
                        YChart.ChartAxis[0].Scale = 20.0;
                        ZChart.ChartAxis[0].Scale = 20.0;
                        XChart.ChartAxis[0].Scale = 20.0;
                        LIPOAdcSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.LIPO_ADC;
                        break;
                    case "ITG3200":
                        YChart.ChartAxis[0].Scale = 5.0;
                        ZChart.ChartAxis[0].Scale = 5.0;
                        XChart.ChartAxis[0].Scale = 5.0;
                        ITG3200Sensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.ITG3200;
                        break;
                    case "MPU60x0GYRO":
                        RawYChart.ChartAxis[0].Scale = .01;
                        RawZChart.ChartAxis[0].Scale = .01;
                        RawXChart.ChartAxis[0].Scale = .01;
                        YChart.ChartAxis[0].Scale = 1.0;
                        ZChart.ChartAxis[0].Scale = 1.0;
                        XChart.ChartAxis[0].Scale = 1.0;
                        MPU60x0GyroSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.MPU60x0GYRO;
                        break;
                    case "MPU60x0ACC":
                        YChart.ChartAxis[0].Scale = 5.0;
                        ZChart.ChartAxis[0].Scale = 5.0;
                        XChart.ChartAxis[0].Scale = 5.0;
                        MPU60x0AccSensor.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        sensorId = NiVek.Common.Models.Sensors.MPU60x0ACC;
                        break;
                }

                var visibleCharts = _charts.Where(chrt => chrt.Visibility == Windows.UI.Xaml.Visibility.Visible);

                foreach (var row in ChartGrid.RowDefinitions)
                    ChartGrid.RowDefinitions[0].Height = new GridLength(0);

                var chartHeight = ChartGrid.ActualHeight / visibleCharts.Count();
                foreach (var chart in visibleCharts)
                {
                    ChartGrid.RowDefinitions[Convert.ToInt32(chart.Tag)].Height = new GridLength(chartHeight);
                    chart.Height = chartHeight;
                    chart.Background = new SolidColorBrush(Colors.Red);
                }

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
        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if (status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                Debug.WriteLine("SET CONN STATE");
                var sensorConfig = await Drone.GetAsync<NiVek.Common.Models.SensorConfig>(NiVek.Common.Comms.Common.ModuleTypes.Sensor, SensorModule.CMD_ReadCfgValues, IncomingMessage.SensorConfigSettings);
                DataContext = sensorConfig;
                sensorConfig.IsBound = true;
            }
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.SensorBattery:
                    switch (SensorType)
                    {
                        case "LIPOADC":
                            var battData = NiVek.Common.Models.BatteryCondition.Create(msg.Payload);
                            XChart.AddDataPoint(battData.Cell1);
                            YChart.AddDataPoint(battData.Cell2);
                            ZChart.AddDataPoint(battData.Cell3);
                            break;
                    }
                    break;

                case IncomingMessage.SensorConfigUpdated:
                    var snsrConfig = (NiVek.Common.Models.SensorConfig)ConfigSettings.DataContext;
                    snsrConfig.UpdateApplied(msg.Payload[0] << 8 | msg.Payload[1]);
                    break;
                case IncomingMessage.SensorData:

                    var update = NiVek.Common.Models.SensorUpdate.Create(msg.Payload);
                    break;
                case IncomingMessage.SensorDiagnostics:


                    switch (SensorType)
                    {
                        case "MPU60x0ACC":
                        case "MPU60x0GYRO":
                        case "LSM303ACC":
                        case "L3GD20":
                        case "ITG6500":
                        case "ITG3200":
                        case "ADXL345":
                            {
                                var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload);
                                RawXChart.AddDataPoint(snsrData.RawX);
                                RawYChart.AddDataPoint(snsrData.RawY);
                                RawZChart.AddDataPoint(snsrData.RawZ);
                                XChart.AddDataPoint(snsrData.X);
                                YChart.AddDataPoint(snsrData.Y);
                                ZChart.AddDataPoint(snsrData.Z);
                            }
                            break;

                        case "HMS5883":
                        case "LSM303MAG":
                            {
                                var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload);
                                RawXChart.AddDataPoint(snsrData.RawX);
                                XChart.AddDataPoint(snsrData.X * 100.0);
                                RawYChart.AddDataPoint(snsrData.RawY);
                                RawZChart.AddDataPoint(snsrData.RawZ);
                            }

                            break;
                        case "HCSR04":
                            {
                                var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload, false);
                                XChart.AddDataPoint(snsrData.X * 10.0f);
                            }
                            break;
                        case "MS5611":
                        case "BMP085":
                            {
                                var snsrData = NiVek.Common.Models.SensorDetail.Create(msg.Payload, false);
                                //Data Returned in CM w/ no decimals, 
                                //want displayed in meters w/ 2 decimals so we are OK
                                XChart.AddDataPoint(snsrData.X);
                                YChart.AddDataPoint((snsrData.Y / 10.0f) + 1000.0);
                                ZChart.AddDataPoint(snsrData.Z);
                            }
                            break;
                    }

                    break;
            }
        }

        private async void Restart_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendMessageAsync(new OutgoingMessage() {ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_SensorRestart, ExpectACK = true });
        }

        private async void Reset_Click_1(object sender, RoutedEventArgs e)
        {
            var msg = new OutgoingMessage() { MessageId = SensorModule.CMD_SensorResetDefaults, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, ExpectACK = true };
            await Drone.SendMessageAsync(msg);
        }

        private async void CancelCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_CancelMagCalibration });
            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
        }

        private async void EndCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_EndMagCalibration });
        }

        private async void BeginCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_BeginMagCalibrationXY });

            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            EndCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
        }

        private async void Zero_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_SensorZero });
        }

        public void ChartHidden(object sender, EventArgs args)
        {

        }
    }
}
