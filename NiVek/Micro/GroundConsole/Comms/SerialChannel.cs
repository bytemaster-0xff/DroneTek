using NiVek.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Remoting.Channels;
using System.Text;
using System.Threading.Tasks;

namespace GroundConsole.Comms
{
    public class SerialChannel : ChannelBase
    {
        System.IO.Ports.SerialPort _commoPort;

        protected override void Send(byte[] buffer, int offset, int len)
        {
            _commoPort.Write(buffer, offset, len);
        }

        protected override void AddMessageEntry(NiVek.Common.Comms.MessageEntry entry)
        {
         
        }

        protected override void UpdateMessageEntry(NiVek.Common.Comms.MessageEntry entry)
        {
         
        }

        public override void Connect(string host, int port)
        {
            _commoPort = new System.IO.Ports.SerialPort(host, port, System.IO.Ports.Parity.None, 8, System.IO.Ports.StopBits.One);
            _commoPort.Open();
            _commoPort.DataReceived += _commoPort_DataReceived;
        }
        
        void _commoPort_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {            

            var bytesRead = _commoPort.BytesToRead;
            var buffer = new byte[bytesRead];

            _commoPort.Read(buffer, 0, bytesRead);
            Parse(buffer, 0, bytesRead);            
        }

        public override void Close()
        {
            _commoPort.Close();
            _commoPort.Dispose();
            _commoPort = null;
        }
    }
}
