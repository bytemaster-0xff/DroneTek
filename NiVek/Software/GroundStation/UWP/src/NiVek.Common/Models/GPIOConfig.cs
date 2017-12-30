using System;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Diagnostics;
using NiVek.Common.Modules;
using System.IO;
using NiVek.Common.Comms;
using Newtonsoft.Json;

namespace NiVek.Common.Models
{
    [ModuleConfig(MessageId = GPIOModule.CMD_SetPIDConstant, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO)]
    public class GPIOConfig : ConfigBase, IModel
    {
        #region Constants as defined from NiVek
        //      const short PID_EEPROM_OFFSET = 0x700; FYI
        const byte K_PITCH_RATE_P_ADDR = 0;
        const byte K_PITCH_RATE_I_ADDR = 2;
        const byte K_PITCH_RATE_D_ADDR = 4;
        const byte K_PITCH_RATE_I_DECAY_FACTOR_ADDR = 6;
        const byte K_PITCH_RATE_I_MAX_ADDR = 8;
        const byte K_PITCH_STEADY_P_ADDR = 10;
        const byte K_PITCH_STEADY_I_ADDR = 12;
        const byte K_PITCH_STEADY_I_DECAY_FACTOR_ADDR = 14;
        const byte K_PITCH_STEADY_I_MAX_ADDR = 16;

        const byte K_ROLL_RATE_P_ADDR = 18;
        const byte K_ROLL_RATE_I_ADDR = 20;
        const byte K_ROLL_RATE_D_ADDR = 22;
        const byte K_ROLL_RATE_I_DECAY_FACTOR_ADDR = 24;
        const byte K_ROLL_RATE_I_MAX_ADDR = 26;
        const byte K_ROLL_STEADY_P_ADDR = 28;
        const byte K_ROLL_STEADY_I_ADDR = 30;
        const byte K_ROLL_STEADY_I_DECAY_FACTOR_ADDR = 32;
        const byte K_ROLL_STEADY_I_MAX_ADDR = 34;

        const byte K_YAW_RATE_P_ADDR = 36;
        const byte K_YAW_RATE_I_ADDR = 38;
        const byte K_YAW_RATE_D_ADDR = 40;
        const byte K_YAW_RATE_I_DECAY_FACTOR_ADDR = 42;
        const byte K_YAW_RATE_I_MAX_ADDR = 44;
        const byte K_YAW_STEADY_P_ADDR = 46;
        const byte K_YAW_STEADY_I_ADDR = 48;
        const byte K_YAW_STEADY_I_DECAY_FACTOR_ADDR = 50;
        const byte K_YAW_STEADY_I_MAX_ADDR = 52;

        const byte K_ALTITUDE_RATE_P_ADDR = 54;
        const byte K_ALTITUDE_RATE_I_ADDR = 56;
        const byte K_ALTITUDE_RATE_D_ADDR = 58;
        const byte K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR = 60;
        const byte K_ALTITUDE_RATE_I_MAX_ADDR = 62;
        const byte K_ALTITUDE_STEADY_P_ADDR = 64;
        const byte K_ALTITUDE_STEADY_I_ADDR = 66;
        const byte K_ALTITUDE_STEADY_I_DECAY_FACTOR_ADDR = 68;
        const byte K_ALTITUDE_STEADY_I_MAX_ADDR = 70;

        const byte K_STARBOARD_FRONT = 72;
        const byte K_STARBOARD_REAR = 74;
        const byte K_PORT_FRONT_ADDR = 76;
        const byte K_PORT_REAR_ADDR = 78;

        const byte K_ESC_UPDATE_RATE = 80;

        const byte K_PITCH_STABLE_BAND = 81;
        const byte K_ROLL_STABLE_BAND = 82;
        const byte K_HEADING_STABLE_BAND = 83;
        const byte K_ALTITUDE_STABLE_BAND = 84;

        const byte PID_SAMPLE_RATE = 85;

        const byte K_THROTTLE_SENSITIVITY = 86;
        const byte K_PITCH_SENSITIVITY = 87;
        const byte K_ROLL_SENSITIVITY = 88;
        const byte K_YAW_SENSITIVITY = 89;

        const byte K_PITCH_FILTER_D = 90;
        const byte K_ROLL_FILTER_D = 91;
        const byte K_YAW_FILTER_D = 92;
        const byte K_ALTITUDE_FILTER_D = 93;
        const byte FRAME_CONFIG = 94;

        const byte K_INITIAL_THROTTLE = 95;
        const byte K_INITIAL_ALTITUDE = 96;

        const byte K_AUTOPILOT_SAMPLE_RATE = 97;

        const byte K_PITCH_FOLLOW_ROLL_PID = 98;
        const byte K_THROTTLE_DEAD_SPACE = 99;
        #endregion

        const float P_STEADY_SCALER = 100.0f;
        const float I_STEADY_SCALER = 100.0f;

