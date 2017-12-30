using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Commo
{
    public class SensorModule
    {
        public const int CMD_SensorZero = 0x81;
        public const int CMD_SensorStart = 0x90;
        public const int CMD_SensorStop = 0x91;
        public const int CMD_SensorRestart = 0x92;
        public const int CMD_SensorResetDefaults = 0x93;
        public const int CMD_BeginSensorDiagnostics = 0x94;
        public const int CMD_EndSensorDiagnostics = 0x95;

        public const int CMD_BeginMagCalibration = 0xA0;
        public const int CMD_CancelMagCalibration = 0xA1;
        public const int CMD_EndMagCalibration = 0xA2;

        public const int CMD_BeginSensorFusionDiag = 0xB0;
        public const int CMD_EndSensorFusionDiag = 0xB1;
        public const int CMD_ReadSensorFusionConfig = 0xB2;
        
        public const int CMD_SetCfg_Value = 0xC0;
        public const int CMD_ReadCfgValues = 0xC1;
        public const int CMD_ResetCfg_Value = 0xC2;

        public const int ITG3200_OFFSET = 0x20;
        public const int ITG3200_DLP_FS = ITG3200_OFFSET + 0x05;
        public const int ITG3200_SMPL_DIVIDER = ITG3200_OFFSET + 0x06;

        public const int LSM303_ACC_OFFSET = 0x40;
        public const int LSM303_ACC_REG1 = LSM303_ACC_OFFSET;
        public const int LSM303_ACC_REG2 = LSM303_ACC_OFFSET + 1;
        public const int LSM303_ACC_REG4 = LSM303_ACC_OFFSET + 2;

        public const int LSM303_MAG_OFFSET = 0x50;
        public const int LSM3030_MAG_CRA = LSM303_MAG_OFFSET + 0x0C;

        public const int L3G_EEPROM_OFFSET = 0x60;

        public const int BMP085_EEPROM_OFFSET = 0x70;

        public const int SENSOR_CFG_EEPROM_OFFSET = 0x80;
        public const int SENSOR_CFG_TIME_CONSTANT = SENSOR_CFG_EEPROM_OFFSET + 1;
        public const int SENSOR_CFG_ACC_LPF = SENSOR_CFG_EEPROM_OFFSET + 2;

    }
}
