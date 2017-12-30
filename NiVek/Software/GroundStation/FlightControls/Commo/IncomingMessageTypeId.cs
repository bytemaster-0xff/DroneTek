using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class IncomingMessageTypeId
    {
        public const byte Ack = 0x06;
        public const byte Nak = 0x15;

        public const byte SensorMinimal = 0x10;
        public const byte SensorGps = 0x11;
        public const byte SensorBattery = 0x12;
        public const byte SystemStatus = 0x13;
        public const byte Targets = 0x14;
        public const byte MotorStatus = 0x16;

        public const byte SensorData = 0x80;
        public const byte PIDSensorData = 0x81;
        

        public const byte SensorDiagnostics = 0x32;
        public const byte SensorFusionDiag = 0x33;
        public const byte SensorFusionConfig = 0x34;
        public const byte MSG_InvalidConfigOperation = 0x35; /* Can not configure the sensors while the device is armed */

        public const byte GPIOPidConstants = 0x20;

        public const byte SensorConfigSettings = 0x30;
        public const byte SensorConfigUpdated = 0x31;

        public const byte PWMDataIn = 135;
        public const byte RCChannel = 136;
        public const byte PWMCalibratedSuccess = 137;

        public const byte Ping = 0x64;
    }
}
