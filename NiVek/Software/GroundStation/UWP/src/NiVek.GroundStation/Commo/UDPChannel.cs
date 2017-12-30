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
using NiVek.Common.Comms;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class UDPChannel : Channel
    {
        DatagramSocket _incomingSocket;
        IOutputStream _outputStream;

        public UDPChannel()
        {

        }


        protected async override void Send(byte[] buffer, int offset, int len)
        {
            if (_outputStream != null)
                await _outputStream.WriteAsync(buffer.AsBuffer());
        }

        public async override Task ConnectAsync(String host, short port)
        {
            Debug.WriteLine("Attempt to connect to {0} on port {1}", host, port);

            Connecting();

            _incomingSocket = new DatagramSocket();
            _incomingSocket.MessageReceived += _incomingSocket_MessageReceived;
            await _incomingSocket.BindServiceNameAsync(port.ToString());
            try
            {
                _outputStream = await _incomingSocket.GetOutputStreamAsync(new HostName(host), port.ToString());
            }
            catch (Exception ex)
            {
                DidNotConnect(ex.Message);
                Debug.WriteLine("Could not connect, will retry soon.");
            }
        }

        void _incomingSocket_MessageReceived(DatagramSocket sender, DatagramSocketMessageReceivedEventArgs args)
        {
            var buffer = args.GetDataReader().DetachBuffer().ToArray();

            Parse(buffer, 0, buffer.Length);
        }

        public override void Disconnect(String reason)
        {
            if (ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {
                Disconnecting(reason);

                if (_incomingSocket != null)
                    _incomingSocket.Dispose();

                if (_outputStream != null)
                    _outputStream.Dispose();

                _outputStream = null;
                _incomingSocket = null;

                Disconnected(reason);
            }
        }
    }
}
