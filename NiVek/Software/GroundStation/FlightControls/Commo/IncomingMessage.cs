using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class IncomingMessage
    {
        public IncomingMessage()
        {
            SerialNumber = 0;
            PayloadSize = 0;
            ExpectACK = false;
            SystemId = 0;
            MessageId = 0;
            Payload = null;
        }

        public UInt16 SerialNumber { get; set; }
        public UInt16 PayloadSize { get; set; }
        public bool ExpectACK { get; set; }
        public byte SystemId { get; set; }
        public byte MessageId { get; set; }
        
        public byte[] Payload { get; set; }

        public int TotalMessageCount { get; set; }
        public int ErrorMessageCount { get; set; }
        public float ErrorRate { get; set; }
    }
}
