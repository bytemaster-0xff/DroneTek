using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class MotorConfig : Views.NiVekPage
    {
        private const double MOTOR_TRIM_INTERVAL = 0.01;


        public MotorConfig()
        {
            this.InitializeComponent();
        }

        private void initMotorConfig()
        {
            DataContext = Drone;
            Drone.Config.IsBound = true;

            PwrFront.SelectedIndex = 0;
            PwrPort.SelectedIndex = 0;
            PwrPortFront.SelectedIndex = 0;
            PwrPortRear.SelectedIndex = 0;
            PwrRear.SelectedIndex = 0;
            PwrStarboard.SelectedIndex = 0;
            PwrStarboardFront.SelectedIndex = 0;
            PwrStarboardRear.SelectedIndex = 0;
            PwrAll1.SelectedIndex = 0;
            PwrAll2.SelectedIndex = 0;
        }


        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            initMotorConfig();

            await Drone.SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_StartMotorConfig, ExpectACK = true });
        }


        protected async override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            DataContext = null;
            Drone.Config.IsBound = true;

            Drone.ThrottleInput = 0;

            await Drone.SendMessageAsync(new OutgoingMessage(){ ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_EndMotorConfig, ExpectACK = true });
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            if(status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {                
                var config = await Drone.GetAsync<GPIOConfig>(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.GPIOPidConstants);

            }
        }

        private async void MotorPower_Changed(object sender, Windows.UI.Xaml.Controls.SelectionChangedEventArgs e)
        {
                KeyDisplay selectedItem = (sender as ComboBox).SelectedItem as KeyDisplay;
                if (selectedItem != null)
                {
                    var msg = new OutgoingMessage() {MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
                    msg.AddByte(Convert.ToByte((sender as ComboBox).Tag));
                    msg.AddByte(Convert.ToByte(selectedItem.Key));
                    await Drone.SendMessageAsync(msg);
                }
        }

        private void LessStarboardFrontMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscStarboardFront -= MOTOR_TRIM_INTERVAL;
        }

        private void MoreStarboardFrontMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscStarboardFront += MOTOR_TRIM_INTERVAL;
        }

        private void LessStarboardRearMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscStarboardRear -= MOTOR_TRIM_INTERVAL;
        }

        private void MoreStarboardRearMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscStarboardRear += MOTOR_TRIM_INTERVAL;
        }

        private void LessPortFrontMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscPortFront -= MOTOR_TRIM_INTERVAL;
        }

        private void MorePortFrontMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscPortFront += MOTOR_TRIM_INTERVAL;
        }

        private void LessPortRearMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscPortRear -= MOTOR_TRIM_INTERVAL;
        }

        private void MorePortRearMotor_Click(object sender, RoutedEventArgs e)
        {
            Drone.Config.EscPortRear += MOTOR_TRIM_INTERVAL;
        }
    }
}
