using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class OutgoingMessage
    {
        public enum ModuleTypes
        {
            Sensor = 70,
            GPIO = 80,
            WiFi = 90,
            System = 100,
            Proxy = 200
        }

        public OutgoingMessage()
        {
            Payload = new byte[2048];
            PayloadSize = 0;
        }

        public bool ExpectACK { get; set; } //1 Byte
        public ModuleTypes ModuleType { get; set; } //1 Byte
        public byte MessageId { get; set; } //1 Byte
        
        public UInt16 SerialNumber { get; set; } //2bytes
        public UInt16 PayloadSize { get; set; } //2Bytes

        //SOH, STX, ETX, EOT  - 4 Bytes
        //Check Sum - 1 Byte
        //Total overhead is 11 bytes per message for framing.
        

        public byte[] Payload { get; set; } //

        public byte[] Buffer
        {
            get
            {
                byte checkSum = 0x00;
                
                var buffer = new byte[PayloadSize + 12]; //Add in the overhead
                var size = (UInt16)buffer.Length << 8;
                buffer[0] = NiVekConsole.SOH;
                buffer[1] = (byte)(ExpectACK ? 0xFF : 0x00);
                buffer[2] = (byte)ModuleType;
                buffer[3] = (byte)MessageId;
                buffer[4] = (byte)(SerialNumber >> 8);
                buffer[5] = (byte)(SerialNumber & 0xFF);                

                buffer[6] = (byte)(PayloadSize >> 8);
                buffer[7] = (byte)(PayloadSize & 0xFF);

                buffer[8] = NiVekConsole.STX;

                for (var idx = 0; idx < PayloadSize; ++idx)
                {
                    buffer[9 + idx] = Payload[idx];
                    checkSum += Payload[idx];
                }

                buffer[9 + PayloadSize] = NiVekConsole.ETX;
                buffer[9 + PayloadSize + 1] = checkSum;
                buffer[9 + PayloadSize + 2] = NiVekConsole.EOT;

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
