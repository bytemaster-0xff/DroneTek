using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace NiVekConfigurator.Commo
{
    public class Channel
    {
        public event EventHandler Connected;
        public event EventHandler FailedToConnect;
        public event EventHandler Disconnected;
        public event EventHandler<String> MessageReceived;

        StringBuilder _inBuffer;

        SerialPort _port;

        Timer _connectTimer;

        public Channel()
        {
            _connectTimer = new Timer(1000);
            _connectTimer.Elapsed += _connectTimer_Elapsed;
            _connectTimer.Start();
            _inBuffer = new StringBuilder();
        }

        void _connectTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (_port != null)
            {
                try
                {
                    _port.Write("ping\r");
                }
                catch (Exception)
                {
                    try
                    {
                        _port.Dispose();
                    }
                    catch(Exception) { }

                    Disconnected(this, null);

                    _port = null;
                }
            }
            else if (SerialPorts.Contains(PortName))
            {
                try
                {
                _port = new SerialPort(PortName, 57600);
                _port.DataReceived += _port_DataReceived;
                _port.Open();

                Connected(this, null);
                }
                catch(Exception)
                {
                    FailedToConnect(this, null);
                }
            }
        }

        public void Connect(String portName)
        {
            if (_port != null)
            {
                try
                {
                    _port.Close();
                }
                catch (Exception)
                {
                    try
                    {
                        _port.Dispose();
                    }
                    catch (Exception)
                    {

                    }
                }

                _port = null;
            }

            PortName = portName;
        }

        void _port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var bytesRecieved = _port.BytesToRead;
            var buffer = new byte[bytesRecieved];

            _port.Read(buffer, 0, bytesRecieved);
            foreach (var ch in buffer)
            {
                if (ch == '\r')
                {
                    if(_inBuffer.ToString() != "ping" && _inBuffer.ToString() != "pong")
                        MessageReceived(this, _inBuffer.ToString());

                    _inBuffer.Clear();
                }
                else if (ch == '\n') { }
                else
                    _inBuffer.Append((char)ch);
            }
        }

        public List<String> SerialPorts
        {
            get
            {
                return System.IO.Ports.SerialPort.GetPortNames().ToList();
            }
        }        

        public void Send(String msg)
        {
            try
            {
                if (msg.StartsWith("+++"))
                    _port.Write("+++");
                else
                    _port.Write(msg);
            }
            catch (Exception)
            {
                try
                {
                    _port.Dispose();
                }
                catch (Exception) { }

                Disconnected(this, null);

                _port = null;
            }   
        }

        public String PortName
        {
            get { return Properties.Settings.Default.PortName; }
            set
            {
                Properties.Settings.Default.PortName = value;
                Properties.Settings.Default.Save();
            }
        }


        public void Close()
        {
            if (_port != null)
            {
                try
                {
                    _port.Close();
                }
                catch (Exception)
                {
                    try
                    {
                        _port.Dispose();
                    }
                    catch (Exception)
                    {

                    }
                }

                _port = null;
            }
        }

    }
}
