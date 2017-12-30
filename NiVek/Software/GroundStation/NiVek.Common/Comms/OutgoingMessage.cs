using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common
{
    public class OutgoingMessage
    {
        public OutgoingMessage()
        {
            Payload = new byte[2048];
            PayloadSize = 0;
        }

        public bool ExpectACK { get; set; } //1 Byte
        public NiVek.Common.Comms.Common.ModuleTypes ModuleType { get; set; } //1 Byte
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
                buffer[0] = Common.Comms.Common.SOH;
                buffer[1] = (byte)(ExpectACK ? 0xFF : 0x00);
                buffer[2] = SourceAddress.Value;
                buffer[3] = DestinationAddress.Value;
                buffer[4] = (byte)ModuleType;
                buffer[5] = (byte)MessageId;
                buffer[6] = (byte)(SerialNumber >> 8);
                buffer[7] = (byte)(SerialNumber & 0xFF);

                buffer[8] = (byte)(PayloadSize >> 8);
                buffer[9] = (byte)(PayloadSize & 0xFF);

                buffer[10] = Common.Comms.Common.STX;

                checkSum ^= buffer[1];
                checkSum ^= buffer[2];
                checkSum ^= buffer[3];
                checkSum ^= buffer[4];
                checkSum ^= buffer[5];
                checkSum ^= buffer[6];
                checkSum ^= buffer[7];

                for (var idx = 0; idx < PayloadSize; ++idx)
                {
                    buffer[11 + idx] = Payload[idx];
                    checkSum ^= Payload[idx];
                }

                buffer[11 + PayloadSize] = Common.Comms.Common.ETX;
                buffer[11 + PayloadSize + 1] = checkSum;
                buffer[11 + PayloadSize + 2] = Common.Comms.Common.EOT;

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
