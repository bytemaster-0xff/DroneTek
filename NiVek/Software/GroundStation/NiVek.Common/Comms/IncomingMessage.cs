using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    public class IncomingMessage
    {
        public const byte Ack = 0x06;
        public const byte Nak = 0x15;

        public const byte SensorMinimal = 0x10;
        public const byte SensorGps = 0x11;
        public const byte SensorBattery = 0x12;
        public const byte SystemStatus = 0x13;
        public const byte Targets = 0x14;
        public const byte MotorStatus = 0x16;
        public const byte AltitudeData = 0x17;

        public const byte SensorData = 0x80;
        public const byte PIDSensorData = 0x81;
        public const byte PIDTuningDetail = 0x82;

        public const byte SensorDiagnostics = 0x32;
        public const byte SensorFusionDiag = 0x33;
        public const byte SensorFusionConfig = 0x34;
        public const byte MSG_InvalidConfigOperation = 0x35; /* Can not configure the sensors while the device is armed */

        public const byte GPIOPidConstants = 0x20;
        public const byte MicroConfig = 0x21;

        public const byte SensorConfigSettings = 0x30;
        public const byte SensorConfigUpdated = 0x31;

        public const byte PWMDataIn = 135;
        public const byte RCChannel = 136;
        public const byte PWMCalibratedSuccess = 137;

        public const byte Ping = 0x64;

        public const byte NiVekName = 200;

        public IncomingMessage()
        {
            SerialNumber = 0;
            PayloadSize = 0;
            ExpectACK = false;
            SystemId = 0;
            MessageId = 0;
            Payload = null;
            CalcCheckSum = 0;
        }

        public UInt16 SerialNumber { get; set; }
        public UInt16 PayloadSize { get; set; }
        public bool ExpectACK { get; set; }
        public byte SystemId { get; set; }

        public byte SourceAddress { get; set; }
        public byte DestAddress { get; set; }
        public byte MessageId { get; set; }
        
        public byte[] Payload { get; set; }

        public int TotalMessageCount { get; set; }
        public int ErrorMessageCount { get; set; }
        public float ErrorRate { get; set; }

        public byte TxChecksum { get; set; }
        public byte CalcCheckSum { get; set; }

        public void ClearCheckSum()
        {
            CalcCheckSum = 0;
        }

        public void XORCheckSum(byte ch)
        {
            CalcCheckSum ^= ch;
        }

        public bool ChecksumValid
        {
            get {  return TxChecksum == CalcCheckSum;  }
        }

    }
}