        const float P_RATE_SCALER = 10000.0f;
        const float I_RATE_SCALER = 10000.0f;
        const float D_RATE_SCALER = 10000.0f;

        const float I_DECAY_SCALER = 100.0f;
        const float I_MAX_SCALER = 10.0f;

        const float STABILITY_SCALER = 10.0f;
        const float SENSITIVITY_SCALER = 10.0f;

        const float ESC_SCALER = 100.0f;

        const float THROTTLE_SCALING_FACTOR = 2.55f; /* Convert 100% into 255, etc... */

        short getShort(byte[] buffer, int idx)
        {
            return (short)(buffer[idx] | buffer[idx + 1] << 8);
        }

        public GPIOConfig()
        {

        }

        public static GPIOConfig Create(byte[] buffer)
        {
            var cfg = new GPIOConfig();
            cfg.Update(buffer);
            return cfg;
        }

        private void Update(byte[] buffer)
        {
            _rollSteadyP = getShort(buffer, K_ROLL_STEADY_P_ADDR) / P_STEADY_SCALER;    //0
            _rollSteadyI = getShort(buffer, K_ROLL_STEADY_I_ADDR) / I_STEADY_SCALER;    //2
            _rollRateP = getShort(buffer, K_ROLL_RATE_P_ADDR) / P_RATE_SCALER;          // 4
            _rollRateI = getShort(buffer, K_ROLL_RATE_I_ADDR) / I_RATE_SCALER;
            _rollRateD = getShort(buffer, K_ROLL_RATE_D_ADDR) / D_RATE_SCALER;
            _rollRateIDecayFactor = getShort(buffer, K_ROLL_RATE_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _rollRateIMax = getShort(buffer, K_ROLL_RATE_I_MAX_ADDR) / I_MAX_SCALER;
            _rollSteadyIMax = getShort(buffer, K_ROLL_STEADY_I_MAX_ADDR) / I_MAX_SCALER;
            _rollSteadyIDecayFactor = getShort(buffer, K_ROLL_STEADY_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _rollDFilter = buffer[K_ROLL_FILTER_D];

            _pitchSteadyP = getShort(buffer, K_PITCH_STEADY_P_ADDR) / P_STEADY_SCALER;
            _pitchSteadyI = getShort(buffer, K_PITCH_STEADY_I_ADDR) / I_STEADY_SCALER;
            _pitchRateP = getShort(buffer, K_PITCH_RATE_P_ADDR) / P_RATE_SCALER;
            _pitchRateI = getShort(buffer, K_PITCH_RATE_I_ADDR) / I_RATE_SCALER;
            _pitchRateD = getShort(buffer, K_PITCH_RATE_D_ADDR) / D_RATE_SCALER;
            _pitchRateIDecayFactor = getShort(buffer, K_PITCH_RATE_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _pitchRateIMax = getShort(buffer, K_PITCH_RATE_I_MAX_ADDR) / I_MAX_SCALER;
            _pitchSteadyIMax = getShort(buffer, K_PITCH_STEADY_I_MAX_ADDR) / I_MAX_SCALER;
            _pitchSteadyIDecayFactor = getShort(buffer, K_PITCH_STEADY_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _pitchDFilter = buffer[K_PITCH_FILTER_D];

            _yawSteadyP = getShort(buffer, K_YAW_STEADY_P_ADDR) / P_STEADY_SCALER;
            _yawSteadyI = getShort(buffer, K_YAW_STEADY_I_ADDR) / I_STEADY_SCALER;
            _yawRateP = getShort(buffer, K_YAW_RATE_P_ADDR) / P_RATE_SCALER;
            _yawRateI = getShort(buffer, K_YAW_RATE_I_ADDR) / I_RATE_SCALER;
            _yawRateD = getShort(buffer, K_YAW_RATE_D_ADDR) / D_RATE_SCALER;
            _yawRateIDecayFactor = getShort(buffer, K_YAW_RATE_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _yawRateIMax = getShort(buffer, K_YAW_RATE_I_MAX_ADDR) / I_MAX_SCALER;
            _yawSteadyIMax = getShort(buffer, K_YAW_STEADY_I_MAX_ADDR) / I_MAX_SCALER;
            _yawDFilter = buffer[K_YAW_FILTER_D];

            _altitudeSteadyP = getShort(buffer, K_ALTITUDE_STEADY_P_ADDR) / P_STEADY_SCALER;
            _altitudeSteadyI = getShort(buffer, K_ALTITUDE_STEADY_I_ADDR) / I_STEADY_SCALER;
            _altitudeRateP = getShort(buffer, K_ALTITUDE_RATE_P_ADDR) / P_RATE_SCALER;
            _altitudeRateI = getShort(buffer, K_ALTITUDE_RATE_I_ADDR) / I_RATE_SCALER;
            _altitudeRateD = getShort(buffer, K_ALTITUDE_RATE_D_ADDR) / D_RATE_SCALER;
            _altitudeRateIDecayFactor = getShort(buffer, K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _altitudeRateIMax = getShort(buffer, K_ALTITUDE_RATE_I_MAX_ADDR) / I_MAX_SCALER;
            _altitudeSteadyIMax = getShort(buffer, K_ALTITUDE_STEADY_I_MAX_ADDR) / I_MAX_SCALER;
            _altitudeDFilter = buffer[K_ALTITUDE_FILTER_D];

            _escStarboardFront = getShort(buffer, K_STARBOARD_FRONT) / ESC_SCALER;
            _escStarboardRear = getShort(buffer, K_STARBOARD_REAR) / ESC_SCALER;
            _escPortFront = getShort(buffer, K_PORT_FRONT_ADDR) / ESC_SCALER;
            _escPortRear = getShort(buffer, K_PORT_REAR_ADDR) / ESC_SCALER;

            _escUpateRate = buffer[K_ESC_UPDATE_RATE];
            _pidSampleRate = buffer[PID_SAMPLE_RATE];

            _frameConfig = buffer[FRAME_CONFIG];

            _initialAltutide = buffer[K_INITIAL_ALTITUDE];
            _initialThrottle = Math.Round(buffer[K_INITIAL_THROTTLE] / THROTTLE_SCALING_FACTOR, 1);
            _autoPilotSampleRate = buffer[K_AUTOPILOT_SAMPLE_RATE];

            _pitchSteadyBand = (byte)buffer[K_PITCH_STABLE_BAND] / STABILITY_SCALER;
            _rollSteadyBand = (byte)buffer[K_ROLL_STABLE_BAND] / STABILITY_SCALER;
            _headingSteadyBand = (byte)buffer[K_HEADING_STABLE_BAND] / STABILITY_SCALER;
            _altitudeSteadyBand = (byte)buffer[K_ALTITUDE_STABLE_BAND] / STABILITY_SCALER;

            _throttleSensitivity = (sbyte)buffer[K_THROTTLE_SENSITIVITY] / 100.0f;
            _pitchSensitivity = (sbyte)buffer[K_PITCH_SENSITIVITY] / SENSITIVITY_SCALER;
            _rollSensitivity = (sbyte)buffer[K_ROLL_SENSITIVITY] / SENSITIVITY_SCALER;
            _yawSensitivity = (sbyte)buffer[K_YAW_SENSITIVITY] / SENSITIVITY_SCALER;

            _pitchFollowsRoll = (byte)buffer[K_PITCH_FOLLOW_ROLL_PID];

            _throttleDeadSpace = (byte)buffer[K_THROTTLE_DEAD_SPACE];

            if ((byte)buffer[K_PITCH_FOLLOW_ROLL_PID] != 255)
                Debug.WriteLine("PFR IS NOT CORRECT!");

            if ((byte)buffer[K_INITIAL_ALTITUDE] != 210)
                Debug.WriteLine("INIT ALT NOT CORRECT");

            _isReady = true;
        }

        public void Update(BufferReader rdr)
        {
            var buffer = rdr.GetRawBuffer();
            Update(buffer);
        }

        #region Deserialize from NiVek
        public GPIOConfig(Drone drone, byte[] buffer)
        {
            this.Drone = drone;
            Update(buffer);
        }
        #endregion


        #region Steady Band Parameters
        double _pitchSteadyBand;
        [DeviceSetting(SettingAddress = K_PITCH_STABLE_BAND, ScalingFactor = STABILITY_SCALER)]
        public double PitchSteadyBand
        {
            get { return _pitchSteadyBand; }
            set { Set(value, ref _pitchSteadyBand, () => PitchSteadyBand); }
        }

        double _rollSteadyBand;
        [DeviceSetting(SettingAddress = K_ROLL_STABLE_BAND, ScalingFactor = STABILITY_SCALER)]
        public double RollSteadyBand
        {
            get { return _rollSteadyBand; }
            set { Set(value, ref _rollSteadyBand, () => RollSteadyBand); }
        }

        double _headingSteadyBand;
        [DeviceSetting(SettingAddress = K_HEADING_STABLE_BAND, ScalingFactor = STABILITY_SCALER)]
        public double HeadingSteadyBand
        {
            get { return _headingSteadyBand; }
            set { Set(value, ref _headingSteadyBand, () => HeadingSteadyBand); }
        }

        double _altitudeSteadyBand;
        [DeviceSetting(SettingAddress = K_ALTITUDE_STABLE_BAND, ScalingFactor = STABILITY_SCALER)]
        public double AltitudeSteadyBand
        {
            get { return _altitudeSteadyBand; }
            set { Set(value, ref _altitudeSteadyBand, () => AltitudeSteadyBand); }
        }
        #endregion

        #region Gain Parameters
        double _throttleSensitivity;
        [DeviceSetting(SettingAddress = K_THROTTLE_SENSITIVITY, ScalingFactor = 100.0f)]
        public double ThrottleSensitivity
        {
            get { return _throttleSensitivity; }
            set { Set(value, ref _throttleSensitivity, () => ThrottleSensitivity); }
        }


        double _pitchSensitivity;
        [DeviceSetting(SettingAddress = K_PITCH_SENSITIVITY, ScalingFactor = SENSITIVITY_SCALER)]
        public double PitchSensitivity
        {
            get { return _pitchSensitivity; }
            set { Set(value, ref _pitchSensitivity, () => PitchSensitivity); }
        }

        double _rollSensitivity;
        [DeviceSetting(SettingAddress = K_ROLL_SENSITIVITY, ScalingFactor = SENSITIVITY_SCALER)]
        public double RollSensitivity
        {
            get { return _rollSensitivity; }
            set { Set(value, ref _rollSensitivity, () => RollSensitivity); }
        }

        double _yawSensitivity;
        [DeviceSetting(SettingAddress = K_YAW_SENSITIVITY, ScalingFactor = SENSITIVITY_SCALER)]
        public double YawSensitivity
        {
            get { return _yawSensitivity; }
            set { Set(value, ref _yawSensitivity, () => YawSensitivity); }
        }
        #endregion

        #region Pitch PID Settings
        double _pitchSteadyP;
        [DeviceSetting(SettingAddress = K_PITCH_STEADY_P_ADDR, ScalingFactor = P_STEADY_SCALER)]
        public double PitchSteadyP
        {
            get { return _pitchSteadyP; }
            set { Set(value, ref _pitchSteadyP, () => PitchSteadyP); }
        }

        double _pitchSteadyI;
        [DeviceSetting(SettingAddress = K_PITCH_STEADY_I_ADDR, ScalingFactor = I_STEADY_SCALER)]
        public double PitchSteadyI
        {
            get { return _pitchSteadyI; }
            set { Set(value, ref _pitchSteadyI, () => PitchSteadyI); }
        }

        double _pitchSteadyIDecayFactor;
        [DeviceSetting(SettingAddress = K_PITCH_STEADY_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]

        public double PitchSteadyIDecayFactor
        {
            get { return _pitchSteadyIDecayFactor; }
            set { Set(value, ref _pitchSteadyIDecayFactor, () => PitchSteadyIDecayFactor); }
        }

        double _pitchRateP;
        [DeviceSetting(SettingAddress = K_PITCH_RATE_P_ADDR, ScalingFactor = P_RATE_SCALER)]
        public double PitchRateP
        {
            get { return _pitchRateP; }
            set { Set(value, ref _pitchRateP, () => PitchRateP); }
        }

        double _pitchRateI;
        [DeviceSetting(SettingAddress = K_PITCH_RATE_I_ADDR, ScalingFactor = I_RATE_SCALER)]
        public double PitchRateI
        {
            get { return _pitchRateI; }
            set { Set(value, ref _pitchRateI, () => PitchRateI); }
        }

        double _pitchRateD;
        [DeviceSetting(SettingAddress = K_PITCH_RATE_D_ADDR, ScalingFactor = D_RATE_SCALER)]
        public double PitchRateD
        {
            get { return _pitchRateD; }
            set { Set(value, ref _pitchRateD, () => PitchRateD); }
        }

        double _pitchRateIDecayFactor;
        [DeviceSetting(SettingAddress = K_PITCH_RATE_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double PitchRateIDecayFactor
        {
            get { return _pitchRateIDecayFactor; }
            set { Set(value, ref _pitchRateIDecayFactor, () => PitchRateIDecayFactor); }
        }

        double _pitchRateIMax;
        [DeviceSetting(SettingAddress = K_PITCH_RATE_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double PitchRateIMax
        {
            get { return _pitchRateIMax; }
            set { Set(value, ref _pitchRateIMax, () => PitchRateIMax); }
        }

        double _pitchSteadyIMax;
        [DeviceSetting(SettingAddress = K_PITCH_STEADY_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double PitchSteadyIMax
        {
            get { return _pitchSteadyIMax; }
            set { Set(value, ref _pitchSteadyIMax, () => PitchSteadyIMax); }
        }

        byte _pitchDFilter;
        [DeviceSetting(SettingAddress = K_PITCH_FILTER_D)]
        public byte PitchDFilter
        {
            get { return _pitchDFilter; }
            set { Set(value, ref _pitchDFilter, () => PitchDFilter); }
        }
        #endregion

        #region Roll PID Settings
        double _rollSteadyP;
        [DeviceSetting(SettingAddress = K_ROLL_STEADY_P_ADDR, ScalingFactor = P_STEADY_SCALER)]
        public double RollSteadyP
        {
            get { return _rollSteadyP; }
            set { Set(value, ref _rollSteadyP, () => RollSteadyP); }
        }

        double _rollSteadyI;
        [DeviceSetting(SettingAddress = K_ROLL_STEADY_I_ADDR, ScalingFactor = I_STEADY_SCALER)]

        public double RollSteadyI
        {
            get { return _rollSteadyI; }
            set { Set(value, ref _rollSteadyI, () => RollSteadyI); }
        }

        double _rollSteadyIDecayFactor;
        [DeviceSetting(SettingAddress = K_ROLL_STEADY_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double RollSteadyIDecayFactor
        {
            get { return _rollSteadyIDecayFactor; }
            set { Set(value, ref _rollSteadyIDecayFactor, () => RollSteadyIDecayFactor); }
        }

        double _rollRateP;
        [DeviceSetting(SettingAddress = K_ROLL_RATE_P_ADDR, ScalingFactor = P_RATE_SCALER)]
        public double RollRateP
        {
            get { return _rollRateP; }
            set { Set(value, ref _rollRateP, () => RollRateP); }
        }

        double _rollRateI;
        [DeviceSetting(SettingAddress = K_ROLL_RATE_I_ADDR, ScalingFactor = I_RATE_SCALER)]
        public double RollRateI
        {
            get { return _rollRateI; }
            set { Set(value, ref _rollRateI, () => RollRateI); }
        }

        double _rollRateD;
        [DeviceSetting(SettingAddress = K_ROLL_RATE_D_ADDR, ScalingFactor = D_RATE_SCALER)]
        public double RollRateD
        {
            get { return _rollRateD; }
            set { Set(value, ref _rollRateD, () => RollRateD); }
        }

        double _rollRateIDecayFactor;
        [DeviceSetting(SettingAddress = K_ROLL_RATE_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double RollRateIDecayFactor
        {
            get { return _rollRateIDecayFactor; }
            set { Set(value, ref _rollRateIDecayFactor, () => RollRateIDecayFactor); }
        }

        double _rollRateIMax;
        [DeviceSetting(SettingAddress = K_ROLL_RATE_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double RollRateIMax
        {
            get { return _rollRateIMax; }
            set { Set(value, ref _rollRateIMax, () => RollRateIMax); }
        }

        double _rollSteadyIMax;
        [DeviceSetting(SettingAddress = K_ROLL_STEADY_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double RollSteadyIMax
        {
            get { return _rollSteadyIMax; }
            set { Set(value, ref _rollSteadyIMax, () => RollSteadyIMax); }
        }

        byte _rollDFilter;
        [DeviceSetting(SettingAddress = K_ROLL_FILTER_D)]
        public byte RollDFilter
        {
            get { return _rollDFilter; }
            set { Set(value, ref _rollDFilter, () => RollDFilter); }
        }
        #endregion

        #region Yaw PID Settings
        double _yawSteadyP;
        [DeviceSetting(SettingAddress = K_YAW_STEADY_P_ADDR, ScalingFactor = P_STEADY_SCALER)]
        public double YawSteadyP
        {
            get { return _yawSteadyP; }
            set { Set(value, ref _yawSteadyP, () => YawSteadyP); }
        }


        double _yawSteadyI;
        [DeviceSetting(SettingAddress = K_YAW_STEADY_I_ADDR, ScalingFactor = I_STEADY_SCALER)]
        public double YawSteadyI
        {
            get { return _yawSteadyI; }
            set { Set(value, ref _yawSteadyI, () => YawSteadyI); }
        }

        double _yawSteadyIDecayFactor;
        [DeviceSetting(SettingAddress = K_YAW_STEADY_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double YawSteadyIDecayFactor
        {
            get { return _yawSteadyIDecayFactor; }
            set { Set(value, ref _yawSteadyIDecayFactor, () => YawSteadyIDecayFactor); }
        }


        double _yawRateP;
        [DeviceSetting(SettingAddress = K_YAW_RATE_P_ADDR, ScalingFactor = P_RATE_SCALER)]
        public double YawRateP
        {
            get { return _yawRateP; }
            set { Set(value, ref _yawRateP, () => YawRateP); }
        }


        double _yawRateI;
        [DeviceSetting(SettingAddress = K_YAW_RATE_I_ADDR, ScalingFactor = I_RATE_SCALER)]
        public double YawRateI
        {
            get { return _yawRateI; }
            set { Set(value, ref _yawRateI, () => YawRateI); }
        }

        double _yawRateD;
        [DeviceSetting(SettingAddress = K_YAW_RATE_D_ADDR, ScalingFactor = D_RATE_SCALER)]
        public double YawRateD
        {
            get { return _yawRateD; }
            set { Set(value, ref _yawRateD, () => YawRateD); }
        }


        double _yawRateIDecayFactor;
        [DeviceSetting(SettingAddress = K_YAW_RATE_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double YawRateIDecayFactor
        {
            get { return _yawRateIDecayFactor; }
            set { Set(value, ref _yawRateIDecayFactor, () => YawRateIDecayFactor); }
        }

        double _yawRateIMax;
        [DeviceSetting(SettingAddress = K_YAW_RATE_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double YawRateIMax
        {
            get { return _yawRateIMax; }
            set { Set(value, ref _yawRateIMax, () => YawRateIMax); }
        }

        double _yawSteadyIMax;
        [DeviceSetting(SettingAddress = K_YAW_STEADY_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double YawSteadyIMax
        {
            get { return _yawSteadyIMax; }
            set { Set(value, ref _yawSteadyIMax, () => YawSteadyIMax); }
        }

        byte _yawDFilter;
        [DeviceSetting(SettingAddress = K_YAW_FILTER_D)]
        public byte YawDFilter
        {
            get { return _yawDFilter; }
            set { Set(value, ref _yawDFilter, () => YawDFilter); }
        }
        #endregion

        #region Altitude PID Parameters
        double _altitudeSteadyP;
        [DeviceSetting(SettingAddress = K_ALTITUDE_STEADY_P_ADDR, ScalingFactor = P_STEADY_SCALER)]
        public double AltitudeSteadyP
        {
            get { return _altitudeSteadyP; }
            set { Set(value, ref _altitudeSteadyP, () => AltitudeSteadyP); }
        }


        double _altitudeSteadyI;
        [DeviceSetting(SettingAddress = K_ALTITUDE_STEADY_I_ADDR, ScalingFactor = I_STEADY_SCALER)]
        public double AltitudeSteadyI
        {
            get { return _altitudeSteadyI; }
            set { Set(value, ref _altitudeSteadyI, () => AltitudeSteadyI); }
        }

        double _altitudeSteadyIDecayFactor;
        [DeviceSetting(SettingAddress = K_ALTITUDE_STEADY_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double AltitudeSteadyIDecayFactor
        {
            get { return _altitudeSteadyIDecayFactor; }
            set { Set(value, ref _altitudeSteadyIDecayFactor, () => AltitudeSteadyIDecayFactor); }
        }


        double _altitudeRateP;
        [DeviceSetting(SettingAddress = K_ALTITUDE_RATE_P_ADDR, ScalingFactor = P_RATE_SCALER)]
        public double AltitudeRateP
        {
            get { return _altitudeRateP; }
            set { Set(value, ref _altitudeRateP, () => AltitudeRateP); }
        }


        double _altitudeRateI;
        [DeviceSetting(SettingAddress = K_ALTITUDE_RATE_I_ADDR, ScalingFactor = I_RATE_SCALER)]
        public double AltitudeRateI
        {
            get { return _altitudeRateI; }
            set { Set(value, ref _altitudeRateI, () => AltitudeRateI); }
        }

        double _altitudeRateD;
        [DeviceSetting(SettingAddress = K_ALTITUDE_RATE_D_ADDR, ScalingFactor = D_RATE_SCALER)]
        public double AltitudeRateD
        {
            get { return _altitudeRateD; }
            set { Set(value, ref _altitudeRateD, () => AltitudeRateD); }
        }

        double _altitudeRateIDecayFactor;
        [DeviceSetting(SettingAddress = K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR, ScalingFactor = I_DECAY_SCALER)]
        public double AltitudeRateIDecayFactor
        {
            get { return _altitudeRateIDecayFactor; }
            set { Set(value, ref _altitudeRateIDecayFactor, () => AltitudeRateIDecayFactor); }
        }

        double _altitudeRateIMax;
        [DeviceSetting(SettingAddress = K_ALTITUDE_RATE_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double AltitudeRateIMax
        {
            get { return _altitudeRateIMax; }
            set { Set(value, ref _altitudeRateIMax, () => AltitudeRateIMax); }
        }

        double _altitudeSteadyIMax;
        [DeviceSetting(SettingAddress = K_ALTITUDE_STEADY_I_MAX_ADDR, ScalingFactor = I_MAX_SCALER)]
        public double AltitudeSteadyIMax
        {
            get { return _altitudeSteadyIMax; }
            set { Set(value, ref _altitudeSteadyIMax, () => AltitudeSteadyIMax); }
        }

        byte _altitudeDFilter;
        [DeviceSetting(SettingAddress = K_ALTITUDE_FILTER_D)]
        public byte AltitudeDFilter
        {
            get { return _altitudeDFilter; }
            set { Set(value, ref _altitudeDFilter, () => AltitudeDFilter); }
        }
        #endregion

        #region ESC Gain Parameters
        double _escStarboardFront;
        [DeviceSetting(SettingAddress = K_STARBOARD_FRONT, ScalingFactor = ESC_SCALER)]

        public double EscStarboardFront
        {
            get { return _escStarboardFront; }
            set { Set(value, ref _escStarboardFront, () => EscStarboardFront); }
        }

        double _escStarboardRear;
        [DeviceSetting(SettingAddress = K_STARBOARD_REAR, ScalingFactor = ESC_SCALER)]
        public double EscStarboardRear
        {
            get { return _escStarboardRear; }
            set { Set(value, ref _escStarboardRear, () => EscStarboardRear); }
        }

        double _escPortFront;
        [DeviceSetting(SettingAddress = K_PORT_FRONT_ADDR, ScalingFactor = ESC_SCALER)]
        public double EscPortFront
        {
            get { return _escPortFront; }
            set { Set(value, ref _escPortFront, () => EscPortFront); }
        }

        double _escPortRear;
        [DeviceSetting(SettingAddress = K_PORT_REAR_ADDR, ScalingFactor = ESC_SCALER)]
        public double EscPortRear
        {
            get { return _escPortRear; }
            set { Set(value, ref _escPortRear, () => EscPortRear); }
        }
        #endregion

        #region PID Sample Rate/ESC Sample Rate/Auto Pilot Sample Rate
        byte _pidSampleRate;
        [DeviceSetting(SettingAddress = PID_SAMPLE_RATE)]
        public byte PIDSampleRate
        {
            get { return _pidSampleRate; }
            set { Set(value, ref _pidSampleRate, () => PIDSampleRate); }
        }

        byte _autoPilotSampleRate;
        [DeviceSetting(SettingAddress = K_AUTOPILOT_SAMPLE_RATE)]
        public byte AutoPilotSampleRate
        {
            get { return _autoPilotSampleRate; }
            set { Set(value, ref _autoPilotSampleRate, () => AutoPilotSampleRate); }
        }


        byte _escUpateRate;
        [DeviceSetting(SettingAddress = K_ESC_UPDATE_RATE)]
        public byte ESCUpdateRate
        {
            get { return _escUpateRate; }
            set { Set(value, ref _escUpateRate, () => ESCUpdateRate); }
        }
        #endregion

        #region Frame Type
        byte _frameConfig;
        [DeviceSetting(SettingAddress = FRAME_CONFIG)]
        public byte FrameConfig
        {
            get { return _frameConfig; }
            set
            {
                if ((value == (byte)GPIOConfig.FrameConfigTypes.X) ||
                    (value == (byte)GPIOConfig.FrameConfigTypes.Cross)) 
                    Set(value, ref _frameConfig, () => FrameConfig);
            }
        }
        #endregion

        #region Initial Altitude/Throttle
        double _initialAltutide = 0x00;
        [DeviceSetting(SettingAddress = K_INITIAL_ALTITUDE)]
        public double InitialAltitude
        {
            get { return _initialAltutide; }
            set { Set(value, ref _initialAltutide, () => InitialAltitude); }
        }

        double _initialThrottle = 0.0;
        [DeviceSetting(SettingAddress = K_INITIAL_THROTTLE, ScalingFactor = THROTTLE_SCALING_FACTOR)]
        public double InitialThrottle
        {
            get { return _initialThrottle; }
            set { Set(value, ref _initialThrottle, () => InitialThrottle); }
        }
        #endregion

        #region Misc Parameters -> Pitch Follows Roll
        byte _pitchFollowsRoll = 0;
        [DeviceSetting(SettingAddress = K_PITCH_FOLLOW_ROLL_PID)]
        public byte PitchFollowsRoll
        {
            get { return _pitchFollowsRoll; }
            set { Set(value, ref _pitchFollowsRoll, () => PitchFollowsRoll); }
        }

        byte _throttleDeadSpace = 0;
        [DeviceSetting(SettingAddress = K_THROTTLE_DEAD_SPACE)]
        public byte ThrottleDeadSpace
        {
            get { return _throttleDeadSpace; }
            set { Set(value, ref _throttleDeadSpace, () => ThrottleDeadSpace); }
        }
        #endregion

        #region Motor Power
        byte _motorPower = 0;
        public byte MotorPower
        {
            get { return _motorPower; }
            set
            {
                if (_motorPower != value)
                {
                    _motorPower = value;
                    SendMotorPower(_motorPower);
                    OnPropertyChanged(() => MotorPower);
                }
            }
        }

        byte _motor = 0xFF;
        public byte Motor
        {
            get { return _motor; }
            set
            {
                if (_motor != value)
                {
                    _motor = value;
                    SendMotorPower(_motorPower);
                    OnPropertyChanged(() => Motor);
                }
            }
        }

        public async void SendMotorPower(byte power)
        {
            Debug.WriteLine("Sending motor power {0} to {1}", power, _motor);
            Drone.ThrottleInput = power;
            OnPropertyChanged(() => Motor);

            var msg = new OutgoingMessage() { MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte(_motor);
            msg.AddByte(power);
            await Drone.SendMessageAsync(msg);
        }

        public enum ESCUpdateRates
        {
            FiftyHz = 1,
            HundredHz = 2,
            OneFiftyHz = 3,
            TwoHundredHz = 4,
            TwoFiftyHz = 5,
            ThreeHundredHz = 6,
            ThreeFiftyHz = 7,
            FourHundredHz = 8
        }
        #endregion

        #region Lists used to bind combo boxes
        public enum FrameConfigTypes
        {
            Cross = 10,
            X = 20
        }

        ObservableCollection<KeyDisplay> _pitchFollowRollCollection = new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "Locked", Key =  255 },
             new KeyDisplay{Display = "Unlocked", Key =  0},
          };

        public ObservableCollection<KeyDisplay> PitchFollowsRollCollection { get { return _pitchFollowRollCollection; } }


        ObservableCollection<KeyDisplay> _dFilter = new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "None", Key =  0 },
             new KeyDisplay{Display = "5Hz", Key =  1 },
             new KeyDisplay{Display = "10Hz", Key =  2 },
             new KeyDisplay{Display = "15Hz", Key =  3},
             new KeyDisplay{Display = "20Hz", Key =  4},
             new KeyDisplay{Display = "25Hz", Key =  5},
             new KeyDisplay{Display = "30Hz", Key =  6},
             new KeyDisplay{Display = "35Hz", Key =  7},
             new KeyDisplay{Display = "40Hz", Key =  8},
        };

        public ObservableCollection<KeyDisplay> DFilterCollection { get { return _dFilter; } }

        ObservableCollection<KeyDisplay> _frameConfigCollection = new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "+", Key =  (byte)GPIOConfig.FrameConfigTypes.Cross },
             new KeyDisplay{Display = "X", Key =  (byte)GPIOConfig.FrameConfigTypes.X },
        };

        public ObservableCollection<KeyDisplay> FrameConfigCollection { get { return _frameConfigCollection; } }


        ObservableCollection<KeyDisplay> _escUpdateRateCollection = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "400Hz", Key =  8 },
                new KeyDisplay{Display = "350Hz", Key =  7 },
                new KeyDisplay{Display = "300Hz", Key =  6},
                new KeyDisplay{Display = "250Hz", Key =  5},
                new KeyDisplay{Display = "200Hz", Key =  4},
                new KeyDisplay{Display = "150Hz", Key =  3},
                new KeyDisplay{Display = "100Hz", Key =  2},
                new KeyDisplay{Display = "50Hz", Key = 1 }
            };
        public ObservableCollection<KeyDisplay> ESCUpdateRateCollection { get { return _escUpdateRateCollection; } }

        ObservableCollection<KeyDisplay> _motorPowerCollection = new ObservableCollection<KeyDisplay>()
            {
                new KeyDisplay{Display = "Off", Key = 0x00 },
                new KeyDisplay{Display = "5%", Key = (byte)(0xFF * 0.05f) },
                new KeyDisplay{Display = "10%", Key = (byte)(0xFF * 0.1f) },
                new KeyDisplay{Display = "15%", Key = (byte)(0xFF * 0.15f) },
                new KeyDisplay{Display = "20%", Key = (byte)(0xFF * 0.2f) },
                new KeyDisplay{Display = "23%", Key = (byte)(0xFF * 0.23f) },
                new KeyDisplay{Display = "26%", Key = (byte)(0xFF * 0.26f) },
                new KeyDisplay{Display = "30%", Key = (byte)(0xFF * 0.3f) },
                new KeyDisplay{Display = "40%", Key = (byte)(0xFF * 0.4f) },
                new KeyDisplay{Display = "50%", Key = (byte)(0xFF * 0.5f) },
                new KeyDisplay{Display = "60%", Key = (byte)(0xFF * 0.6f) },
                new KeyDisplay{Display = "70%", Key = (byte)(0xFF * 0.7f) },
                new KeyDisplay{Display = "80%", Key = (byte)(0xFF * 0.8f) },
                new KeyDisplay{Display = "90%", Key = (byte)(0xFF * 0.9f) },
                new KeyDisplay{Display = "100%", Key = 0xFF }                
            };

        public ObservableCollection<KeyDisplay> MotorPowerCollection { get { return _motorPowerCollection; } }

        ObservableCollection<KeyDisplay> _motorCollectionX = new ObservableCollection<KeyDisplay>(){
                new KeyDisplay{Display = "All", Key = 0xFF },
                new KeyDisplay{Display = "Port Front", Key = 0x01 },
                new KeyDisplay{Display = "Port Rear", Key = 0x02 },
                new KeyDisplay{Display = "Starboard Front", Key = 0x03 },
                new KeyDisplay{Display = "Starboard Rear", Key = 0x04 },                
            };
        public ObservableCollection<KeyDisplay> MotorCollectionX { get { return _motorCollectionX; } }

        ObservableCollection<KeyDisplay> _motorCollectionCross = new ObservableCollection<KeyDisplay>(){
                new KeyDisplay{Display = "All", Key = 0xFF },
                new KeyDisplay{Display = "Front", Key = 0x01 },
                new KeyDisplay{Display = "Port", Key = 0x02 },
                new KeyDisplay{Display = "Starboard", Key = 0x03 },
                new KeyDisplay{Display = "Rear", Key = 0x04 },                
            };
        public ObservableCollection<KeyDisplay> MotorCollectionCross { get { return _motorCollectionCross; } }
        #endregion

        public String GetJSON()
        {
            return JsonConvert.SerializeObject(this);
        }

        public void CreateFromJSON()
        {

        }
    }

    public class ReceivedPIDEventArgs
    {
        public GPIOConfig PIDValue { get; set; }
    }
}
