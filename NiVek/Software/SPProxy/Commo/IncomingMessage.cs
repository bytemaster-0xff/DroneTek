using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SPProxy.Commo
{
    public class IncomingMessage
    {
        public const byte SOH = 0x01;
        public const byte STX = 0x02;
        public const byte ETX = 0x03;
        public const byte EOT = 0x04;

        public const byte ACK = 0x06;
        public const byte NAK = 0x15;


        public IncomingMessage()
        {
            SerialNumber = 0;
            PayloadSize = 0;
            ExpectACK = false;
            SystemId = 0;
            MessageId = 0;
            Payload = null;
        }

        public byte SourceAddress { get; set; }
        public byte DestAddress { get; set; }

        public UInt16 SerialNumber { get; set; }
        public UInt16 PayloadSize { get; set; }
        public bool ExpectACK { get; set; }
        public byte SystemId { get; set; }
        public byte MessageId { get; set; }

        public byte[] Payload { get; set; }

        public byte TxChecksum { get; set; }
        public byte CalcCheckSum { get; set; }


        public static IncomingMessage GetProxyPing()
        {
            var msg = new IncomingMessage()
            {
                SystemId = 200, /* Proxy System Id */
                MessageId = 0x64 /* Ping Message Id */
            };

            return msg;
        }

        public byte[] Buffer
        {
            get
            {
                byte checkSum = 0x00;

                var buffer = new byte[PayloadSize + 12]; //Add in the overhead
                var size = (UInt16)buffer.Length << 8;
                buffer[0] = IncomingMessage.SOH;
                buffer[1] = (byte)(ExpectACK ? 0xFF : 0x00);
                buffer[2] = (byte)SystemId;
                buffer[3] = (byte)MessageId;
                buffer[4] = (byte)(SerialNumber >> 8);
                buffer[5] = (byte)(SerialNumber & 0xFF);

                buffer[6] = (byte)(PayloadSize >> 8);
                buffer[7] = (byte)(PayloadSize & 0xFF);

                buffer[8] = IncomingMessage.STX;

                for (var idx = 0; idx < PayloadSize; ++idx)
                {
                    buffer[9 + idx] = Payload[idx];
                    checkSum += Payload[idx];
                }

                buffer[9 + PayloadSize] = IncomingMessage.ETX;
                buffer[9 + PayloadSize + 1] = checkSum;
                buffer[9 + PayloadSize + 2] = IncomingMessage.EOT;

                return buffer;
            }
        }

        public void XORCheckSum(byte ch)
        {
            CalcCheckSum ^= ch;
        }

        public bool ChecksumValid
        {
            get { return TxChecksum == CalcCheckSum; }
        }

    }
}
