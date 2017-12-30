using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
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

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class PIDSettings : NiVekPage
    {
        public PIDSettings()
        {
            this.InitializeComponent();
        }

        protected async override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            Header.DataContext = Drone;

            if (status == NiVek.Common.Comms.Common.ConnectionStates.Connected)
                Settings.DataContext = await Drone.GetAsync<NiVek.Common.Models.GPIOConfig>(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.GPIOPidConstants);
        }

        private void Save_Click(object sender, RoutedEventArgs e)
        {
            var config = Settings.DataContext as NiVek.Common.Models.GPIOConfig;
            config.Serialize();
        }        
    }
}
