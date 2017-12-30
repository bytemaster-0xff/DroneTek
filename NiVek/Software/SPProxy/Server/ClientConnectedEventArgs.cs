using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace SPProxy.Server
{
    class ClientConnectedEventArgs
    {
        private Socket _socket;
        public ClientConnectedEventArgs(Socket socket)
        {
            _socket = socket;
        }

        public Socket Socket { get { return _socket; } }
    }
}
