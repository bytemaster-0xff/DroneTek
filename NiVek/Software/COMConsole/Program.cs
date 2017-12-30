using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO.Ports;
using System.Diagnostics;

namespace COMConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            String portName = args.Length > 0 ? args[0] : "COM10";

            Console.WriteLine("Opening {0} at 115200 baud for console", portName);

            var port = new System.IO.Ports.SerialPort(portName, 115200);
            port.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(port_DataReceived);
            port.Open();
            while (true)
            {
                Thread.Sleep(1000);
            }

            port.Close();
            port.Dispose();

        }

        static void port_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            var port = (SerialPort)sender;
            var bufferSize = port.BytesToRead;
            var buffer = new byte[bufferSize];

            port.Read(buffer, 0, bufferSize);

            var str = System.Text.ASCIIEncoding.ASCII.GetString(buffer);
            Console.Write(str);

        }
    }
}
