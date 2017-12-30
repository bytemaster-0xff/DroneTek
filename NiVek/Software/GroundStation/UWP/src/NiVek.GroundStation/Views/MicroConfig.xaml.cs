using NiVek.Common.Comms;
using NiVek.Common.Models;
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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MicroConfig : NiVekPage
    {
        MicroConfiguration _configuration;

        public MicroConfig()
        {
            this.InitializeComponent();
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            _configuration = await Drone.GetAsync<MicroConfiguration>(NiVek.Common.Comms.Common.ModuleTypes.GPIO, NiVek.Common.Modules.GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.MicroConfig);

            PitchPID.DataContext = _configuration.PIDPitch;
            RollPID.DataContext = _configuration.PIDRoll;
            YawPID.DataContext = _configuration.PIDYaw;
            AltitudePID.DataContext = _configuration.PIDAlt;
            StablePID.DataContext = _configuration.PIDStable;

        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            
        }
    }
}
