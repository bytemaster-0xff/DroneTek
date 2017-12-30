using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI;
using Windows.UI.Xaml.Media;

namespace NiVek.FlightControls.Models
{
    public class SystemStatus
    {
        #region Enums that match the client
        public enum GPIOStates
        {
            Unknown = -1,
            Startup = 0,
            Flight = 50,
            Idle = 100,
            MotorConfig = 110,
            PIDConfig = 120,
            StartRCCalibration = 160,
            RCCalibration = 161,
            RCEndCalibration = 162
        }

        public enum SystemStates
        {
            Unknown = -1,
            StartingUp = 1,
            RestartRequired = 10,
            Restarting = 11,
            Zeroing = 20,
            ResettingDefaults = 30,
            Ready = 100,
            Failure = 0
        }


        public enum IsArmedStates
        {
            unknown = -1,
            Armed = 0xFF,
            Safe = 0x00
        }

        public enum ControlMethods
        {
            Unknown = -1,
            None = 0,
            RC = 40,
            WiFi = 20
        }

        public enum AltitudeHoldEnum
        {
            Uknown = -1,
            On = 255,
            Off = 0
        }

        public enum FrameConfigEnum
        {
            Cross = 0,
            X = 1
        }


        public enum SensorStates
        {
            Unknown = -1,

            Startup = 0,

            Initializing = 1,
            Ready = 2,
            Restart = 10,
            ResettingDefaults = 11,

            Starting = 100,
            Restarting = 101,
            Resuming = 102,
            Online = 103,
            Stopping = 104,

            Offline = 105,
            Pausing = 106,
            Paused = 107,

            Diagnostics = 108,

            Zero = 110,
            Zeroing = 111,
            ZeroCompleted = 112,

            FusionPitch = 120,
            FusionRoll = 121,
            FusionHeading = 122,
            FusionAltitude = 123,

            CalibratingCompass = 140,
            FinishingMagCal = 141,

            PIDConfig = 150
        }
        #endregion

        public static SystemStatus Create(byte[] buffer)
        {
            try
            {
                var status = new SystemStatus();
                int byteIndex = 0;

                status.SystemState = (SystemStates)buffer[byteIndex++];
                status.SensorState = (SensorStates)buffer[byteIndex++];
                status.GPIOState = (GPIOStates)buffer[byteIndex++];
                status.IsArmed = (IsArmedStates)buffer[byteIndex++];
                status.ControlMethod = (ControlMethods)buffer[byteIndex++];
                status.AltitudeHold = (AltitudeHoldEnum)buffer[byteIndex++];
                status.FrameConfig = (FrameConfigEnum)buffer[byteIndex++];

                status.ArmColor = status.IsArmed == IsArmedStates.Armed ? _armedBrush : _safeBrush;
                status.IsArmedText = status.IsArmed == IsArmedStates.Armed ? "ARMED" : "SAFE";
                status.ArmButtonText = status.IsArmed == IsArmedStates.Armed ? "Safe" : "Arm";

                return status;
            }
            catch (Exception)
            {
                return null;
            }
        }

        #region Properties
        public bool SensorsOnline { get { return SensorState == SensorStates.Online || SensorState == SensorStates.PIDConfig; } }
        public SolidColorBrush ArmColor { get; private set; }
        public static SolidColorBrush _armedBrush = new SolidColorBrush(Colors.Red);
        public static SolidColorBrush _safeBrush = new SolidColorBrush(Colors.Green);
        public SystemStates SystemState { get; private set; }
        public SensorStates SensorState { get; private set; }
        public ControlMethods ControlMethod { get; private set; }
        public GPIOStates GPIOState { get; private set; }
        public AltitudeHoldEnum AltitudeHold { get; private set; }
        public IsArmedStates IsArmed { get; private set; }
        public String IsArmedText { get; private set; }
        public String ArmButtonText { get; private set; }
        public FrameConfigEnum FrameConfig {get; private set; }
        #endregion
    }
}
