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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.Rover.Controls
{
    public sealed partial class Turret : UserControl
    {
        public Turret()
        {
            this.InitializeComponent();
        }

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

        private async void ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            var swtch = sender as ToggleSwitch;
            var msgId = (swtch.IsOn) ? 120 : 121;

            var msg = new Common.OutgoingMessage()
            {
                ModuleType = Common.Comms.Common.ModuleTypes.Turret,
                MessageId = (byte)msgId,
                ExpectACK = true,
            };

            await DroneHub.Instance.Drone.SendMessageAsync(msg);
        }

    }
}
