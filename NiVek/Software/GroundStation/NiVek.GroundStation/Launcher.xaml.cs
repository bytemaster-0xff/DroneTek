using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
using NiVek.FlightControls.Common;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.FlightControls
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Launcher : Page
    {
        public Launcher()
        {
            this.InitializeComponent();
            this.Loaded += Launcher_Loaded;


            DataContext = DroneHub.Instance;
        }

        async void Launcher_Loaded(object sender, RoutedEventArgs e)
        {
            await DroneHub.Instance.Populate();
            ConnectedDrones.ItemsSource = DroneHub.Instance.Drones;
        }


        #region Menu Handlers
        private void PitchRollPID_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.PIDCalibration));
        }

        private void AltitudeHoldPID_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.AltutidePID));
        }

        private void MotorsPID_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.MotorConfig));
        }

        private void HeadingdPID_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.HeadingPID));
        }

        private void Compass_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.CompassConfig));
        }

        private void BlackBoxPlaybox_Click_1(object sender, RoutedEventArgs e)
        {
            Window.Current.Content = new Views.BlackBoxPlayback();
        }

        private void SensorFusion_Click_1(object sender, RoutedEventArgs e)
        {
            if (DroneHub.Instance.Active.Status.Platform == SystemStatus.PlatformEnum.Micro)
                Frame.Navigate(typeof(Views.MicroSensorFusion));
            else
                Frame.Navigate(typeof(Views.SensorFusion));
        }

        private void SensorConfig_Click_1(object sender, RoutedEventArgs e)
        {
            if (DroneHub.Instance.Active.Status.Platform == SystemStatus.PlatformEnum.Micro)
                Frame.Navigate(typeof(Views.MicroConfig));
            else
                Frame.Navigate(typeof(Views.SensorConfiguration));
        }

        private void VibrationAnalysis_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.VibrationAnalysis));
        }

        private void RCControlCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.Calibration));
        }

        private void GoFlying_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(MainPage));
        }

        private void AllPID_Click_1(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.PIDSettings));
        }

        private void Console_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.Console));
        }

        private void Drone_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.DroneSettings), DroneHub.Instance.Active);
        }
        #endregion

        private void AddNewDrone_Tapped(object sender, TappedRoutedEventArgs e)
        {
            Frame.Navigate(typeof(Views.DroneSettings));
        }


        private void Connect_Click(object sender, RoutedEventArgs e)
        {
            if (DroneHub.Instance.Active.CurrentComms.ProtocolType == (Byte)DroneComms.ProtocolTypes.UDP)
            {
                var udpChannel = new UDPChannel();
                udpChannel.ConnectAsync(DroneHub.Instance.Active.CurrentComms.Address, DroneHub.Instance.Active.CurrentComms.Port);
                DroneHub.Instance.Active.Channel = udpChannel;
                DroneHub.Instance.Active.StartPingTimer();
            }
            else if (DroneHub.Instance.Active.CurrentComms.ProtocolType == (Byte)DroneComms.ProtocolTypes.TCP)
            {
                var tcpChannel = new TCPChannel();
                tcpChannel.ConnectAsync(DroneHub.Instance.Active.CurrentComms.Address, DroneHub.Instance.Active.CurrentComms.Port);
                DroneHub.Instance.Active.Channel = tcpChannel;
                DroneHub.Instance.Active.StartPingTimer();
            }
            else if (DroneHub.Instance.Active.CurrentComms.ProtocolType == (Byte)DroneComms.ProtocolTypes.Serial)
            {

            }
        }

        private void ConnectedDrones_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 1)
            {
                DroneHub.Instance.Active = e.AddedItems[0] as Drone;
                App.TheApp.FlightController.Drone = DroneHub.Instance.Active;
                App.TheApp.FlightController.StartListeningFlightStick();

                ConnectedDrones.SelectedIndex = -1;
            }
        }
    }
}
