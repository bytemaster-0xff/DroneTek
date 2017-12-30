using NiVek.WinCommon.Services;
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


namespace NiVek.Rover.Controls
{
    public sealed partial class Drive : UserControl
    {
        public Drive()
        {
            this.InitializeComponent();
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
    }
}
