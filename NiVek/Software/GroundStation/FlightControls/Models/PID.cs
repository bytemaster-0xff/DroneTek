using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NiVek.FlightControls.Commo;
using System.ComponentModel;

namespace NiVek.FlightControls.Models
{
    public class PID : INotifyPropertyChanged
    {
        NiVekConsole _console;

        public PID(byte[] buffer, NiVekConsole console)
        {
            var byteIndex = 0;

            _console = console;

            _rollP = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _rollI = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 1000.0f;
            _rollD = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;

            _pitchP = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _pitchI = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 1000.0f;
            _pitchD = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;

            _yawP = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _yawI = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 1000.0f;
            _yawD = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;

            _escFront = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _escLeft = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _escRight = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
            _escRear = (short)(buffer[byteIndex++] << 8 | buffer[byteIndex++]) / 100.0f;
        }

        public PID(String msg, NiVekConsole console)
        {
            _console = console;

            var parts = msg.Split(',');
            // Position 0 - Serial Number
            // Position 1 - Message Type
            _rollP = Convert.ToSingle(parts[2]) / 100.0f;
            _rollI = Convert.ToSingle(parts[3]) / 100.0f;
            _rollD = Convert.ToSingle(parts[4]) / 100.0f;

            _pitchP = Convert.ToSingle(parts[5]) / 100.0f;
            _pitchI = Convert.ToSingle(parts[6]) / 100.0f;
            _pitchD = Convert.ToSingle(parts[7]) / 100.0f;

            _yawP = Convert.ToSingle(parts[8]) / 100.0f;
            _yawI = Convert.ToSingle(parts[9]) / 100.0f;
            _yawD = Convert.ToSingle(parts[10]) / 100.0f;

            _escFront = Convert.ToSingle(parts[12]) / 100.0f;
            _escLeft = Convert.ToSingle(parts[13]) / 100.0f;
            _escRight = Convert.ToSingle(parts[14]) / 100.0f;
            _escRear = Convert.ToSingle(parts[14]) / 100.0f;
        }

        void SendCalibrationFactor(byte address, short value)
        {
            var msg = new OutgoingMessage() { MessageId = GPIOModule.CMD_SetPIDConstant, ModuleType = OutgoingMessage.ModuleTypes.GPIO };
            msg.Add(address);
            msg.Add(value);
            _console.Send(msg);
        }

        double _pitchP;
        public double PitchP
        {
            get { return _pitchP; }
            set 
            {
                if (_pitchP != value)
                {
                    _pitchP = value;
                    SendCalibrationFactor((byte)0x56, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchP"));   
                }
            }
        }

        double _pitchI;
        public double PitchI
        {
            get { return _pitchI; }
            set
            {
                if (_pitchI != value)
                {
                    _pitchI = value;
                    SendCalibrationFactor((byte)0x58, (short)(value * 1000.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchI"));
                }
            }
        }

        double _pitchD;
        public double PitchD
        {
            get { return _pitchD; }
            set
            {
                if (_pitchD != value)
                {
                    _pitchD = value;
                    SendCalibrationFactor((byte)0x5a, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("PitchD"));
                }
            }
        }

        double _rollP;
        public double RollP
        {
            get { return _rollP; }
            set
            {
                if (_rollP != value)
                {
                    _rollP = value;
                    SendCalibrationFactor((byte)0x50, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollP"));
                }
            }
        }

        double _rollI;
        public double RollI
        {
            get { return _rollI; }
            set
            {
                if (_rollI != value)
                {
                    _rollI = value;
                    SendCalibrationFactor((byte)0x52, (short)(value * 1000.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollI"));
                }
            }
        }

        double _rollD;
        public double RollD
        {
            get { return _rollD; }
            set
            {
                if (_rollD != value)
                {
                    _rollD = value;
                    SendCalibrationFactor((byte)0x54, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("RollD"));
                }
            }
        }


        double _yawP;
        public double YawP
        {
            get { return _yawP; }
            set
            {
                if (_yawP != value)
                {
                    _yawP = value;
                    SendCalibrationFactor((byte)0x5c, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawP"));
                }
            }
        }


        double _yawI;
        public double YawI
        {
            get { return _yawI; }
            set
            {
                if (_yawI != value)
                {
                    _yawI = value;
                    SendCalibrationFactor((byte)0x5e, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawI"));
                }
            }
        }


        double _yawD;
        public double YawD
        {
            get { return _yawD; }
            set
            {
                if (_yawD != value)
                {
                    _yawD = value;
                    SendCalibrationFactor((byte)0x60, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("YawD"));
                }
            }
        }

        double _escFront;
        public double EscFront
        {
            get { return _escFront; }
            set
            {
                if (_escFront != value)
                {
                    _escFront = value;
                    SendCalibrationFactor((byte)0x62, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscFront"));
                }
            }
        }

        double _escRight;
        public double EscRight
        {
            get { return _escRight; }
            set
            {
                if (_escRight != value)
                {
                    _escRight = value;
                    SendCalibrationFactor((byte)0x64, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscRight"));
                }
            }
        }

        double _escLeft;
        public double EscLeft
        {
            get { return _escLeft; }
            set
            {
                if (_escLeft != value)
                {
                    _escLeft = value;
                    SendCalibrationFactor((byte)0x66, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscLeft"));
                }
            }
        }

        double _escRear;
        public double EscRear
        {
            get { return _escRear; }
            set
            {
                if (_escRear != value)
                {
                    _escRear = value;
                    SendCalibrationFactor((byte)0x68, (short)(value * 100.0));
                    PropertyChanged(this, new PropertyChangedEventArgs("EscRear"));
                }
            }
        }


        public event PropertyChangedEventHandler PropertyChanged;
    }

    public class ReceivedPIDEventArgs
    {
        public PID PIDValue {get; set;}
    }
}
