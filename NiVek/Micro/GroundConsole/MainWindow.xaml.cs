using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace GroundConsole
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        Comms.SerialChannel _comms;

        Timer _timer;


        DateTime _lastUpdated = DateTime.MinValue;

        public MainWindow()
        {
            InitializeComponent();

            _comms = new Comms.SerialChannel();
            _comms.Connect("COM8", 115200);
            _comms.MessageReady += _comms_MessageReady;

            _timer = new Timer();
            _timer.Elapsed += _timer_Elapsed;
            _timer.Interval = 250;
            _timer.Start();

            DisconnectedMask.Visibility = System.Windows.Visibility.Visible;
        }

        private Quaternion qX = new Quaternion(new Vector3D(1, 0, 0), 1); //rotations around X-axis
        private Quaternion qY = new Quaternion(new Vector3D(0, 1, 0), 1); //rotations around Y-axis
        private Quaternion qZ = new Quaternion(new Vector3D(0, 0, 1), 1); //rotations around Z-axis

        private MultiWiiPID _wiiPid = null;
        private SensorMinimal _snsrData = null;

        SolidColorBrush RedBrush = new SolidColorBrush(Colors.Red);
        SolidColorBrush GreenBrush = new SolidColorBrush(Colors.Green);

        void _comms_MessageReady(object sender, NiVek.Common.Comms.IncomingMessage e)
        {
            _lastUpdated = DateTime.Now;

            switch ((NiVek.Common.Comms.Common.ModuleTypes)e.SystemId)
            {
                case NiVek.Common.Comms.Common.ModuleTypes.Sensor:
                    switch (e.MessageId)
                    {
                        case IncomingMessage.SensorMinimal:
                            Dispatcher.BeginInvoke((Action)(() =>
                            {
                                var bufferReader = new BufferReader(e.Payload, 0, e.PayloadSize);

                                if (_snsrData == null)
                                {
                                    _snsrData = SensorMinimal.Create(bufferReader);
                                    LiveData.DataContext = _snsrData;
                                }
                                else
                                {
                                    _snsrData.Update(bufferReader);
                                }

                                var pitch = _snsrData.PitchAngle / 90.0f;
                                var roll = _snsrData.RollAngle / 90.0f;

                                var rotation = Matrix3D.Identity;
                                rotation.Rotate(new Quaternion(new Vector3D(0, 1, 0), 180));
                                rotation.Rotate(new Quaternion(new Vector3D(1, 0, 0), _snsrData.PitchAngle));
                                rotation.Rotate(new Quaternion(new Vector3D(0, 0, 1), _snsrData.RollAngle));
                                SphereGeometryModel.Transform = new MatrixTransform3D(rotation);

                                CurrentStatus.Background = _snsrData.IsArmed ? RedBrush : GreenBrush;

                                ErrorRate.Text = String.Format("{0:0.0}% ({1} - {2})", e.ErrorRate, e.ErrorMessageCount, e.TotalMessageCount);
                            }));

                            break;
                    }

                    break;
                case NiVek.Common.Comms.Common.ModuleTypes.GPIO:
                    switch (e.MessageId)
                    {
                    }
                    break;
            }

            var msg = e.SerialNumber;
        }

        void _timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                if ((DateTime.Now - _lastUpdated).TotalMilliseconds > 500)
                    DisconnectedMask.Visibility = System.Windows.Visibility.Visible;
                else
                    DisconnectedMask.Visibility = System.Windows.Visibility.Collapsed;
            }));
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {


        }

        private void CalibrateSensors_Click(object sender, RoutedEventArgs e)
        {
        }

        private void GetData_Click(object sender, RoutedEventArgs e)
        {
            _comms.Safe();
        }

        private void SensorData_Click(object sender, RoutedEventArgs e)
        {
            _comms.Arm();
        }

        private void ZeroCopter_Click(object sender, RoutedEventArgs e)
        {
            _comms.Send(new NiVek.Common.OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_SensorZero, ExpectACK = true });
        }

        private async void CurrentStatus_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_snsrData.IsArmed)
            {
                _comms.Safe();
                await Task.Delay(100);
                _comms.Safe();
                await Task.Delay(100);
                _comms.Safe();
            }
            else
                _comms.Arm();
        }

        private void LiveData_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Up)
                _comms.Throttle++;
            else if (e.Key == Key.Down)
                _comms.Throttle--;
            else if (e.Key == Key.A)
                _comms.Roll--;
            else if (e.Key == Key.D)
                _comms.Roll++;
            else if (e.Key == Key.W)
                _comms.Pitch--;
            else if (e.Key == Key.X)
                _comms.Pitch++;

            _comms.SourceAddress = 1;
            _comms.DestinationAddress = 10;
            _comms.SendControlUpdate();
        }

        private void LiveData_KeyUp(object sender, KeyEventArgs e)
        {

        }
    }
}
