using LagoVista.Core.IOC;
using NiVek.Common.Comms;
using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Usb;
using Windows.Storage.Streams;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    public class ReceivedLine
    {
        public DateTime TimeStamp { get; set; }
        public String Message { get; set; }
        public int Idx { get; set; }
    }

    public sealed partial class DroneSettings : Page
    {
        UsbDevice _consoleInterface = null;
        ObservableCollection<String> _commands = new ObservableCollection<string>();


        private List<String> GET_DEVICE_SCRIPT = new List<String>()
            {
                "get n\r",
                "get a\r",
                "get s\r"
            };

        public DroneSettings()
        {
            this.InitializeComponent();

            DataContext = new NiVek.Common.ViewModels.DroneSettings(SLWIOC.Get<IStorageContext>());
            ViewModel.SendCommands += async (sndr, cmds) =>
                {
                    await SendCommandStrings(cmds);
                };
        }

        private NiVek.Common.ViewModels.DroneSettings ViewModel
        {
            get { return DataContext as NiVek.Common.ViewModels.DroneSettings; }
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            await Connect();

            DeviceConsole.ItemsSource = _receivedCommands;

            if (e.Parameter == null)
                ViewModel.CreateNewDrone();
            else
                ViewModel.SetDrone(e.Parameter as Drone);

        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            OnDisconnectVisualChildren();
            Disconnect();
        }


        private async void QueryDevice()
        {
            await SendCommandStrings(GET_DEVICE_SCRIPT);
        }

        private void Disconnect()
        {
            if (_consoleInterface != null)
            {
                _consoleInterface.Dispose();
                _consoleInterface = null;
            }
        }

        private async Task Connect()
        {
            CommandInput.IsEnabled = false;
            ConnStatus.Fill = new SolidColorBrush(Colors.Yellow);

            var str = Windows.Devices.Usb.UsbDevice.GetDeviceSelector(new Guid("{F70242C7-FB25-443B-9E7E-A4260F373981}"));
            var devices = await DeviceInformation.FindAllAsync(str);
            if (devices.Count == 0)
            {
                Debug.WriteLine("THIS IS A COUNT OF 0 FOR DEVICE :(");
                return;
            }

            foreach (var device in devices)
            {
                Debug.WriteLine(device.Name);
                Debug.WriteLine(device.Id);
                foreach (var prop in device.Properties)
                {
                    try
                    {
                        var dvc = await UsbDevice.FromIdAsync(device.Id);

                        if (dvc != null)
                        {
                            _consoleInterface = dvc;

                            Debug.WriteLine("Connected to Cnsl Intrfc:\r\n  Bulk Out: " + _consoleInterface.DefaultInterface.BulkOutPipes.Count + "\r\n Bulk In: " + _consoleInterface.DefaultInterface.BulkInPipes.Count + "\r\n  IRQ Out: " + _consoleInterface.DefaultInterface.InterruptOutPipes.Count + "\r\n IRQ In: " + _consoleInterface.DefaultInterface.InterruptInPipes.Count + "\r\n");

                            if (_consoleInterface.DefaultInterface.InterruptInPipes.Count > 0)
                            {
                                var irqPipe = _consoleInterface.DefaultInterface.InterruptInPipes[0];
                                irqPipe.DataReceived += Console_DataReceived;
                                ConnStatus.Fill = new SolidColorBrush(Colors.Green);
                                CommandInput.IsEnabled = true;
                                QueryDevice();
                            }

                            return;
                        }
                    }
                    catch (Exception ex)
                    {
                        ConnStatus.Fill = new SolidColorBrush(Colors.Red);
                        Debug.WriteLine(ex.Message);
                    }
                }
            }
        }

        StringBuilder _inputBuffer = new StringBuilder();
        private ObservableCollection<ReceivedLine> _receivedCommands = new ObservableCollection<ReceivedLine>();

        async void Console_DataReceived(UsbInterruptInPipe sender, UsbInterruptInEventArgs args)
        {
            IBuffer buffer = args.InterruptData;

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                var bytes = buffer.ToArray();
                foreach (var ch in bytes)
                {
                    if (ch == '\r')
                    {
                        if (_inputBuffer.ToString().Trim() != String.Empty)
                        {
                            var msg = _inputBuffer.ToString().Trim();
                            var vm = DataContext as NiVek.Common.ViewModels.DroneSettings;
                            vm.ProcessMessage(msg);
                            _receivedCommands.Add(new ReceivedLine()
                            {

                                Message = msg,
                                TimeStamp = DateTime.Now,
                                Idx = _receivedCommands.Count + 1
                            });

                            _commands.Add(_inputBuffer.ToString());
                            DeviceConsole.UpdateLayout();
                            DeviceConsole.ScrollIntoView(_receivedCommands.Last());
                        }

                        _inputBuffer.Clear();
                    }
                    else
                    {
                        _inputBuffer.Append((char)ch);
                    }
                }
            });
        }

        private async void SendCommandString(String command)
        {
            UsbBulkOutPipe writePipe = _consoleInterface.DefaultInterface.BulkOutPipes[0];

            var stream = writePipe.OutputStream;
            var writer = new DataWriter(stream);
            writer.WriteString(command);

            UInt32 bytesWritten = 0;
            try
            {
                bytesWritten = await writer.StoreAsync();
            }
            catch (Exception exception)
            {
                Debug.WriteLine("ERROR WRITING: " + exception);
            }
            finally
            {
                Debug.WriteLine("Data written: " + bytesWritten + " bytes.");
            }
        }

        private async Task SendCommandStrings(List<String> commands)
        {
            UsbBulkOutPipe writePipe = _consoleInterface.DefaultInterface.BulkOutPipes[0];

            foreach (var command in commands)
            {
                SendCommandString(command);
                await Task.Delay(250);
            }
        }

        private void Commands_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (CommandInput.Text.EndsWith("\r\n"))
            {
                SendCommandString(CommandInput.Text.TrimEnd('\n'));
                CommandInput.Text = String.Empty;
            }
        }

        private void BackButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.Frame != null && this.Frame.CanGoBack)
            {
                if (_consoleInterface != null)
                    _consoleInterface.Dispose();

                base.Frame.GoBack();
            }
        }
    }
}
