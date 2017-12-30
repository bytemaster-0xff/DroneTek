using LagoVista.Core.ViewModels;
using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Linq;

namespace NiVek.Common.Models
{
    public class SystemStatus : ViewModelBase
    {
        #region Enums that match the client
        public enum GPIOStates
        {
            Unknown = -1,
            Startup = 0,
            Ready = 50,
            Launching = 51,
            Flying = 52,
            Landing = 53,
            Crashed = 60,
            Idle = 100,
            MotorConfig = 110,
            PIDConfig = 120,
            StartRCCalibration = 160,
            RCCalibration = 161,
            RCEndCalibration = 162
        }

        public enum PlatformEnum
        {
            Full = 100,
            Mini = 101,
            Micro = 102
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

        public enum FlightModeEnum
        {
            Stable = 0,
            Acrobat = 1
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

            XYCalibratingCompass = 140,
            ZCalibratingCompass = 141,
            FinishingMagCal = 142,

            PIDConfig = 150
        }
        #endregion

        public static SystemStatus Create(byte[] buffer)
        {
            try
            {
                var status = new SystemStatus();
                status.Update(buffer);
                return status;
            }
            catch (Exception)
            {
                return null;
            }
        }

        public void Update(byte[] buffer)
        {
            int byteIndex = 0;

            FirmwareVersion = (buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0;
            Platform = (PlatformEnum)buffer[byteIndex++];
            SystemState = (SystemStates)buffer[byteIndex++];
            SensorState = (SensorStates)buffer[byteIndex++];
            GPIOState = (GPIOStates)buffer[byteIndex++];
            IsArmed = (IsArmedStates)buffer[byteIndex++];
            ControlMethod = (ControlMethods)buffer[byteIndex++];
            AltitudeHold = (AltitudeHoldEnum)buffer[byteIndex++];
            FrameConfig = (FrameConfigEnum)buffer[byteIndex++];
            FlightMode = (FlightModeEnum)buffer[byteIndex++];
        }

        #region Properties
        public bool SensorsOnline { get { return SensorState == SensorStates.Online || SensorState == SensorStates.PIDConfig; } }
        public byte[] ArmColor { get; private set; }
        public static uint _armedBrush = 0xFFFF0000;
        public static uint _safeBrush = 0xFF0000FF;

        private PlatformEnum _platform;
        public PlatformEnum Platform { get { return _platform; } private set { Set(ref _platform, value); } }
        private FlightModeEnum _flightMode;
        public FlightModeEnum FlightMode { get { return _flightMode; } private set { Set(ref _flightMode, value); } }
        private SystemStates _systemState;
        public SystemStates SystemState { get { return _systemState; } private set { Set(ref _systemState, value); } }
        private SensorStates _sensorState;
        public SensorStates SensorState { get { return _sensorState; } private set { Set(ref _sensorState, value); } }
        private ControlMethods _controlMethod;
        public ControlMethods ControlMethod
        {
            get { return _controlMethod; }
            private set
            {
                if (_controlMethod != value)
                {
                    _controlMethod = value;
                    RaisePropertyChanged(() => ControlMethod);
                    RaisePropertyChanged(() => ControlMethodText);
                }
            }
        }

        public String ControlMethodText
        {
            get
            {
                switch (_controlMethod)
                {
                    case ControlMethods.WiFi: return "WiFi";
                    case ControlMethods.RC: return "Radio";
                }

                return "???";
            }
        }
        private GPIOStates _gpioState;
        public GPIOStates GPIOState
        {
            get { return _gpioState; }
            private set
            {
                if (_gpioState != value)
                {
                    _gpioState = value;
                    RaisePropertyChanged(() => GPIOState);
                    RaisePropertyChanged(() => CanLaunch);
                    RaisePropertyChanged(() => CanLand);
                }
            }
        }
        private AltitudeHoldEnum _altitudeHold;
        public AltitudeHoldEnum AltitudeHold { get { return _altitudeHold; } private set { Set(ref _altitudeHold, value); } }
        private IsArmedStates _isArmed;
        public IsArmedStates IsArmed
        {
            get { return _isArmed; }
            private set
            {
                if (_isArmed != value)
                {
                    _isArmed = value;
                    RaisePropertyChanged(() => IsArmed);
                    RaisePropertyChanged(() => ArmedText);
                    RaisePropertyChanged(() => ArmedButtonText);
                    RaisePropertyChanged(() => ArmedColor);
                    RaisePropertyChanged(() => CanLand);
                    RaisePropertyChanged(() => CanLaunch);
                }

            }
        }
        public String ArmedText
        {
            get
            {
                if (_isArmed == IsArmedStates.Armed)
                    return "Armed";

                return "Safe";
            }
        }

        public String ArmedButtonText
        {
            get
            {
                if (_isArmed == IsArmedStates.Armed)
                    return "SAFE";

                return "ARM";
            }
        }

        public NiVek.Common.Utils.ColorManager.Color ArmedColor
        {
            get
            {
                if (_isArmed == IsArmedStates.Armed)
                    return ColorManager.Red;

                return ColorManager.Green;
            }
        }

        private FrameConfigEnum _frameConfig;
        public FrameConfigEnum FrameConfig { get { return _frameConfig; } private set { Set(ref _frameConfig, value); } }
        private double _firmwwareVersion;
        public double FirmwareVersion { get { return _firmwwareVersion; } private set { Set(ref _firmwwareVersion, value); } }
        #endregion

        public bool CanLaunch
        {
            get
            {
                return (IsArmed == IsArmedStates.Armed
                        && !(GPIOState == GPIOStates.Flying ||
                             GPIOState == GPIOStates.Launching ||
                             GPIOState == GPIOStates.Landing));
            }
        }

        public bool CanLand
        {
            get
            {
                return (IsArmed == IsArmedStates.Armed
                            && (GPIOState == GPIOStates.Flying ||
                                 GPIOState == GPIOStates.Launching ||
                                 GPIOState == GPIOStates.Landing));
            }
        }

    }
}
