using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using NiVek.WinCommon.Comms;
using NiVek.WinCommon.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.Rover
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Launcher : Page
    {
        RoverStatus _currentRoverStatus;

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            DroneHub.Instance.Drone = new Drone() { DroneAddress = 20 };
            DroneHub.Instance.Drone.Channel = new BluetoothChannel();
            DroneHub.Instance.Drone.CurrentComms = new Common.Models.DroneComms() { Address = "NiVekRover", Port = 0 };
            DroneHub.Instance.Drone.Channel.MessageReady += Comms_MessageReady;

            DataContext = DroneHub.Instance;
            _currentRoverStatus = new RoverStatus();
            RoverSensors.DataContext = _currentRoverStatus;
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }

        async void Comms_OnExceptionOccurred(object sender, Exception ex)
        {
            var md = new MessageDialog(ex.Message, "We've got a problem with bluetooth...");
            md.Commands.Add(new UICommand("Ah.. thanks.."));
            md.DefaultCommandIndex = 0;
            await md.ShowAsync();
        }

        async void Comms_MessageReady(object sender, IncomingMessage e)
        {
            if (e.SystemId == 70 && e.MessageId == 0x80)
            {
                Debug.WriteLine("GOT MESSAGE " + e);
                await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    _currentRoverStatus.Update(e.Payload);
                });
            }
        }




        public Launcher()
        {
            this.InitializeComponent();
        }
        private async void Connect_Click(object sender, RoutedEventArgs e)
        {
            await DroneHub.Instance.Drone.Connect();
        }

        private async void Stop_Click(object sender, RoutedEventArgs e)
        {
            var msg = new Common.OutgoingMessage()
            {
                ModuleType = Common.Comms.Common.ModuleTypes.Motor,
                MessageId = 110,
                ExpectACK = true,
            };

            await DroneHub.Instance.Drone.SendMessageAsync(msg);
        }


        private async void Pwr_Click(object sender, RoutedEventArgs e)
        {            
            var msg = new Common.OutgoingMessage()
            {
                ModuleType = Common.Comms.Common.ModuleTypes.Motor,
                MessageId = Convert.ToByte((sender as Button).Tag),
                ExpectACK = true,
            };            

            msg.AddByte(25);

            await DroneHub.Instance.Drone.SendMessageAsync(msg);
        }

        byte _pan = 90;
        byte _tilt = 90;

        private async void Turret_Click(object sender, RoutedEventArgs e)
        {
            var msgId = 110;
            switch ((sender as Button).Tag.ToString())
            {
                case "up": msgId = 111; break;
                case "down": msgId = 112; break;
                case "left": msgId = 113; break;
                case "right": msgId = 114; break;
                case "cntr": msgId = 110; break;
                case "startpan": msgId = 120; break;
                case "stoppan": msgId = 121; break;
            }

            var msg = new Common.OutgoingMessage()
            {
                ModuleType = Common.Comms.Common.ModuleTypes.Turret,
                MessageId = (byte)msgId,
                ExpectACK = true,
            };

            await DroneHub.Instance.Drone.SendMessageAsync(msg);
        }


        private void DisconnectButton_Click(object sender, RoutedEventArgs e)
        {
            DroneHub.Instance.Drone.Disconnect("Manual Disconnect");
        }

        bool _isCalibrating = false;

        private async void Calibrate_Click(object sender, RoutedEventArgs e)
        {

            var msg = new Common.OutgoingMessage()
            {
                ModuleType = Common.Comms.Common.ModuleTypes.Sensor,
                MessageId = (byte)(_isCalibrating ? SensorModule.CMD_EndMagCalibration : SensorModule.CMD_BeginMagCalibrationXY),
                ExpectACK = true,
            };

            _isCalibrating = !_isCalibrating;

            await DroneHub.Instance.Drone.SendMessageAsync(msg);
        }

    }
}
