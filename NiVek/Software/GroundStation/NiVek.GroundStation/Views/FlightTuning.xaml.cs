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

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class FlightTuning : NiVekPage
    {
        public FlightTuning()
        {
            this.InitializeComponent();
        }

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
            Frame.Navigate(typeof(Views.SensorFusion));
        }

        private void SensorConfig_Click_1(object sender, RoutedEventArgs e)
        {
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


        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {

        }
    }
}
