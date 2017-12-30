using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Usb;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage.Streams;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Console : NiVekPage
    {
        UsbDevice _consoleInterface = null;

        ObservableCollection<String> _commands = new ObservableCollection<string>();

        public Console()
        {
            this.InitializeComponent();
            ResponseStrings.ItemsSource = _commands;
            base.HeaderItem = Header;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            Connect();
        }

        private async void Connect()
        {
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
                            }
                            return;
                        }
                    }
                    catch (Exception ex)
                    {
                        //CommandStatus.Text = "Err: " + ex.Message + "\r" + ex.StackTrace;
                        //SendNow.IsEnabled = false;
                        Debug.WriteLine(ex.Message);
                    }

            }
        }

        private void ProcessMessage(String msg)
        {
            if (msg.Trim().Length > 0)
            {
                Debug.WriteLine(msg);

                var cmdParts = msg.Split('=');
                if (cmdParts.Count() == 2)
                {
                    switch (cmdParts[0].ToLower())
                    {
                        case "ip":
                            {
                                var ipParts = cmdParts[1].Split(':');
                                RemoteAddress.Text = ipParts[0];
                                RemotePort.Text = ipParts[1];
                            }
                            break;
                        case "flags":
                            break;
                        case "deviceid":
                            AccessPointName.Text = cmdParts[1];
                            break;
                        case "proto":
                            if (cmdParts[1].ToLower().StartsWith("udp"))
                                radUDP.IsChecked = true;
                            else if (cmdParts[1].ToLower().StartsWith("tcp"))
                                radTCP.IsChecked = true;

                            break;
                        case "join":
                            switch (cmdParts[1])
                            {
                                case "1": radInfrastructure.IsChecked = true; break;
                                case "7": radAccessPoint.IsChecked = true; break;

                            }
                            break;
                        case "host":
                            {
                                var ipParts = cmdParts[1].Split(':');
                                HostAddress.Text = ipParts[0];
                            }
                            break;
                        case "ssid":
                            SSID.Text = cmdParts[1];
                            break;

                        case "passphrase":
                            SSIDPassword.Text = cmdParts[1];
                            break;
                    }
                }
            }
        }

        private StringBuilder _parsedStringBuffer = new StringBuilder();

        private void HandleInputString(String str)
        {
            foreach(var ch in str.ToCharArray())
            {
                if (ch == '\n' || ch == '\r')
                {
                    if(_parsedStringBuffer.Length > 0)
                        ProcessMessage(_parsedStringBuffer.ToString());

                    _parsedStringBuffer.Clear();
                }
                else
                    _parsedStringBuffer.Append(ch);

            }

        }

        async void Console_DataReceived(UsbInterruptInPipe sender, UsbInterruptInEventArgs args)
        {
            IBuffer buffer = args.InterruptData;

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                var bytes = buffer.ToArray();
                var inputString = System.Text.Encoding.UTF8.GetString(bytes, 0, (int)buffer.Length);
                inputString = inputString.TrimEnd('\r');

                _commands.Add(inputString);                
                HandleInputString(inputString);
            });
        }

        private async void SendCommandString(String command)
        {
            UsbBulkOutPipe writePipe = _consoleInterface.DefaultInterface.BulkOutPipes[0];

            var stream = writePipe.OutputStream;
            DataWriter writer = new DataWriter(stream);
            writer.WriteString(command);

            UInt32 bytesWritten = 0;
            try
            {
                Debug.WriteLine("Writing Data to End Point: " + writePipe.EndpointDescriptor.EndpointNumber);
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

        private async void SendCommandStrings(List<String> commands)
        {
            UsbBulkOutPipe writePipe = _consoleInterface.DefaultInterface.BulkOutPipes[0];

            foreach (var command in commands)
            {
                var stream = writePipe.OutputStream;
                DataWriter writer = new DataWriter(stream);
                writer.WriteString(command);

                await Task.Delay(750);

                UInt32 bytesWritten = 0;
                try
                {
                    Debug.WriteLine("Writing Data to End Point: " + writePipe.EndpointDescriptor.EndpointNumber);
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
        }

        private void Commands_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (CommandInput.Text.EndsWith("\r\n"))
            {
                SendCommandString(CommandInput.Text.TrimEnd('\n'));
                CommandInput.Text = String.Empty;
            }
        }

        private void GetIPAddress_Click(object sender, RoutedEventArgs e)
        {
            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "get ip \r",
                "get wlan \r",
                "get opt \r",
                "reboot\r",
                "exit\r"
            };

            SendCommandStrings(script);
        }

        private void radAccessPoint_Click(object sender, RoutedEventArgs e)
        {

        }

        private void radInfrastructure_Checked(object sender, RoutedEventArgs e)
        {

        }

        private void SetInfrastructure_Click(object sender, RoutedEventArgs e)
        {
            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set wlan join 1 \r",
                "set w s " + SSID.Text + "\r",
                "set w p " + SSIDPassword.Text + "\r",
                "save\r",
                "reboot\r",
                "exit\r"
            };

            SendCommandStrings(script);
        }

        private void SetHostAddress_Click(object sender, RoutedEventArgs e)
        {
            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set ip host " + HostAddress.Text + "\r",
                "save\r",
                "reboot\r",
                "exit\r"
            };

            SendCommandStrings(script);
        }

        private void SetProtocol_Click(object sender, RoutedEventArgs e)
        {
            var ipMode = "1";
            if (radTCP.IsChecked.HasValue && radTCP.IsChecked.Value)
                ipMode = "2";

            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set ip proto " + ipMode + "\r",
                "save\r",
                "reboot\r",
                "exit\r"
            };

            SendCommandStrings(script);
        }

        private void SetRemoteAddress_Click(object sender, RoutedEventArgs e)
        {
            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set ip dhcp 0\r",
                "set ip address " + RemoteAddress.Text  + "\r",                
                "set ip remote " + RemotePort.Text + "\r",                
                "set ip localport " + RemotePort.Text + "\r",                
                "save\r",
                "reboot\r",
                "exit\r"
            };

            SendCommandStrings(script);
        }

        private async void SetAP_Click(object sender, RoutedEventArgs e)
        {
            if (String.IsNullOrEmpty(AccessPointName.Text))
            {
                var msgDialog = new MessageDialog("Access Point Name is required.");
                await msgDialog.ShowAsync();
                return;
            }

            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set w j 7\r",
                "set i d 4\r",
                "set i a 1.2.3.4\r",
                "set i h 0.0.0.0\r",
                "set w s " + AccessPointName.Text + "\r",
                "set o d " + AccessPointName.Text + "\r",
                "save\r",
                "reboot\r",
                "exit\r",
            };

            SendCommandStrings(script);
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
            
        }
    }
}
