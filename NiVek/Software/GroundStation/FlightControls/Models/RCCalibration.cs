using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Models
{
    public class RCCalibration
    {
        public static RCCalibration Create(byte[] buffer)
        {
            return new RCCalibration(buffer);
        }
        
        public RCCalibration(byte[] buffer)
        {
            byte idx = 0;

            Radio1Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            Radio2Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            Radio3Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            Radio4Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            Radio5Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            Radio6Raw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);

            CalMinPitch = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalIdlePitch = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalMaxPitch = (UInt16)(buffer[idx++] | buffer[idx++] << 8);

            CalMinRoll = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalIdleRoll = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalMaxRoll = (UInt16)(buffer[idx++] | buffer[idx++] << 8);

            CalMinYaw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalIdleYaw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalMaxYaw = (UInt16)(buffer[idx++] | buffer[idx++] << 8);

            CalMinThrottle = (UInt16)(buffer[idx++] | buffer[idx++] << 8);
            CalMaxThrottle = (UInt16)(buffer[idx++] | buffer[idx++] << 8);

            Throttle = (byte)buffer[idx++];
            Pitch = (sbyte)buffer[idx++];
            Roll = (sbyte)buffer[idx++];
            Yaw = (sbyte)buffer[idx++];

            Aux1 = buffer[idx++];
            Aux2 = buffer[idx++];

            IsCalibrated = buffer[idx++] > 0;
        }

        public NiVek.FlightControls.Models.SystemStatus.SensorStates SensorState { get; private set; }
        public NiVek.FlightControls.Models.SystemStatus.ControlMethods ControlMethod { get; private set; }
        public NiVek.FlightControls.Models.SystemStatus.SystemStates GPIOAppState { get; private set; }

        public bool IsCalibrated { get; set; }

        public UInt16 Radio1Raw { get; set; }
        public UInt16 Radio2Raw { get; set; }
        public UInt16 Radio3Raw { get; set; }
        public UInt16 Radio4Raw { get; set; }
        public UInt16 Radio5Raw { get; set; }
        public UInt16 Radio6Raw { get; set; }

        public byte Throttle { get; set; }
        public sbyte Pitch { get; set; }
        public sbyte Roll { get; set; }
        public sbyte Yaw { get; set; }
        public byte Aux1 { get; set; }
        public byte Aux2 { get; set; }


        public UInt16 CalMinPitch { get; set; }
        public UInt16 CalIdlePitch { get; set; }
        public UInt16 CalMaxPitch { get; set; }

        public UInt16 CalMinRoll { get; set; }
        public UInt16 CalIdleRoll { get; set; }
        public UInt16 CalMaxRoll { get; set; }

        public UInt16 CalMinYaw { get; set; }
        public UInt16 CalIdleYaw { get; set; }
        public UInt16 CalMaxYaw { get; set; }

        public UInt16 CalMinThrottle { get; set; }
        public UInt16 CalMaxThrottle { get; set; }
    }
}
