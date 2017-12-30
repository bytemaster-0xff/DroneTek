using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace SPProxy.Server
{
    class Client : IDisposable
    {
        Socket _socket;
        byte[] _readBuffer;

        public EventHandler SocketClosed;
        public EventHandler<DataReceivedEventArgs> DataReceived;

        public Client(Socket socket) 
        {
            _socket = socket;
            _readBuffer = new byte[100];
        }

        public int BytesAvailable
        {
            get
            {
                if (_socket != null)
                {
                    try
                    {
                        return _socket.Available;
                    }
                    catch (Exception)
                    {
                        return 0;
                    }
                }
                else
                    return 0;
            }
        }

        public void ReadBytes()
        {
            if (BytesAvailable > 0)
            {
                if (_socket != null)
                {


                    var bytesRead = _socket.Receive(_readBuffer, 100, SocketFlags.None);
                    var buffer = new byte[bytesRead];
                    
                    for(var idx = 0; idx < bytesRead; ++idx)
                        buffer[idx] = _readBuffer[idx];

                    DataReceived(this, new DataReceivedEventArgs() {Buffer = buffer, BufferLen = bytesRead });
                }
            }
        }

        public void SendData(byte[] data)
        {
            if (_socket != null)
            {
                try
                {
                    _socket.Send(data);
                }
                catch (Exception)
                {
                    Close();
                    _socket = null;
                    SocketClosed(this, null);

                }
            }
        }

        public void Close()
        {
            if (_socket == null)
                throw new NullReferenceException();

            try
            {
                _socket.Shutdown(SocketShutdown.Both);
                _socket.Close();
            }
            catch { }

            _socket = null;
        }

        public void Dispose()
        {
            if (_socket != null)
                Close();
        }

    }
}