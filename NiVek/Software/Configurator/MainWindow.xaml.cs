using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace NiVekConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Commo.Channel _commsChannel;

        ObservableCollection<String> _messages;

        public MainWindow()
        {
            InitializeComponent();

            _commsChannel = new Commo.Channel();

            SerialPorts.ItemsSource = _commsChannel.SerialPorts;
            SerialPorts.SelectedItem = _commsChannel.PortName;

            _commsChannel.Connected += _commsChannel_Connected;
            _commsChannel.Disconnected += _commsChannel_Disconnected;
            _commsChannel.FailedToConnect += _commsChannel_FailedToConnect;
            _commsChannel.MessageReceived += _commsChannel_MessageReceived;

            this.Unloaded += MainWindow_Unloaded;

            _messages = new ObservableCollection<string>();

            IncomingMessages.ItemsSource = _messages;

            OutgoingMessage.IsEnabled = false;
        }

        public Visual GetDescendantByType(Visual element, Type type)
        {
            if (element == null) return null;
            if (element.GetType() == type) return element;
            Visual foundElement = null;
            if (element is FrameworkElement)
            {
                (element as FrameworkElement).ApplyTemplate();
            }
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(element); i++)
            {
                Visual visual = VisualTreeHelper.GetChild(element, i) as Visual;
                foundElement = GetDescendantByType(visual, type);
                if (foundElement != null)
                    break;
            }
            return foundElement;
        }  


        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            _commsChannel.Close();
            _commsChannel = null;
        }

        void _commsChannel_MessageReceived(object sender, string e)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                if (e.Trim().Length > 0)
                {
                    _messages.Add(e);

                    var cmdParts = e.Split('=');
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
                                if(cmdParts[1].ToLower().StartsWith("udp"))
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

                    ScrollViewer _ScrollViewer = GetDescendantByType(IncomingMessages, typeof(ScrollViewer)) as ScrollViewer;
                    _ScrollViewer.ScrollToEnd();  

                    IncomingMessages.UpdateLayout();
                    //IncomingMessages.ScrollIntoView(IncomingMessages.Items[IncomingMessages.Items.Count - 1]);

                    IncomingMessages.ScrollIntoView(_messages.Last());
                }
            }));
        }

        void _commsChannel_FailedToConnect(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                ConnectionStatus.Background = new SolidColorBrush(Colors.Red);
                ConnectionStatus.Text = "Failed Connect";
                OutgoingMessage.IsEnabled = false;
                OutgoingMessage.Text = String.Empty;
            }));
        }

        void _commsChannel_Disconnected(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                ConnectionStatus.Background = new SolidColorBrush(Colors.Red);
                ConnectionStatus.Text = "Not Connected";
                OutgoingMessage.IsEnabled = false;
                OutgoingMessage.Text = String.Empty;
            }));
        }

        void _commsChannel_Connected(object sender, EventArgs e)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                ConnectionStatus.Background = new SolidColorBrush(Colors.Green);
                ConnectionStatus.Text = "Connected";
                OutgoingMessage.IsEnabled = true;
            }));
        }

        private void Refresh_Click_1(object sender, RoutedEventArgs e)
        {
            SerialPorts.ItemsSource = _commsChannel.SerialPorts;
        }

        private void SerialPorts_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            var portName = e.AddedItems[0] as String;
            _commsChannel.Connect(portName);
        }

        private void OutgoingMessage_TextChanged_1(object sender, TextChangedEventArgs e)
        {
            if (OutgoingMessage.Text.EndsWith("\r\n"))
            {
                var msg = OutgoingMessage.Text;
                _commsChannel.Send(msg.TrimEnd('\n'));
                OutgoingMessage.Text = String.Empty;
            }
        }

        private async void SendCommandString(List<string> commands)
        {
            foreach (var cmd in commands)
            {
                _commsChannel.Send(cmd);
                await Task.Delay(500);
            }
        }

        private async void SetAP_Click_1(object sender, RoutedEventArgs e)
        {
            if (String.IsNullOrEmpty(AccessPointName.Text))
            {
                MessageBox.Show("Access Point Name is required.");
                AccessPointName.Focus();
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

            SendCommandString(script);
        }

        private void SetUDP_Click_1(object sender, RoutedEventArgs e)
        {
            var script = new List<String>()
            {
                "comms\r",
                "$$$\r",
                "set ip proto 1\r",                
                "set ip flags 0x40\r",
                "save\r",
                "reboot\r",
                "exit\r"
            };

            SendCommandString(script);
        }

        private void SetRemoteAddress_Click_1(object sender, RoutedEventArgs e)
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

            SendCommandString(script);
        }

        private void GetIPAddress_Click_1(object sender, RoutedEventArgs e)
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

            SendCommandString(script);
        }

        private void SetHostAddress_Click_1(object sender, RoutedEventArgs e)
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

            SendCommandString(script);
        }

        private void SetInfrastructure_Click_1(object sender, RoutedEventArgs e)
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

            SendCommandString(script);
        }

        private void radAccessPoint_Checked_1(object sender, RoutedEventArgs e)
        {
            
        }

        private void radInfrastructure_Checked_1(object sender, RoutedEventArgs e)
        {

        }

        private void SetProtocol_Click_1(object sender, RoutedEventArgs e)
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

            SendCommandString(script);
        }
    }
}
