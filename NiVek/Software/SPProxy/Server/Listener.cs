using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SPProxy.Server
{
    class Listener
    {
        bool _running;
        TcpListener _listener;

        public EventHandler<ClientConnectedEventArgs> ClientConnected;

        public Listener(IPAddress addr, int port)
        {
            _listener = new TcpListener(addr, port);
        }


        public void Start()
        {
            var listenerThread = new Thread(() =>
            {
                _running = true;
                _listener.Start();
                while (_running)
                {
                    try
                    {
                        var socket = _listener.AcceptSocket();
                        if (socket != null)
                        {
                            Console.WriteLine("Socket Connected.");
                            if (ClientConnected != null)
                                ClientConnected(this, new ClientConnectedEventArgs(socket));
                        }
                    }
                    catch { /* Exception thrown when listener is aborted, catch and exit while loop */ }
                }

                Console.WriteLine("Leaving while loop.");
            });

            listenerThread.Start();
        }

        public void Stop()
        {
            _running = false;
            _listener.Stop();
        }
    }
}
