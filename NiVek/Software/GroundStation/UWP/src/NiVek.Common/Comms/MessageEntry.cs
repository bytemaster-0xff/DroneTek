using NiVek.Common.Comms;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    public class MessageEntry : INotifyPropertyChanged
    {
        public enum AckStatusTypes
        {
            NA,
            Pending,
            Ack,
            NotAck,
            Timeout
        }

        public uint RowColor
        {
            get
            {
                switch (AckStatus)
                {
                    case AckStatusTypes.NA: return 0x00;
                    case AckStatusTypes.Pending: return 0xFF00FFFF;
                    case AckStatusTypes.Ack: return 0xFF00FF00;
                    case AckStatusTypes.Timeout:
                    case AckStatusTypes.NotAck: return 0xFFFF0000;
                }

                return 0x00;
            }
        }

        public byte? SourceAddress { get; set; }
        public byte? DestinationAddress { get; set; }

        public int SerialNumber { get; set; }
        public NiVek.Common.Comms.Common.ModuleTypes ModuleType { get; set; }
        public int MessageTypeId { get; set; }
        public string MessageType
        {
            get
            {
                switch (ModuleType)
                {
                    case NiVek.Common.Comms.Common.ModuleTypes.GPIO:

                        switch (MessageTypeId)
                        {
                            case GPIOModule.CMD_Kill: return "KILL!";
                            case GPIOModule.CMD_ArmNiVek: return "Arm";
                            case GPIOModule.CMD_SafeNiVek: return "Safe";
                            case GPIOModule.CMD_LaunchNiVek: return "Launch";
                            case GPIOModule.CMD_LandNiVek: return "Land";

                            case GPIOModule.CMD_ReadAllPIDConstants: return "Rd All PID";
                            case GPIOModule.CMD_SetPIDConstant: return "Set PID";
                            case GPIOModule.CMD_ResetPIDValues: return "Reset PID";

                            case GPIOModule.CMD_UseRC: return "Use RC";
                            case GPIOModule.CMD_UseWiFi: return "Use WiFi";

                            case GPIOModule.CMD_Set_AltitudeHold: return "AltHold On";
                            case GPIOModule.CMD_Clear_AltitudeHold: return "AltHoldOff";

                            case GPIOModule.CMD_ThrottleYawRollThrottle: return "CtrlInput";
                            case GPIOModule.CMD_Set_ESC_Power: return "ESC PWER";

                            case GPIOModule.CMD_Start_RC_CalibrationMode: return "Strt RC Cal";
                            case GPIOModule.CMD_CalibrateRCChanncel: return "Cal RC Chnl";
                            case GPIOModule.CMD_Cancel_RC_CalibrationMode: return "Cncl RC Cal";
                            case GPIOModule.CMD_End_RC_CalibrationMode: return "End RC Cal";

                            case GPIOModule.CMD_StartMotorConfig: return "Strt Mtr Cfg";
                            case GPIOModule.CMD_EndMotorConfig: return "End Mtr Cfg";

                            case GPIOModule.CMD_StartPIDConfig: return "Strt PID Cfg";
                            case GPIOModule.CMD_EndPIDConfig: return "End PID Cfg";


                        }
                        break;

                    case NiVek.Common.Comms.Common.ModuleTypes.System:
                        switch (MessageTypeId)
                        {
                            case Modules.NivekSystem.Ping: return "Ping";
                            case Modules.NivekSystem.WelcomePing: return "Welcome Ping";
                        }
                        break;
                    case NiVek.Common.Comms.Common.ModuleTypes.Sensor:
                        switch (MessageTypeId)
                        {
                            case SensorModule.CMD_SensorZero: return "Zero";
                            case SensorModule.CMD_SensorStart: return "Start";
                            case SensorModule.CMD_SensorStop: return "Stop";
                            case SensorModule.CMD_SensorRestart: return "Restart";
                            case SensorModule.CMD_SensorResetDefaults: return "ResetDefaults";
                            case SensorModule.CMD_BeginSensorDiagnostics: return "BeginSnsrDiag";
                            case SensorModule.CMD_EndSensorDiagnostics: return "EndSnsrDiag";

                            case SensorModule.CMD_BeginMagCalibrationXY: return "BgnMagCalXY";
                            case SensorModule.CMD_BeginMagCalibrationZ: return "BgnMagCalZ";
                            case SensorModule.CMD_CancelMagCalibration: return "CnclMagCal";
                            case SensorModule.CMD_EndMagCalibration: return "EndMagCal";

                            case SensorModule.CMD_BeginSensorFusionDiag: return "BngSnsrFus";
                            case SensorModule.CMD_EndSensorFusionDiag: return "EndSnsrFus";
                            case SensorModule.CMD_ReadSensorFusionConfig: return "ReadSnsrFus";

                            case SensorModule.CMD_SetCfg_Value: return "SetCfgValue";
                            case SensorModule.CMD_ReadCfgValues: return "ReadCfgValue";
                            case SensorModule.CMD_ResetCfg_Value: return "RestCfgValue";

                        }
                        break;
                }

                return "?";
            }

        }


        AckStatusTypes _ackStatus;
        public AckStatusTypes AckStatus
        {
            get { return _ackStatus; }
            set
            {
                _ackStatus = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("AckStatus"));

                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("RowColor"));
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
