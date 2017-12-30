using NiVek.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Windows.Threading;
using NiVek.Common.Comms;


namespace NiVek.WP8.Comms
{
    public class UDPChannel : Channel
    {
        DispatcherTimer _commoTimeout; 
        DatagramSocket _socket;
        IOutputStream _outputStream;

        int _portNumber;
        String _address;
        
        public UDPChannel()
        {
            _socket = new DatagramSocket();
            _socket.MessageReceived += _socket_MessageReceived;

            _commoTimeout = new DispatcherTimer();
            _commoTimeout.Interval = TimeSpan.FromMilliseconds(250);
            _commoTimeout.Tick += _commoTimeout_Tick;
            _commoTimeout.Start();
        }

        void _commoTimeout_Tick(object sender, object e)
        {
            //SendPing("Welcome", NiVek.Common.Modules.NivekSystem.Ping);
        //    CheckWatchdog();
        }

        void _socket_MessageReceived(DatagramSocket sender, DatagramSocketMessageReceivedEventArgs args)
        {
            var buffer = args.GetDataReader().DetachBuffer().ToArray();

            Parse(buffer, 0, buffer.Length);
        }

        protected override async void Send(byte[] buffer, int offset, int len)
        {            
            await _outputStream.WriteAsync(buffer.AsBuffer());
        }

        public async override void Connect(string host, int port)
        {
            _address = host;
            _portNumber = port;

            await _socket.BindServiceNameAsync(port.ToString());

            _outputStream = await _socket.GetOutputStreamAsync(new HostName(host), port.ToString());

            //SendPing("Welcome", NiVek.Common.Modules.NivekSystem.WelcomePing);
        }

        public override void Close()
        {
            _socket.Dispose();
        }
    }
}
