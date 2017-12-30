using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.ApplicationSettings;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    public sealed partial class HeadingPID : NiVekPage
    {
        public HeadingPID()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {            
            base.OnNavigatedTo(e);
            YawChart.Start();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            YawChart.Stop();
        }

        private async void TargetHeading_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 1)
            {
                var item = Convert.ToInt16((e.AddedItems[0] as ComboBoxItem).Content);

                var msg = new OutgoingMessage();
                msg.ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO;
                msg.MessageId = GPIOModule.CMD_SetHeading;
                msg.Add(item);
                msg.ExpectACK = true;
                await Drone.SendMessageAsync(msg);
            }
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            throw new NotImplementedException();
        }
    }
}
