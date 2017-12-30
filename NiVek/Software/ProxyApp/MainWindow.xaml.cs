using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
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

namespace ProxyApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        enum SerialPortStates
        {
            Connecting,
            Connected,
            Disconnecting,
            Disconnected,
        }

        const int MSG_OUTGOING_MSG_NAME = 200;
        const int MSG_BroadcastAddr = 0xFF;
        const int MSG_SytemId = 200;
        const int MSG_Ping = 100;
        const int MSG_SetName = 200;
        const int MSG_GetName = 201;

        SerialPortStates _serialPortState = SerialPortStates.Disconnected;

        static System.IO.Ports.SerialPort _rcvr;

        static Server.ClientManager _server;

        static Timer _socketWatchDogTimer;
        static Timer _serialWatchDogTimer;

        static Comms.MessageParser _parserFromComputer;
        static Comms.MessageParser _parserFromDrone;

        public MainWindow()
        {
            InitializeComponent();

            this.Closing += MainWindow_Closing;

            Common.COMPort = "COM8";
            Common.DeviceName = "Proxy";
            Common.LocalAddress = 10;

            initParser();
            initClient();
            initServer();
            initWatchdogs();
            
        }

        void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_rcvr != null)
            {
                try
                {
                    _server.Stop();
                    _rcvr.Dispose();

                }
                catch (Exception) { }

                _rcvr = null;
            }
        }

        void clientStartListening()
        {
            _rcvr.DataReceived += _rcvr_DataReceived;
        }

        int serialEventLoop = 0;

        void _rcvr_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            serialEventLoop++;

            Debug.WriteLine("Going in! " + serialEventLoop);

            var bytesToRead = _rcvr.BytesToRead;
            Debug.WriteLine("AFTER GETTING BYTE COUNT");
            var buffer = new byte[bytesToRead];

            _rcvr.Read(buffer, 0, bytesToRead);
            Debug.WriteLine("AFTER PERFORMING READ");

            _parserFromDrone.ParseBuffer(buffer, bytesToRead);

            Debug.WriteLine("AFTER PARSING BUFFER");

            if(_server != null)
                _server.SendData(buffer);

            serialEventLoop--;

            //Debug.WriteLine("Open Serial Event Loops " + serialEventLoop);
        }

        void initParser()
        {
            _parserFromComputer = new Comms.MessageParser(false);
            _parserFromComputer.MessageReceived += _parserFromComputer_MessageReceived;
            _parserFromComputer.MessageToBeSent += _parserFromComputer_MessageToBeSent;

            _parserFromDrone = new Comms.MessageParser(true);
            _parserFromDrone.MessageReceived += _parserFromDrone_MessageReceived;
            _parserFromDrone.MessageToBeSent += _parserFromDrone_MessageToBeSent;

        }

        void _parserFromDrone_MessageToBeSent(object sender, Comms.OutgoingMessage e)
        {
            
        }

        void _parserFromDrone_MessageReceived(object sender, Comms.MessageReceivedEventArgs e)
        {
                  if (e.Msg.SystemId == 70 && e.Msg.MessageId == 0x34)
                  {
                      Debug.WriteLine("RECEVIED SENSOR FUSION MESSAGE! {0} ", e.Msg.PayloadSize);
                  }

/*            if (e.Msg.SystemId == 100 && e.Msg.MessageId == 200)
                Debug.WriteLine("SENDING BACK NAME!");
            else
                Debug.WriteLine(String.Format("MESAGE FROM {0} {1} {2} - {3} {4}", e.Msg.SerialNumber, e.Msg.SourceAddress, e.Msg.DestAddress, e.Msg.SystemId, e.Msg.MessageId));*/
        }

        void _parserFromComputer_MessageToBeSent(object sender, Comms.OutgoingMessage e)
        {
            _server.Send(e);
        }

        void _parserFromComputer_MessageReceived(object sender, Comms.MessageReceivedEventArgs e)
        {
            if (e.Msg.DestAddress == Common.LocalAddress)
            {
                switch (e.Msg.MessageId)
                {
                    case MSG_GetName:
                        var msg = new Comms.OutgoingMessage()
                        {
                            SourceAddress = Common.LocalAddress,
                            DestinationAddress = e.Msg.DestAddress,
                            MessageId = MSG_OUTGOING_MSG_NAME,
                            ModuleType = MSG_SytemId
                        };

                        /* Sorry...stupid way, probably do with a lot less (or cleaner) code */
                        var buffer = new byte[32];
                        var nameBytes = System.Text.ASCIIEncoding.UTF8.GetBytes(Common.DeviceName);
                        for (var idx = 0; idx < buffer.Length; ++idx)
                            buffer[idx] = (idx >= nameBytes.Length) ? (byte)0x00 : nameBytes[idx];

                        msg.Payload = buffer;
                        msg.PayloadSize = 32;

                        _server.Send(msg);

                        break;
                }
            }
            else
            {
                try
                {
                    _rcvr.Write(e.Msg.Buffer, 0, e.Msg.Buffer.Length);
                    if (e.Msg.SystemId != 100 && e.Msg.MessageId != 100)
                    {
                        Debug.WriteLine("Sending Output Message {0} {1} - {2} {3} Size {4}", e.Msg.SourceAddress, e.Msg.DestAddress, e.Msg.SystemId, e.Msg.MessageId, e.Msg.Buffer.Length);

                        foreach (byte ch in e.Msg.Buffer)
                        {
                            Debug.WriteLine("BUF-> {0}", (int)ch);
                        }

                        Debug.WriteLine("/r/n");
                    }
                }
                catch(Exception )
                {
                    try
                    {
                        _rcvr.Close();
                        try
                        {
                            _rcvr.Dispose();
                        }
                        catch (Exception) { }
                    }
                    catch (Exception) { }
                    _rcvr = null;
                    _serialPortState = SerialPortStates.Disconnected;
                }
            }
        }

        void initClient()
        {
            try
            {
                _serialPortState = SerialPortStates.Connecting;
                _rcvr = new System.IO.Ports.SerialPort(Common.COMPort, 115200);
                _rcvr.Open();
                _serialPortState = SerialPortStates.Connected;
                clientStartListening();
            }
            catch (Exception)
            {
                try
                {
                    if (_rcvr != null)
                        _rcvr.Dispose();
                }
                catch (Exception) { }

                _rcvr = null;

                _serialPortState = SerialPortStates.Disconnected;
                return;
            }
        }

        void initServer()
        {
            _server = new Server.ClientManager();
            _server.Start();
            IPAddresses.ItemsSource = _server.IPAddresses;

            _server.DataReceived += (sndr, rcvArgs) =>
            {
                _parserFromComputer.ParseBuffer(rcvArgs.Buffer, rcvArgs.BufferLen);

                /*if (_rcvr.IsOpen)
                {
                    _rcvr.Write(rcvArgs.Buffer, 0, rcvArgs.BufferLen);
                }*/

            };
        }

        SolidColorBrush _yellowBrush = new SolidColorBrush(Colors.Yellow);
        SolidColorBrush _greenBrush = new SolidColorBrush(Colors.Green);
        SolidColorBrush _redBrush = new SolidColorBrush(Colors.Red);

        void initWatchdogs()
        {
            _socketWatchDogTimer = new Timer((state) =>
            {
                Dispatcher.BeginInvoke((Action)(() =>
                {
                    TCPPortStatusIndicator.Fill = _server.ConnectedClientCount > 0 ? _greenBrush : _redBrush;
                }));

                /*_server.Send(new Comms.OutgoingMessage() { 
                    SourceAddress = Common.LocalAddress, 
                    DestinationAddress = MSG_BroadcastAddr, 
                    MessageId = MSG_Ping, 
                    ModuleType =  MSG_SytemId 
                });*/

            }, null, 0, 2000);


            _serialWatchDogTimer = new Timer((state) =>
            {
                SolidColorBrush fillBrush = null;
                switch (_serialPortState)
                {
                    case SerialPortStates.Connected: fillBrush = _greenBrush; break;
                    case SerialPortStates.Disconnected:
                        initClient();
                        fillBrush = _redBrush;
                        break;
                    case SerialPortStates.Connecting:
                        fillBrush = _yellowBrush;
                        break;
                }

                Dispatcher.BeginInvoke((Action)(() =>
                {
                    COMPortStatusIndicator.Fill = fillBrush;
                }));

            }, null, 0, 2000);
        }

        private void Arm_Click(object sender, RoutedEventArgs e)
        {
            var msg = new Comms.OutgoingMessage()
            {
                ExpectACK = true,
                SourceAddress = 10,
                DestinationAddress = 20,
                ModuleType = 80,
                MessageId = 70
            };

            var buffer = msg.Buffer;
            _rcvr.Write(buffer, 0, buffer.Length);
        }

        private void Safe_Click(object sender, RoutedEventArgs e)
        {
            var msg = new Comms.OutgoingMessage()
            {
                ExpectACK = true,
                SourceAddress = 10,
                DestinationAddress = 20,
                ModuleType = 80,
                MessageId = 71
            };

            var buffer = msg.Buffer;
            _rcvr.Write(buffer, 0, buffer.Length);
        }

        private void Another_Click(object sender, RoutedEventArgs e)
        {
            var msg = new Comms.OutgoingMessage() { SourceAddress= 10, DestinationAddress = 20, MessageId = 92, ModuleType = 80, ExpectACK = true };
            msg.AddByte(1);

            msg.AddByte(70);
            msg.AddByte(20);
            msg.AddByte(10);

            var buffer = msg.Buffer;
            _rcvr.Write(buffer, 0, buffer.Length);

            foreach(var ch in buffer)
            {
                Debug.WriteLine("CHAR -> {0} ", (int)ch);
            }
        }
    }
}
