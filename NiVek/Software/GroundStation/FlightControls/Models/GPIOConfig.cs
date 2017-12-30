using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NiVek.FlightControls.Commo;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Diagnostics;

namespace NiVek.FlightControls.Models
{
    public class GPIOConfig : INotifyPropertyChanged
    {
  //      const short PID_EEPROM_OFFSET = 0x700; FYI

        const short K_PITCH_P_ADDR = 0;
        const short K_PITCH_I_ADDR = 2;
        const short K_PITCH_D_ADDR = 4;
        const short K_PITCH_I_DECAY_FACTOR_ADDR = 6;
        const short K_PITCH_I_MAX_ADDR = 8;

        const short K_ROLL_P_ADDR = 10;
        const short K_ROLL_I_ADDR = 12;
        const short K_ROLL_D_ADDR = 14;
        const short K_ROLL_I_DECAY_FACTOR_ADDR = 16;
        const short K_ROLL_I_MAX_ADDR = 18;

        const short K_YAW_P_ADDR = 20;
        const short K_YAW_I_ADDR = 22;
        const short K_YAW_D_ADDR = 24;
        const short K_YAW_I_DECAY_FACTOR_ADDR = 26;
        const short K_YAW_I_MAX_ADDR = 28;

        const short K_ALTITUDE_P_ADDR = 30;
        const short K_ALTITUDE_I_ADDR = 32;
        const short K_ALTITUDE_D_ADDR = 34;
        const short K_ALTITUDE_I_DECAY_FACTOR_ADDR = 36;
        const short K_ALTITUDE_I_MAX_ADDR = 38;

        const short K_STARBOARD_FRONT = 40;
        const short K_STARBOARD_REAR = 42;
        const short K_PORT_FRONT_ADDR = 44;
        const short K_PORT_REAR_ADDR = 46;

        const short K_ESC_UPDATE_RATE = 48;

        const short K_PITCH_STABLE_BAND = 49;
        const short K_ROLL_STABLE_BAND = 50;
        const short K_HEADING_STABLE_BAND = 51;
        const short K_ALTITUDE_STABLE_BAND = 52;
        
        const short PID_SAMPLE_RATE = 53;

        const short K_PITCH_SENSITIVITY = 54;
        const short K_ROLL_SENSITIVITY = 55;
        const short K_YAW_SENSITIVITY = 56;

        const short K_PITCH_FILTER_D = 57;
        const short K_ROLL_FILTER_D = 58;
        const short K_YAW_FILTER_D = 59;
        const short K_ALTITUDE_FILTER_D = 60;
        const short FRAME_CONFIG = 61;

        const float P_SCALER = 10000.0f;
        const float I_SCALER = 10000.0f;
        const float D_SCALER = 10000.0f;

        const float P_DISPLAY_SCALER = 100.0f;
        const float I_DISPLAY_SCALER = 100.0f;
        const float D_DISPLAY_SCALER = 100.0f;

        const float I_DECAY_SCALER = 100.0f;
        const float I_MAX_SCALER = 10.0f;
        const float STABILITY_SCALER = 10.0f;
        const float SENSITIVITY_SCALER = 10.0f;

        const float ESC_SCALER = 100.0f;

        public enum FrameConfigTypes
        {
            Cross,
            X
        }

        short getShort(byte[] buffer, int idx)
        {
            return (short)(buffer[idx] | buffer[idx + 1] << 8);
        }

