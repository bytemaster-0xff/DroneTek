using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProxyApp.Comms
{
    public class OutgoingMessage
    {
        public const byte SOH = 0x01;
        public const byte STX = 0x02;
        public const byte ETX = 0x03;
        public const byte EOT = 0x04;

        public const byte ACK = 0x06;
        public const byte NAK = 0x15;

        public OutgoingMessage()
        {
            Payload = new byte[2048];
            PayloadSize = 0;
        }

        public bool ExpectACK { get; set; } //1 Byte
        public byte ModuleType { get; set; } //1 Byte
        public byte MessageId { get; set; } //1 Byte

        public UInt16 SerialNumber { get; set; } //2bytes
        public UInt16 PayloadSize { get; set; } //2Bytes

        public byte? SourceAddress { get; set; }

        public byte? DestinationAddress { get; set; }

        //SOH, STX, ETX, EOT  - 4 Bytes
        //Check Sum - 1 Byte
        //Total overhead is 11 bytes per message for framing.


        public byte[] Payload { get; set; } //

        public byte[] Buffer
        {
            get
            {
                byte checkSum = 0x00;

                var buffer = new byte[PayloadSize + 14]; //Add in the overhead
                var size = (UInt16)buffer.Length << 8;
                buffer[0] = SOH;
                buffer[1] = (byte)(ExpectACK ? 0xFF : 0x00);
                buffer[2] = SourceAddress.Value;
                buffer[3] = DestinationAddress.Value;
                buffer[4] = (byte)ModuleType;
                buffer[5] = (byte)MessageId;
                buffer[6] = (byte)(SerialNumber >> 8);
                buffer[7] = (byte)(SerialNumber & 0xFF);

                buffer[8] = (byte)(PayloadSize >> 8);
                buffer[9] = (byte)(PayloadSize & 0xFF);

                buffer[10] = STX;

                checkSum = 0;
                checkSum ^= (byte)(ExpectACK ? 0xFF : 0x00);
                checkSum ^= (byte)(SourceAddress.Value);
                checkSum ^= (byte)(DestinationAddress.Value);
                checkSum ^= ModuleType;
                checkSum ^= MessageId;
                checkSum ^= (byte)(SerialNumber >> 8);
                checkSum ^= (byte)(SerialNumber & 0xFF);

                for (var idx = 0; idx < PayloadSize; ++idx)
                {
                    buffer[11 + idx] = Payload[idx];
                    checkSum ^= Payload[idx];
                }

                buffer[11 + PayloadSize] = ETX;
                buffer[11 + PayloadSize + 1] = checkSum;
                buffer[11 + PayloadSize + 2] = EOT;

                return buffer;
            }
        }

        public void AddSByte(sbyte value)
        {
            Payload[PayloadSize++] = (byte)value;
        }


        public void AddByte(byte value)
        {
            Payload[PayloadSize++] = value;
        }

        public void Add(Int16 value)
        {
            Payload[PayloadSize++] = (byte)(value >> 8);
            Payload[PayloadSize++] = (byte)(value & 0xFF);
        }

        public void Add(UInt16 value)
        {
            Payload[PayloadSize++] = (byte)(value >> 8);
            Payload[PayloadSize++] = (byte)(value & 0xFF);
        }
    }

}
