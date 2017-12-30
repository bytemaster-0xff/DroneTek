using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NiVek.Common.Comms;
using NiVek.Common.Models;
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
using System.Diagnostics;
using NiVek.Common;
using NiVek.Common.Modules;


// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class Calibration : Views.NiVekPage
    {
        int _calibrationIndex = 0;
        List<CalibrationPoint> _calibrationPoints = CalibrationPoint.Get();

        public enum CompassCalibrationStates
        {
            Idle,
            Calibrating
        }


        public Calibration()
        {
            this.InitializeComponent();

            this.Unloaded += Calibration_Unloaded;
        }

        async void Calibration_Unloaded(object sender, RoutedEventArgs e)
        {
            if(CancelCalibration.Visibility == Windows.UI.Xaml.Visibility.Visible)
                await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_Cancel_RC_CalibrationMode);
        }        

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            BeginCalibration.IsEnabled = (status == NiVek.Common.Comms.Common.ConnectionStates.Connected);
            CaptureChannel.IsEnabled = (status == NiVek.Common.Comms.Common.ConnectionStates.Connected);
            FinalizeCalibration.IsEnabled = (status == NiVek.Common.Comms.Common.ConnectionStates.Connected);
        }

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.PWMDataIn:
                    RCSummary.DataContext = NiVek.Common.Models.RCCalibration.Create(msg.Payload);
                    break;
                case IncomingMessage.RCChannel:
                    var setting = msg.Payload[0];
                    var counts = msg.Payload[1] << 8 | msg.Payload[2];

                    Debug.WriteLine("MESSAGE :" + msg.Payload[0] + "," + msg.Payload[1] + "," + msg.Payload[2]);

                    var name = _calibrationPoints.Where(cd=>cd.Code == setting).First().Name;
                    CalibrationStep.Items.Add(String.Format("{0} = {1}", name, counts));
                    break;
                case IncomingMessage.PWMCalibratedSuccess:
                    RCSummary.DataContext = NiVek.Common.Models.RCCalibration.Create(msg.Payload);
                    break;
            }
        }

        private async void BeginCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_Start_RC_CalibrationMode);
            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CaptureChannel.Visibility = Windows.UI.Xaml.Visibility.Visible;
            CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;

            CalibrateStickInstructions.Text = _calibrationPoints[_calibrationIndex].Name;
            _calibrationIndex = 0;
            CalibrationStep.Items.Clear();
        }

        private async void CaptureChannel_Click_1(object sender, RoutedEventArgs e)
        {
            var msg = new OutgoingMessage() {ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_CalibrateRCChanncel, ExpectACK=true };
            msg.AddByte(_calibrationPoints[_calibrationIndex++].Code);
            await Drone.SendMessageAsync(msg);

            if (_calibrationIndex == _calibrationPoints.Count)
            {
                CalibrateStickInstructions.Text = "Press Finish";
                CaptureChannel.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                FinalizeCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
                CancelCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
            else
                CalibrateStickInstructions.Text = _calibrationPoints[_calibrationIndex].Name;
        }

        private async void FinalizeCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_End_RC_CalibrationMode);
            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            FinalizeCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
        }

        private async void CancelCalibration_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_Cancel_RC_CalibrationMode);

            BeginCalibration.Visibility = Windows.UI.Xaml.Visibility.Visible;
            FinalizeCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            CaptureChannel.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            FinalizeCalibration.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
        }        
    }
}