        public GPIOConfig(byte[] buffer)
        {
            _rollP =  getShort(buffer, K_ROLL_P_ADDR) / P_SCALER;
            _rollI = getShort(buffer, K_ROLL_I_ADDR) / I_SCALER;
            _rollD = getShort(buffer, K_ROLL_D_ADDR) / D_SCALER;
            _rollDecayFactor = getShort(buffer, K_ROLL_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _rollIMax = getShort(buffer, K_ROLL_I_MAX_ADDR) / I_MAX_SCALER;
            _rollDFilter = buffer[K_ROLL_FILTER_D];

            _pitchP = getShort(buffer, K_PITCH_P_ADDR) / P_SCALER;
            _pitchI = getShort(buffer, K_PITCH_I_ADDR) / I_SCALER;
            _pitchD = getShort(buffer, K_PITCH_D_ADDR) / D_SCALER;
            _pitchDecayFactor = getShort(buffer, K_PITCH_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _pitchIMax = getShort(buffer, K_PITCH_I_MAX_ADDR) / I_MAX_SCALER;
            _pitchDFilter = buffer[K_PITCH_FILTER_D];

            _yawP = getShort(buffer, K_YAW_P_ADDR) / P_SCALER;
            _yawI = getShort(buffer, K_YAW_I_ADDR) / I_SCALER;
            _yawD = getShort(buffer, K_YAW_D_ADDR) / D_SCALER;
            _yawDecayFactor = getShort(buffer, K_YAW_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _yawIMax = getShort(buffer, K_YAW_I_MAX_ADDR) / I_MAX_SCALER;
            _yawDFilter = buffer[K_YAW_FILTER_D];

            _altitudeP = getShort(buffer, K_ALTITUDE_P_ADDR) / P_SCALER;
            _altitudeI = getShort(buffer, K_ALTITUDE_I_ADDR) / I_SCALER;
            _altitudeD = getShort(buffer, K_ALTITUDE_D_ADDR) / D_SCALER;
            _altitudeDecayFactor = getShort(buffer, K_ALTITUDE_I_DECAY_FACTOR_ADDR) / I_DECAY_SCALER;
            _altitudeIMax = getShort(buffer, K_ALTITUDE_I_MAX_ADDR) / I_MAX_SCALER;
            _altitudeDFilter = buffer[K_ALTITUDE_FILTER_D];

            _escStarboardFront = getShort(buffer, K_STARBOARD_FRONT) / ESC_SCALER;
            _escStarboardRear = getShort(buffer, K_STARBOARD_REAR) / ESC_SCALER;
            _escPortFront = getShort(buffer, K_PORT_FRONT_ADDR) / ESC_SCALER;
            _escPortRear = getShort(buffer, K_PORT_REAR_ADDR) / ESC_SCALER;

            _escUpateRate = buffer[K_ESC_UPDATE_RATE];
            _pidSampleRate = buffer[PID_SAMPLE_RATE];

            _frameConfig = buffer[FRAME_CONFIG];

            _pitchSteadyBand = (byte)buffer[K_PITCH_STABLE_BAND] / STABILITY_SCALER;
            _rollSteadyBand = (byte)buffer[K_ROLL_STABLE_BAND] / STABILITY_SCALER;
            _headingSteadyBand = (byte)buffer[K_HEADING_STABLE_BAND] / STABILITY_SCALER;
            _altitudeSteadyBand = (byte)buffer[K_ALTITUDE_STABLE_BAND] / STABILITY_SCALER;

            _pitchSensitivity = (sbyte)buffer[K_PITCH_SENSITIVITY] / SENSITIVITY_SCALER;
            _rollSensitivity = (sbyte)buffer[K_ROLL_SENSITIVITY] / SENSITIVITY_SCALER;
            _yawSensitivity = (sbyte)buffer[K_YAW_SENSITIVITY] / SENSITIVITY_SCALER;
        }

        void SendCalibrationFactor(byte address, short value)
        {
            var msg = new OutgoingMessage() { MessageId = GPIOModule.CMD_SetPIDConstant, ModuleType = OutgoingMessage.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte(address);
            msg.Add(value);
            App.Commo.Send(msg);
        }

        byte _pidSampleRate;
        public byte PIDSampleRate
        {
            get { return _pidSampleRate; }
            set
            {
                if (_pidSampleRate != value)
                {
                    _pidSampleRate = value;
                    SendCalibrationFactor((byte)PID_SAMPLE_RATE, (short)value);
                    PropertyChanged(this, new PropertyChangedEventArgs("PIDSampleRate"));
                }
            }
        }

        double _pitchSteadyBand;
        public double PitchSteadyBand
        {
            get { return _pitchSteadyBand; }
            set
            {
                if (_pitchSteadyBand != value)
                {
                    _pitchSteadyBand = value;
                    SendCalibrationFactor((byte)K_PITCH_STABLE_BAND, (short)(_pitchSteadyBand * STABILITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchSteadyBand"));
                }
            }
        }

        double _rollSteadyBand;
        public double RollSteadyBand
        {
            get { return _rollSteadyBand; }
            set
            {
                if (_rollSteadyBand != value)
                {
                    _rollSteadyBand = value;
                    SendCalibrationFactor((byte)K_ROLL_STABLE_BAND, (short)(_rollSteadyBand * STABILITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollSteadyBand"));
                }
            }
        }

        double _headingSteadyBand;
        public double HeadingSteadyBand
        {
            get { return _headingSteadyBand; }
            set
            {
                if (_headingSteadyBand != value)
                {
                    _headingSteadyBand = value;
                    SendCalibrationFactor((byte)K_HEADING_STABLE_BAND, (short)(_headingSteadyBand * STABILITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("HeadingSteadyBand"));
                }
            }
        }

        double _altitudeSteadyBand;
        public double AltitudeSteadyBand
        {
            get { return _altitudeSteadyBand; }
            set
            {
                if (_altitudeSteadyBand != value)
                {
                    _altitudeSteadyBand = value;
                    SendCalibrationFactor((byte)K_ALTITUDE_STABLE_BAND, (short)(_altitudeSteadyBand * STABILITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeSteadyBand"));
                }
            }
        }

        double _pitchSensitivity;
        public double PitchSensitivity
        {
            get { return _pitchSensitivity; }
            set
            {
                if (_pitchSensitivity != value)
                {
                    _pitchSensitivity = value;
                    SendCalibrationFactor((byte)K_PITCH_SENSITIVITY, (short)(_pitchSensitivity * SENSITIVITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchSensitivity"));
                }
            }
        }

        double _rollSensitivity;
        public double RollSensitivity
        {
            get { return _rollSensitivity; }
            set
            {
                if (_rollSensitivity != value)
                {
                    _rollSensitivity = value;
                    SendCalibrationFactor((byte)K_ROLL_SENSITIVITY, (short)(_rollSensitivity * SENSITIVITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollSensitivity"));
                }
            }
        }

        double _yawSensitivity;
        public double YawSensitivity
        {
            get { return _yawSensitivity; }
            set
            {
                if (_yawSensitivity != value)
                {
                    _yawSensitivity = value;
                    SendCalibrationFactor((byte)K_YAW_SENSITIVITY, (short)(_yawSensitivity * SENSITIVITY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawSensitivity"));
                }
            }
        }


        double _pitchP;
        public double PitchP
        {
            get { return _pitchP * P_DISPLAY_SCALER; }
            set
            {
                if (_pitchP != value / P_DISPLAY_SCALER)
                {
                    _pitchP = value / P_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_PITCH_P_ADDR, (short)(_pitchP * P_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchP"));
                }
            }
        }

        double _pitchI;
        public double PitchI
        {
            get { return _pitchI * I_DISPLAY_SCALER; }
            set
            {
                if (_pitchI != value / I_DISPLAY_SCALER)
                {
                    _pitchI = value / I_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_PITCH_I_ADDR, (short)(_pitchI * I_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchI"));
                }
            }
        }
        double _pitchD;
        public double PitchD
        {
            get { return _pitchD * D_DISPLAY_SCALER; }
            set
            {
                if (_pitchD != value / D_DISPLAY_SCALER)
                {
                    _pitchD = value / D_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_PITCH_D_ADDR, (short)(_pitchD * D_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchD"));
                }
            }
        }

        double _pitchDecayFactor;
        public double PitchIDecayFactor
        {
            get { return _pitchDecayFactor; }
            set
            {
                if (_pitchDecayFactor != value)
                {
                    _pitchDecayFactor = value;
                    SendCalibrationFactor((byte)K_PITCH_I_DECAY_FACTOR_ADDR, (short)(_pitchDecayFactor * I_DECAY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchIDecayFactor"));
                }
            }
        }

        double _pitchIMax;
        public double PitchIMax
        {
            get { return _pitchIMax; }
            set
            {
                if (_pitchIMax != value)
                {
                    _pitchIMax = value;
                    SendCalibrationFactor((byte)K_PITCH_I_MAX_ADDR, (short)(_pitchIMax * I_MAX_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchIMax"));
                }
            }
        }

        byte _pitchDFilter;
        public byte PitchDFilter
        {
            get { return _pitchDFilter; }
            set
            {
                if (_pitchDFilter != value)
                {
                    _pitchDFilter = value;
                    SendCalibrationFactor((byte)K_PITCH_FILTER_D, (short)_pitchDFilter);
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchDFilter"));
                }
            }
        }

        double _rollP;
        public double RollP
        {
            get { return _rollP * P_DISPLAY_SCALER; }
            set
            {
                if (_rollP != value / P_DISPLAY_SCALER)
                {
                    _rollP = value / P_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ROLL_P_ADDR, (short)(_rollP * P_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollP"));
                }
            }
        }

        double _rollI;
        public double RollI
        {
            get { return _rollI * I_DISPLAY_SCALER; }
            set
            {
                if (_rollI != value / I_DISPLAY_SCALER)
                {
                    _rollI = value / I_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ROLL_I_ADDR, (short)(_rollI * I_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollI"));
                }
            }
        }

        double _rollD;
        public double RollD
        {
            get { return _rollD * D_DISPLAY_SCALER; }
            set
            {
                if (_rollD != value / D_DISPLAY_SCALER)
                {
                    _rollD = value / D_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ROLL_D_ADDR, (short)(_rollD * D_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollD"));
                }
            }
        }

        double _rollDecayFactor;
        public double RollIDecayFactor
        {
            get { return _rollDecayFactor; }
            set
            {
                if (_rollDecayFactor != value)
                {
                    _rollDecayFactor = value;
                    SendCalibrationFactor((byte)K_ROLL_I_DECAY_FACTOR_ADDR, (short)(_rollDecayFactor * I_DECAY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollIDecayFactor"));
                }
            }
        }

        double _rollIMax;
        public double RollIMax
        {
            get { return _rollIMax; }
            set
            {
                if (_rollIMax != value)
                {
                    _rollIMax = value;
                    SendCalibrationFactor((byte)K_ROLL_I_MAX_ADDR, (short)(_rollIMax * I_MAX_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollIMax"));
                }
            }
        }

        byte _rollDFilter;
        public byte RollDFilter
        {
            get {
                Debug.WriteLine("Getting Roll D Filter ");    
                return _rollDFilter; 
            }
            set
            {
                if (_rollDFilter != value)
                {
                    _rollDFilter = value;
                    SendCalibrationFactor((byte)K_ROLL_FILTER_D, (short)_rollDFilter);
                    PropertyChanged(this, new PropertyChangedEventArgs("RollDFilter"));
                }
            }
        }

        double _yawP;
        public double YawP
        {
            get { return _yawP * P_DISPLAY_SCALER; }
            set
            {
                if (_yawP != value / P_DISPLAY_SCALER)
                {
                    _yawP = value / P_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_YAW_P_ADDR, (short)(_yawP * P_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawP"));
                }
            }
        }


        double _yawI;
        public double YawI
        {
            get { return _yawI * I_DISPLAY_SCALER; }
            set
            {
                if (_yawI != value / I_DISPLAY_SCALER)
                {
                    _yawI = value / I_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_YAW_I_ADDR, (short)(_yawI * I_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawI"));
                }
            }
        }


        double _yawD;
        public double YawD
        {
            get { return _yawD * D_DISPLAY_SCALER; }
            set
            {
                if (_yawD != value / D_DISPLAY_SCALER)
                {
                    _yawD = value / D_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_YAW_D_ADDR, (short)(_yawD * D_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawD"));
                }
            }
        }


        double _yawDecayFactor;
        public double YawIDecayFactor
        {
            get { return _yawDecayFactor; }
            set
            {
                if (_yawDecayFactor != value)
                {
                    _yawDecayFactor = value;
                    SendCalibrationFactor((byte)K_YAW_I_DECAY_FACTOR_ADDR, (short)(_yawDecayFactor * I_DECAY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawIDecayFactor"));
                }
            }
        }

        double _yawIMax;
        public double YawIMax
        {
            get { return _yawIMax; }
            set
            {
                if (_yawIMax != value)
                {
                    _yawIMax = value;
                    SendCalibrationFactor((byte)K_YAW_I_MAX_ADDR, (short)(_yawIMax * I_MAX_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawIMax"));
                }
            }
        }

        byte _yawDFilter;
        public byte YawDFilter
        {
            get { return _yawDFilter; }
            set
            {
                if (_yawDFilter != value)
                {
                    _yawDFilter = value;
                    SendCalibrationFactor((byte)K_YAW_FILTER_D, (short)_yawDFilter);
                    PropertyChanged(this, new PropertyChangedEventArgs("YawDFilter"));
                }
            }
        }

        double _altitudeP;
        public double AltitudeP
        {
            get { return _altitudeP * P_DISPLAY_SCALER; }
            set
            {
                if (_altitudeP != value / P_DISPLAY_SCALER)
                {
                    _altitudeP = value / P_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ALTITUDE_P_ADDR, (short)(_altitudeP * P_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeP"));
                }
            }
        }


        double _altitudeI;
        public double AltitudeI
        {
            get { return _altitudeI * I_DISPLAY_SCALER; }
            set
            {
                if (_altitudeI != value / I_DISPLAY_SCALER)
                {
                    _altitudeI = value / I_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ALTITUDE_I_ADDR, (short)(_altitudeI * I_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeI"));
                }
            }
        }


        double _altitudeD;
        public double AltitudeD
        {
            get { return _altitudeD * D_DISPLAY_SCALER; }
            set
            {
                if (_altitudeD != value / D_DISPLAY_SCALER)
                {
                    _altitudeD = value / D_DISPLAY_SCALER;
                    SendCalibrationFactor((byte)K_ALTITUDE_D_ADDR, (short)(_altitudeD * D_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeD"));
                }
            }
        }

        double _altitudeDecayFactor;
        public double AltitudeIDecayFactor
        {
            get { return _altitudeDecayFactor; }
            set
            {
                if (_altitudeDecayFactor != value)
                {
                    _altitudeDecayFactor = value;
                    SendCalibrationFactor((byte)K_ALTITUDE_I_DECAY_FACTOR_ADDR, (short)(_altitudeDecayFactor * I_DECAY_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeIDecayFactor"));
                }
            }
        }

        double _altitudeIMax;
        public double AltitudeIMax
        {
            get { return _altitudeIMax; }
            set
            {
                if (_altitudeIMax != value)
                {
                    _altitudeIMax = value;
                    SendCalibrationFactor((byte)K_ALTITUDE_I_MAX_ADDR, (short)(_altitudeIMax * I_MAX_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeIMax"));
                }
            }
        }

        byte _altitudeDFilter;
        public byte AltitudeDFilter
        {
            get { return _altitudeDFilter; }
            set
            {
                if (_altitudeDFilter != value)
                {
                    _altitudeDFilter = value;
                    SendCalibrationFactor((byte)K_ALTITUDE_FILTER_D, (short)_altitudeDFilter);
                    PropertyChanged(this, new PropertyChangedEventArgs("AltitudeDFilter"));
                }
            }
        }

        double _escStarboardFront;
        public double EscStarboardFront
        {
            get { return _escStarboardFront; }
            set
            {
                if (_escStarboardFront != value)
                {
                    _escStarboardFront = value;
                    SendCalibrationFactor((byte)K_STARBOARD_FRONT, (short)(_escStarboardFront * ESC_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscStarboardFront"));
                }
            }
        }

        double _escStarboardRear;
        public double EscStarboardRear
        {
            get { return _escStarboardRear; }
            set
            {
                if (_escStarboardRear != value)
                {
                    _escStarboardRear = value;
                    SendCalibrationFactor((byte)K_STARBOARD_REAR, (short)(_escStarboardRear * ESC_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscStarboardRear"));
                }
            }
        }

        double _escPortFront;
        public double EscPortFront
        {
            get { return _escPortFront; }
            set
            {
                if (_escPortFront != value)
                {
                    _escPortFront = value;
                    SendCalibrationFactor((byte)K_PORT_FRONT_ADDR, (short)(value * ESC_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscPortFront"));
                }
            }
        }

        double _escPortRear;
        public double EscPortRear
        {
            get { return _escPortRear; }
            set
            {
                if (_escPortRear != value)
                {
                    _escPortRear = value;
                    SendCalibrationFactor((byte)K_PORT_REAR_ADDR, (short)(_escPortRear * ESC_SCALER));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscPortRear"));
                }
            }
        }

        byte _escUpateRate;
        public byte ESCUpdateRate
        {
            get { return _escUpateRate; }
            set
            {
                if (_escUpateRate != value)
                {
                    _escUpateRate = value;
                    SendCalibrationFactor((byte)K_ESC_UPDATE_RATE, (short)_escUpateRate); //Even though this is only a byte, we always send two bytes for the value.  The firmware will sort this out.
                    PropertyChanged(this, new PropertyChangedEventArgs("ESCUpdateRate"));
                }
            }
        }

        byte _frameConfig;
        public byte FrameConfig
        {
            get { return _frameConfig; }
            set
            {
                if (_frameConfig != value)
                {
                    _frameConfig = value;
                    SendCalibrationFactor((byte)FRAME_CONFIG, (short)_frameConfig);
                    PropertyChanged(this, new PropertyChangedEventArgs("FrameConfig"));
                }
            }
        }

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
                    PropertyChanged(this, new PropertyChangedEventArgs("MotorPower"));
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
                    PropertyChanged(this, new PropertyChangedEventArgs("Motor"));
                }
            }
        }

        public void SendMotorPower(byte power)
        {
            Debug.WriteLine("Sending motor power {0} to {1}", power, _motor);

            var msg = new OutgoingMessage() { MessageId = GPIOModule.CMD_Set_ESC_Power, ModuleType = OutgoingMessage.ModuleTypes.GPIO, ExpectACK = true };
            msg.AddByte(_motor);
            msg.AddByte(power);
            App.Commo.Send(msg);
            PropertyChanged(this, new PropertyChangedEventArgs("Motor"));

            App.Commo.Throttle = power;
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


        public event PropertyChangedEventHandler PropertyChanged;

        ObservableCollection<KeyDisplay> _dFilter = new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "None", Key =  0 },
             new KeyDisplay{Display = "10Hz", Key =  1 },
             new KeyDisplay{Display = "15Hz", Key =  2},
             new KeyDisplay{Display = "20Hz", Key =  3},
             new KeyDisplay{Display = "25Hz", Key =  4},
             new KeyDisplay{Display = "30Hz", Key =  5},
             new KeyDisplay{Display = "35Hz", Key =  6},
             new KeyDisplay{Display = "40Hz", Key =  7},
        };

        public ObservableCollection<KeyDisplay> DFilterCollection { get { return _dFilter; } }

        ObservableCollection<KeyDisplay> _frameConfigCollection= new ObservableCollection<KeyDisplay>() {
             new KeyDisplay{Display = "+", Key =  0 },
             new KeyDisplay{Display = "X", Key =  1 },
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
                new KeyDisplay{Display = "0", Key = 0x00 },
                new KeyDisplay{Display = "5%", Key = (byte)(0xFF * 0.05f) },
                new KeyDisplay{Display = "10%", Key = (byte)(0xFF * 0.1f) },
                new KeyDisplay{Display = "15%", Key = (byte)(0xFF * 0.15f) },
                new KeyDisplay{Display = "20%", Key = (byte)(0xFF * 0.2f) },
                new KeyDisplay{Display = "23%", Key = (byte)(0xFF * 0.23f) },
                new KeyDisplay{Display = "26%", Key = (byte)(0xFF * 0.26f) },
                new KeyDisplay{Display = "30%", Key = (byte)(0xFF * 0.3f) },
                new KeyDisplay{Display = "40%", Key = (byte)(0xFF * 0.4f) },
                new KeyDisplay{Display = "50%", Key = (byte)(0xFF * 0.5f) },
                new KeyDisplay{Display = "53%", Key = (byte)(0xFF * 0.53f) },
                new KeyDisplay{Display = "56%", Key = (byte)(0xFF * 0.56f) },
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
    }

    public class ReceivedPIDEventArgs
    {
        public GPIOConfig PIDValue { get; set; }
    }
}
