using NiVek.Common.Modules;
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
    public sealed partial class Settings : UserControl
    {
        public Settings()
        {
            this.InitializeComponent();
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
