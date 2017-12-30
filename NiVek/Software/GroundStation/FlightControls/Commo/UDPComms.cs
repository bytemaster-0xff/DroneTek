using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;

namespace NiVek.FlightControls.Commo
{
    public class UDPComms
    {
        DatagramSocket _incomingSocket;
        IOutputStream _outputStream;

        public async Task<bool> ConnectUDP()
        {
            _incomingSocket = new DatagramSocket();
            _incomingSocket.MessageReceived += _incomingSocket_MessageReceived;
            await _incomingSocket.BindServiceNameAsync("9050");
            _outputStream = await _incomingSocket.GetOutputStreamAsync(new HostName("10.1.1.230"), "9050");

            return true;
        }

        IncomingMessage _currentUDPMessage = null;


    }
}
