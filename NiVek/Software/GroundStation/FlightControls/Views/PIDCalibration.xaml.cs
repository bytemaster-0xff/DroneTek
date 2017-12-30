using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Shapes;
using System.Collections.ObjectModel;
using Windows.Storage;
using Windows.Media.Capture;
using NiVek.Common;
using NiVek.Common.Modules;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using Windows.UI.Xaml.Media.Imaging;
using Windows.ApplicationModel.Core;
using Windows.Foundation;
using System.Threading.Tasks;
using System.Diagnostics;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class PIDCalibration : Views.NiVekPage
    {
        NiVek.Common.Models.GPIOConfig _pid;
        private bool _paused;

        public PIDCalibration()
        {
            this.InitializeComponent();
            this.Loaded += PIDCalibration_Loaded;
            this.Unloaded += PIDCalibration_Unloaded;

            //_wrapper = new D2DWrapper();
            //_wrapper.Initialize(CoreApplication.MainView.CoreWindow);

            MotorPower.ItemsSource = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "Off", Key = 0x00 },
                new KeyDisplay{Display = "10%", Key = (byte)(0xFF * 0.1f) },
                new KeyDisplay{Display = "20%", Key = (byte)(0xFF * 0.20f) },
                new KeyDisplay{Display = "30%", Key = (byte)(0xFF * 0.3f) },
                new KeyDisplay{Display = "35%", Key = (byte)(0xFF * 0.35f) },
                new KeyDisplay{Display = "40%", Key = (byte)(0xFF * 0.4f) },
                new KeyDisplay{Display = "45%", Key = (byte)(0xFF * 0.45f) },
                new KeyDisplay{Display = "50%", Key = (byte)(0xFF * 0.5f) },            
                new KeyDisplay{Display = "53%", Key = (byte)(0xFF * 0.53f) },            
                new KeyDisplay{Display = "56%", Key = (byte)(0xFF * 0.56f) },            
                new KeyDisplay{Display = "60%", Key = (byte)(0xFF * 0.60f) },            
                new KeyDisplay{Display = "63%", Key = (byte)(0xFF * 0.60f) },            
                new KeyDisplay{Display = "66%", Key = (byte)(0xFF * 0.60f) }            
            };

            MotorPower.SelectedIndex = 0;
            _paused = false;

        }

        void PIDCalibration_Loaded(object sender, RoutedEventArgs e)
        {
            Axis.SelectedIndex = 0;
            MediaCapture captureMgr = new MediaCapture();
            
        }

        protected async override void OnNavigatedTo(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {            
            base.OnNavigatedTo(e);
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_StartPIDConfig);
            DataContext = Drone;

            MotorChart.Start();
            Chart.Start();
        }

        protected override void OnNavigatedFrom(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            MotorChart.Stop();
            Chart.Stop();
        }

        async void PIDCalibration_Unloaded(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_EndPIDConfig);
            
        }

        private Targets _targets;

        protected override void MessageArrived(IncomingMessage msg)
        {
            switch (msg.MessageId)
            {
                case IncomingMessage.SystemStatus:
                    var systemStatus = NiVek.Common.Models.SystemStatus.Create(msg.Payload);
                    if (systemStatus != null)
                    {
                        if (systemStatus.IsArmed == SystemStatus.IsArmedStates.Safe || systemStatus.IsArmed == SystemStatus.IsArmedStates.unknown)
                            MotorPower.SelectedIndex = 0;

                        if (systemStatus.SensorsOnline)
                            Settings.Visibility = Windows.UI.Xaml.Visibility.Visible;
                        else
                            Settings.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                    }
                    break;
                case IncomingMessage.Targets:
                      _targets = NiVek.Common.Models.Targets.Create(msg.Payload); 
                    break;
                case IncomingMessage.PIDTuningDetail:
                    {
                        var pidTuningDetail = NiVek.Common.Models.PIDTuningDetails.Create(msg.Payload);
                        GraphData(pidTuningDetail);
                    }
                    break;

            }
        }

        private async void ResetPID_Click_1(object sender, RoutedEventArgs e)
        {
            await Drone.SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_ResetPIDValues);
        }

        private async void RefreshPID_Click_1(object sender, RoutedEventArgs e)
        {
            _pid = await Drone.GetAsync<GPIOConfig>(NiVek.Common.Comms.Common.ModuleTypes.GPIO, GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.GPIOPidConstants);
        }

        public void GraphData(NiVek.Common.Models.PIDTuningDetails update)
        {
            if (update == null)
                return;

            if (_paused)
                return;

            if ((update.Axis == PIDTuningDetails.AxisTypes.Pitch && Axis.SelectedIndex == PITCH_IDX) ||
                (update.Axis == PIDTuningDetails.AxisTypes.Roll && Axis.SelectedIndex == ROLL_IDX))
            {
                Chart.AddDataPoint(new double[] {
                update.Angle,
                update.TargetRate,
                update.ErrorSigma,
                update.Rate,
                update.P_SteadyComponent,
                update.I_SteadyComponent,
                update.P_RateComponent,
                update.I_RateComponent,
                update.D_RateComponent
                });
            }
            if ((update.Axis == PIDTuningDetails.AxisTypes.Pitch && Axis.SelectedIndex == PITCH_IDX) ||
                (update.Axis == PIDTuningDetails.AxisTypes.Roll && Axis.SelectedIndex == ROLL_IDX))
            {
                MotorChart.AddDataPoint(new double[] {
                update.Power1,
                update.Power2,
                });

            }
        }

        private async void ComboBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                var item = e.AddedItems[0] as KeyDisplay;

                var msg = new OutgoingMessage() {MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
                msg.AddByte(0xFF);
                msg.AddByte(item.Key);
                Drone.ThrottleInput = item.Key;
                await Drone.SendMessageAsync(msg);
            }
        }

        //YUCK!
        const int NONE_IDX = 0;
        const int PITCH_IDX = 1;
        const int ROLL_IDX = 2;
        const int YAW_IDX = 3;
        const int ALTITUDE_IDX = 4;
        const int MOTOR_IDX = 5;

        private void Measure_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            PitchSettings.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            RollSettings.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            switch (Axis.SelectedIndex)
            {
                case 0: break;
                case 1: PitchSettings.Visibility = Windows.UI.Xaml.Visibility.Visible; break;
                case 2: RollSettings.Visibility = Windows.UI.Xaml.Visibility.Visible; break;
            }
        }


        private void Pause_Click_1(object sender, RoutedEventArgs e)
        {
            _paused = !_paused;

            if (_paused)
                Pause.Content = "Continue";
        }

        private void RollAngle_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (RollAngle != null)
            {
                switch (RollAngle.SelectedIndex)
                {
                    case 0: Drone.RollInput = -33; break;
                    case 1: Drone.RollInput = -20; break;
                    case 2: Drone.RollInput = -10; break;
                    case 3: Drone.RollInput = -5; break;
                    case 4: Drone.RollInput = 0; break;
                    case 5: Drone.RollInput = 5; break;
                    case 6: Drone.RollInput = 10; break;
                    case 7: Drone.RollInput = 20; break;
                    case 8: Drone.RollInput = 33; break;
                }
            }
        }

        private void RollAngleToggle_Checked(object sender, RoutedEventArgs e)
        {


            Chart.ChartAxis[0].Visibility = Chart.ChartAxis[0].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollTargetToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[1].Visibility = Chart.ChartAxis[1].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollSigmaToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[2].Visibility = Chart.ChartAxis[2].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollDerivToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[3].Visibility = Chart.ChartAxis[3].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        /* Correction Values Error */

        
        private void RollSteadyPToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[4].Visibility = Chart.ChartAxis[4].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollSteadyIToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[5].Visibility = Chart.ChartAxis[5].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        /* Correction Values Rate */

        private void RollPToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[6].Visibility = Chart.ChartAxis[6].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollIToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[7].Visibility = Chart.ChartAxis[7].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }

        private void RollDToggle_Checked(object sender, RoutedEventArgs e)
        {
            Chart.ChartAxis[8].Visibility = Chart.ChartAxis[8].Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }


        private void RollSteadyDToggle_Checked(object sender, RoutedEventArgs e)
        {
            //    DFactorLine.Visibility = DFactorLine.Visibility == Windows.UI.Xaml.Visibility.Visible ? Windows.UI.Xaml.Visibility.Collapsed : Windows.UI.Xaml.Visibility.Visible;
        }


        private void PowerDown_Click_1(object sender, RoutedEventArgs e)
        {
            MotorPower.SelectedIndex = 0;
        }

        private void Power20_Click_1(object sender, RoutedEventArgs e)
        {

            MotorPower.SelectedIndex = 2;
        }

        private void Power30_Click_1(object sender, RoutedEventArgs e)
        {
            MotorPower.SelectedIndex = 3;
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            
        }
    }
}
