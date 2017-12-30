using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SPProxy.Server
{
    class ClientManager
    {
        bool _running;
        public event PropertyChangedEventHandler PropertyChanged;
        public EventHandler<DataReceivedEventArgs> DataReceived;
        Listener _listener;

        //All the connected clients.
        List<Client> _clients = new List<Client>();
        //When sending data if it fails, the client will be added to this list to later be removed.
        List<Client> _clientsToRemove = new List<Client>();

        public ClientManager()
        {
            IPAddresses = Dns.GetHostEntry(Dns.GetHostName()).AddressList
                    .Where(addr => addr.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                    .OrderBy(addr => addr.ToString())
                    .Select(addr => addr.ToString()).ToList();

            _listener = new Listener(IPAddress.Any, Common.ListenerPort);
            _listener.ClientConnected += (sndr, args) =>
            {
                AddClient(new Client(args.Socket));
            };
        }

        public List<String> IPAddresses { get; set; }

        private int _connectedClientCount;
        public int ConnectedClientCount
        {
            get { return _connectedClientCount; }
            set
            {
                if (_connectedClientCount != value)
                {
                    _connectedClientCount = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("ConnectedClientCount"));
                        PropertyChanged(this, new PropertyChangedEventArgs("ConnectionStatus"));
                    }
                }
            }
        }

        public string ConnectionStatus
        {
            set { }
            get { return string.Format("Number of connected clients: {0}", _connectedClientCount); }
        }

        public void AddClient(Client client)
        {
            ConnectedClientCount++;

            client.DataReceived += (sender, args) =>
                {
                    DataReceived(sender, args);
                };

            client.SocketClosed += (sender, args) =>
            {
                lock (_clients)
                {
                    _clientsToRemove.Add(sender as Client);
                }
                Console.WriteLine("Socket Closed!");
                ConnectedClientCount--;
            };

            lock (_clients)
            {
                _clients.Add(client);
            }
        }

        public void SendData(byte[] buffer)
        {
            lock (_clients)
            {
                _clients.ForEach(client => client.SendData(buffer));
                //If we can't send data to the client this class will be notified the client failed.
                //it will be added to the _clientsToRemove list to be removed from future notifications.
                _clientsToRemove.ForEach(client => _clients.Remove(client));

                //After removing, 
                _clientsToRemove.Clear();
            }
        }

        public void Start()
        {
            _listener.Start();

            var clientManagerThread = new Thread(() =>
            {
                _running = true;
                while (_running)
                {
                    lock (_clients)
                    {
                        _clients.ForEach(client => client.ReadBytes());
                    }
                    Thread.Sleep(1);
                }

            });

            clientManagerThread.Start();
        }


        public void Stop()
        {
            lock (_clientsToRemove)
            {
                _clients.ForEach(client => client.Close());
                _clients.Clear();
            }
            _listener.Stop();

            _running = false;
        }


    }

}

