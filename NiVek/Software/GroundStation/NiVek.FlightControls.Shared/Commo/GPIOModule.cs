using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class GPIOModule
    {
        public const int CMD_Kill = 60;

        public const int CMD_ArmNiVek = 70;
        public const int CMD_SafeNiVek = 71;

        public const int CMD_ArmESCs = 72;

        public const int CMD_ReadAllPIDConstants = 90;
        public const int CMD_SetPIDConstant = 91;
        public const int CMD_ResetPIDValues = 92;

        public const int CMD_UseRC = 100;
        public const int CMD_UseWiFi = 101;

        public const int CMD_Set_AltitudeHold = 105;
        public const int CMD_Clear_AltitudeHold = 106;

        public const int CMD_ThrottleYawRollThrottle = 110;

        public const int CMD_Set_ESC_Power = 120;

        public const int CMD_Start_RC_CalibrationMode = 130;
        public const int CMD_CalibrateRCChanncel = 131;
        public const int CMD_Cancel_RC_CalibrationMode = 132;
        public const int CMD_End_RC_CalibrationMode = 133;
        
        public const int CMD_StartMotorConfig = 140;
        public const int CMD_EndMotorConfig = 141;

        public const int CMD_StartPIDConfig = 150;
        public const int CMD_EndPIDConfig = 151;
    }
}
