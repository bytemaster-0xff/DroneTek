using NiVek.Common;
using NiVek.Common.Modules;
using NiVek.FlightControls.Commo;
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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public sealed partial class RightStick : UserControl
    {
        public RightStick()
        {
            this.InitializeComponent();
        }

        DateTime _lastStickSend = DateTime.MinValue;

        void SendStickData()
        {
            var msg = new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_ThrottleYawRollThrottle };
            /*msg.AddByte(App.Commo.Throttle);
            msg.AddSByte(App.Commo.Pitch);
            msg.AddSByte(App.Commo.Roll);
            msg.AddSByte(App.Commo.Yaw);*/
            //App.Commo.Send(msg);
            _lastStickSend = DateTime.Now;
        }


        private void RightStickArea_ManipulationDelta_1(object sender, ManipulationDeltaRoutedEventArgs e)
        {
            var newPosition = e.Position;

            var newLeft = e.Position.X - (RightStickThumb.Height / 2);
            var newTop = e.Position.Y - (RightStickThumb.Height / 2);
            newTop = Math.Min(newTop, RightStickArea.ActualHeight - RightStickThumb.Height);
            newTop = Math.Max(newTop, 0);

            newLeft = Math.Max(newLeft, 0);
            newLeft = Math.Min(newLeft, RightStickArea.ActualWidth - RightStickThumb.Width);

            RightStickThumb.SetValue(Canvas.LeftProperty, newLeft);
            RightStickThumb.SetValue(Canvas.TopProperty, newTop);

            //App.Commo.Pitch = Convert.ToSByte(50 - (((RightStickArea.ActualHeight - (newTop + 30))) / (RightStickArea.ActualHeight - 30)) * 100);
            //App.Commo.Roll = Convert.ToSByte(50 - ((((RightStickArea.ActualWidth - (newLeft + 30))) / (RightStickArea.ActualWidth - 30)) * 100));

            if ((DateTime.Now - _lastStickSend).TotalMilliseconds > 250)
                SendStickData();
        }

        private void RightStickArea_ManipulationCompleted_1(object sender, ManipulationCompletedRoutedEventArgs e)
        {
            if ((DateTime.Now - _lastStickSend).TotalMilliseconds > 250)
                SendStickData();
        }
    }
}
