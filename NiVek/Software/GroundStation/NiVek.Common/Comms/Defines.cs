using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    public class Common
    {
        public enum ModuleTypes
        {
            Sensor = 70,
            GPIO = 80,
            WiFi = 90,
            System = 100,
            Motor = 110,
            Turret = 120,
            Proxy = 200
        }

        public enum ConnectionStates
        {
            Connecting,
            Connected,
            Disconnecting,
            Disconnected
        }

        public enum ErrorCodes
        {
            MessageTooLarge,
            InvalidCRC,
            MissingSTX,
            MissingETX,
            MissingEOT
        }

        #region Constants and Defines
        public const byte SOH = 0x01;
        public const byte STX = 0x02;
        public const byte ETX = 0x03;
        public const byte EOT = 0x04;

        public const byte ACK = 0x06;
        public const byte NAK = 0x15;


        enum MessageStates
        {
            SOH,
            ExpectACK,
            SystemID,
            MsgID,
            SerialNumberMSB,
            SerialNumberLSB,
            SizeMSB,
            SizeLSB,
            STX,
            Payload,
            ETX,
            CheckSum,
            EOT
        }
        #endregion
    }
}
