using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

namespace NiVek.FlightControls.Controls
{
    public sealed partial class LeftStick : UserControl
    {
        DateTime _lastStickSend = DateTime.MinValue;

        public LeftStick()
        {
            this.InitializeComponent();

            MotorPower.ItemsSource = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "Off", Key = 0x00 },
                new KeyDisplay{Display = "5%", Key = (byte)(0xFF * 0.05f) },
                new KeyDisplay{Display = "10%", Key = (byte)(0xFF * 0.1f) },
                new KeyDisplay{Display = "15%", Key = (byte)(0xFF * 0.15f) },
                new KeyDisplay{Display = "20%", Key = (byte)(0xFF * 0.20f) },
                new KeyDisplay{Display = "25%", Key = (byte)(0xFF * 0.25f) },
                new KeyDisplay{Display = "30%", Key = (byte)(0xFF * 0.3f) },
                new KeyDisplay{Display = "40%", Key = (byte)(0xFF * 0.4f) },
                new KeyDisplay{Display = "50%", Key = (byte)(0xFF * 0.5f) }            
            };

            MotorPower.SelectedIndex = 0;
        }

        void SendStickData()
        {
            var msg = new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_ThrottleYawRollThrottle };
/*            msg.AddByte(App.Commo.Throttle);
            msg.AddSByte(App.Commo.Pitch);
            msg.AddSByte(App.Commo.Roll);
            msg.AddSByte(App.Commo.Yaw);*/
        //    App.Commo.Send(msg);

            _lastStickSend = DateTime.Now;
        }

        private void LeftStickArea_ManipulationCompleted_1(object sender, ManipulationCompletedRoutedEventArgs e)
        {
            if ((DateTime.Now - _lastStickSend).TotalMilliseconds > 250)
                SendStickData();
        }

        private void LeftStickArea_ManipulationDelta_1(object sender, ManipulationDeltaRoutedEventArgs e)
        {
            var newLeft = e.Position.X - (LeftStickThumb.Height / 2);
            var newTop = e.Position.Y - (LeftStickThumb.Height / 2);
            newTop = Math.Min(newTop, LeftStickArea.ActualHeight - LeftStickThumb.Height);
            newTop = Math.Max(newTop, 0);

            newLeft = Math.Max(newLeft, 0);
            newLeft = Math.Min(newLeft, LeftStickArea.ActualWidth - LeftStickThumb.Width);

            LeftStickThumb.SetValue(Canvas.LeftProperty, newLeft);
            LeftStickThumb.SetValue(Canvas.TopProperty, newTop);

            //App.Commo.Throttle = Convert.ToByte((((LeftStickArea.ActualHeight - (newTop + 30))) / (LeftStickArea.ActualHeight - 30)) * 255);
            //App.Commo.Yaw = Convert.ToSByte(50 - ((((LeftStickArea.ActualWidth - (newLeft + 30))) / (LeftStickArea.ActualWidth - 30)) * 100));

            if ((DateTime.Now - _lastStickSend).TotalMilliseconds > 250)
                SendStickData();
        }

        private void MotorPower_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                var item = e.AddedItems[0] as NiVek.Common.Models.KeyDisplay;

                /*var msg = new OutgoingMessage() { MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
                msg.AddByte(0xFF);
                msg.AddByte(item.Key);
                App.Commo.Send(msg);*/

              //  App.Commo.Throttle = item.Key;
            }
        }
    }
}