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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkID=390556

namespace NiVek.Rover
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Launcher : Page
    {
        RoverStatus _currentRoverStatus;

        public Launcher()
        {
            this.InitializeComponent();
        }

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
            Ranging.DataContext = _currentRoverStatus;
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

        private async void Connect_Click(object sender, RoutedEventArgs e)
        {
            await DroneHub.Instance.Drone.Connect();
        }
    }
}
