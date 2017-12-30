using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI;
using Windows.UI.Xaml.Media;

namespace NiVek.FlightControls.Commo
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

        public SolidColorBrush RowColor
        {
            get
            {
                switch (AckStatus)
                {
                    case AckStatusTypes.NA: return new SolidColorBrush(Colors.Transparent);
                    case AckStatusTypes.Pending: return new SolidColorBrush(Colors.Yellow);
                    case AckStatusTypes.Ack: return new SolidColorBrush(Colors.Green);
                    case AckStatusTypes.Timeout:
                    case AckStatusTypes.NotAck: return new SolidColorBrush(Colors.Red); 
                }

                return new SolidColorBrush(Colors.Transparent);
            }
        }

        public int SerialNumber { get; set; }
        public NiVek.FlightControls.Commo.OutgoingMessage.ModuleTypes ModuleType { get; set; }
        public int MessageTypeId { get; set; }
        public string MessageType 
        {
            get
            {
                switch (ModuleType)
                {
                    case OutgoingMessage.ModuleTypes.GPIO:

                        switch (MessageTypeId)
                        {
                            case GPIOModule.CMD_Kill: return "KILL!";
                            case GPIOModule.CMD_ArmNiVek: return "Arm";
                            case GPIOModule.CMD_SafeNiVek: return "Safe";

                            case GPIOModule.CMD_ReadAllPIDConstants: return "Read All PID";
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

                    case OutgoingMessage.ModuleTypes.System:
                        switch (MessageTypeId)
                        {
                            case NiVekSys.Ping: return "Ping";
                            case NiVekSys.WelcomePing: return "Welcome Ping";
                        }
                        break;
                    case OutgoingMessage.ModuleTypes.Sensor:
                        switch(MessageTypeId)
                        {
                            case SensorModule.CMD_SensorZero: return "Zero";
                            case SensorModule.CMD_SensorStart: return "Start";
                            case SensorModule.CMD_SensorStop: return "Stop";
                            case SensorModule.CMD_SensorRestart: return "Restart";
                            case SensorModule.CMD_SensorResetDefaults: return "ResetDefaults";
                            case SensorModule.CMD_BeginSensorDiagnostics: return "BeginSnsrDiag";
                            case SensorModule.CMD_EndSensorDiagnostics: return "EndSnsrDiag";

                            case SensorModule.CMD_BeginMagCalibration: return "BgnMagCal";
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
        public AckStatusTypes AckStatus {
            get { return _ackStatus; }
            set {
                _ackStatus =value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("AckStatus"));

                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("RowColor"));
            }
        }
    
    public event PropertyChangedEventHandler PropertyChanged;
}
}
