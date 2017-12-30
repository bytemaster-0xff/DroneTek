using NiVek.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.UI.Xaml;
using System.Diagnostics;
using Windows.UI.Core;

namespace NiVek.FlightControls.Commo
{
    public class UDPChannel : ChannelBase
    {
        DatagramSocket _incomingSocket;
        IOutputStream _outputStream;

        DispatcherTimer _commoTimeout;

        public UDPChannel()
        {
            _commoTimeout = new DispatcherTimer();
            _commoTimeout.Interval = TimeSpan.FromMilliseconds(500);
            _commoTimeout.Tick += _commoTimeout_Tick;
        }

        void _commoTimeout_Tick(object sender, object e)
        {
            SendPing("Welcome", NiVek.Common.Modules.NivekSystem.Ping);
            CheckWatchdog();
        }

        protected async override void Send(byte[] buffer, int offset, int len)
        {
            if(_outputStream != null)
                await _outputStream.WriteAsync(buffer.AsBuffer());
        }

        public async override void Connect(String host, int port)
        {
            Debug.WriteLine("Attempt to connect to {0} on port {1}", host, port);

            _incomingSocket = new DatagramSocket();
            _incomingSocket.MessageReceived += _incomingSocket_MessageReceived;
            await _incomingSocket.BindServiceNameAsync(port.ToString());
            try
            {
                _outputStream = await _incomingSocket.GetOutputStreamAsync(new HostName(host), port.ToString());

                SendPing("Welcome", NiVek.Common.Modules.NivekSystem.WelcomePing);

             
            }
            catch (Exception)
            {
                Debug.WriteLine("Could not connect, will retry soon.");
            }

            _commoTimeout.Start();
        }

        void _incomingSocket_MessageReceived(DatagramSocket sender, DatagramSocketMessageReceivedEventArgs args)
        {
            var buffer = args.GetDataReader().DetachBuffer().ToArray();

            Parse(buffer, 0, buffer.Length);
        }

        public override void Close()
        {
            _commoTimeout.Stop();
            
            if(_incomingSocket != null)
                _incomingSocket.Dispose();
            
            if (_outputStream != null)
                _outputStream.Dispose();
            _outputStream = null;
            _incomingSocket = null;
        }
    }
}
