using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using NiVek.Common.ViewModels;
using NiVek.FlightControls.Commo;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.System.Threading;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    public abstract class NiVekPage : Windows.UI.Xaml.Controls.Page
    {
        public Controls.NiVekHeader HeaderItem { get; set; }

        protected const double SenstivityIncrementSize = 0.2;
        protected const double IncrementSize = 0.02;
        protected const double IntegrationIncrementSize = 0.5;
        protected const double DerivativeIncrementSize = 0.05;
        protected const double IMAX_INCREMENT_SIZE = 0.2f;
        protected const double DECAY_INCREMENT_SIZE = 0.02f;

        public Drone Drone { get { return DroneHub.Instance.Active; } }

        public NiVekPage()
        {
            this.Loaded += NiVekPage_Loaded;

            //DataContext = new FlightConsoleViewModel();
        }

        byte ConvertByteToPercent(int percent)
        {
            return (byte)((percent * 255) / 100);
        }

        protected virtual void GamePadChanged(NiVek.FlightControls.GamePad.ButtonStateChangedEventArgs gamePadState)
        {

        }

        void NiVekPage_Loaded(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            if (Drone != null)
            {
                switch (Drone.Channel.ConnectionState)
                {
                    case NiVek.Common.Comms.Common.ConnectionStates.Connecting: SetConnectionStatus(Drone.Channel.ConnectionState, Colors.Yellow); break;
                    case NiVek.Common.Comms.Common.ConnectionStates.Disconnected: SetConnectionStatus(Drone.Channel.ConnectionState, Colors.Red); break;
                    case NiVek.Common.Comms.Common.ConnectionStates.Connected: SetConnectionStatus(Drone.Channel.ConnectionState, Colors.Green); break;
                }
            }
        }

        public bool CanGoBack()
        {
            return Frame.CanGoBack;
        }

        public void GoBack()
        {
            if (this.Frame != null && this.Frame.CanGoBack) 
                base.Frame.GoBack();
        }


        public void GoBack(object sender, RoutedEventArgs e)
        {
            // Use the navigation frame to return to the previous page
            GoBack();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            if (Drone != null)
            {
                Drone.Channel.OnConnect += _commo_Connected;
                Drone.Channel.OnDisconnect += _commo_Disconnected;
                Drone.Channel.MessageReady += Commo_MessageReceived;
            }
        }

        void Commo_ConnectionEstablished(object sender, EventArgs e)
        {

        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            if (Drone != null)
            {
                Drone.Channel.OnConnect -= _commo_Connected;
                Drone.Channel.OnDisconnect -= _commo_Disconnected;
                Drone.Channel.MessageReady -= Commo_MessageReceived;
            }
        }

        async void _commo_Connecting(object sender, EventArgs e)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates.Connecting, Colors.Yellow);
            });
        }

        async void _commo_Connected(object sender, EventArgs e)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates.Connected, Colors.Green);
            });
        }

        async void _commo_Disconnected(object sender, String e)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates.Disconnected, Colors.Red);
            });
        }

        async void _commo_FailedToConnect(object sender, EventArgs e)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates.Disconnected, Colors.Red);
            });
        }

        async void Commo_MessageReceived(object sender, IncomingMessage msg)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var vm = DataContext;

                MessageArrived(msg);

                if (((NiVek.Common.Comms.Common.ModuleTypes)msg.SystemId == NiVek.Common.Comms.Common.ModuleTypes.System && msg.MessageId == IncomingMessage.Ack) ||
                    ((NiVek.Common.Comms.Common.ModuleTypes)msg.SystemId == NiVek.Common.Comms.Common.ModuleTypes.System && msg.MessageId == IncomingMessage.Nak))
                {
                    UInt16 serialNumber = (UInt16)(msg.Payload[0] << 8 | msg.Payload[1]);

                    Debug.WriteLine("Message Serial Number -> {0}", serialNumber);
                    MessageEntry entry = Drone.Channel.MessageLog.Where(item => item.SerialNumber == serialNumber).FirstOrDefault();
                    if (entry != null)
                    {
                        entry.AckStatus = msg.MessageId == IncomingMessage.Ack ? MessageEntry.AckStatusTypes.Ack : MessageEntry.AckStatusTypes.NotAck;
                        Debug.WriteLine(entry.AckStatus == MessageEntry.AckStatusTypes.Ack ? "ACK!" : "NAK :*(");
                    }
                }
            });
        }

        protected virtual void MessageArrived(IncomingMessage msg) { }

        protected abstract void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status);

        protected void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status, Windows.UI.Color color)
        {
            SetConnectionStatus(status);
        }

        public void BBFilePicker_FileSelected_1(Object sender, StorageFile file)
        {

        }
    }
}
