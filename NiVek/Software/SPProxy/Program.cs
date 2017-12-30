using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SPProxy
{
    class Program
    {
        static System.IO.Ports.SerialPort _rcvr;

        static Server.ClientManager _server;

        static Timer _keepAliveTimer;
        static Timer _serialWatchDogTimer;

        static bool _serialPortConnected;
        static bool _serialPortWatchdogCleared;

        static bool _keepAliveRequired;

        static Commo.MessageParser _parser;

        static void Main(string[] args)
        {
            _parser = new Commo.MessageParser();

            _rcvr = new System.IO.Ports.SerialPort("COM8",115200);
            _rcvr.Open();
                
            _server = new Server.ClientManager();
            _server.Start();

            Console.WriteLine("Listening on: ");
            foreach (var addr in _server.IPAddresses)
                Console.WriteLine("\t" + addr);

            Console.WriteLine("Connect on Port: " + Common.ListenerPort);

            _server.DataReceived += (sndr, rcvArgs) =>
            {
                if(_rcvr.IsOpen)
                    _rcvr.Write(rcvArgs.Buffer, 0, rcvArgs.BufferLen);

            };

            _rcvr.DataReceived += (sndr, drArgs) =>
            {
                var bytesToRead = _rcvr.BytesToRead;
                var buffer = new byte[bytesToRead];

                _rcvr.Read(buffer, 0, bytesToRead);

                _server.SendData(buffer);

                if (_serialPortConnected == false)
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine("NiVek QC Connected!");
                    Console.ForegroundColor = ConsoleColor.White;
                    _serialPortConnected = true;
                }

                _serialPortWatchdogCleared = true;
                _keepAliveRequired = false;
            };

            _keepAliveTimer = new Timer((state) =>
            {
                if(_keepAliveRequired)   
                {
                    var msg = new Commo.IncomingMessage() { MessageId = 100, SystemId = 200 };
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.WriteLine("Serial Buffer Timeout, Sending Watchdog");
                    Console.ForegroundColor = ConsoleColor.White;
                    _server.SendData(msg.Buffer);
                }

                _keepAliveRequired = true;
            },null, 0,2000);

            _serialWatchDogTimer = new Timer((state) =>
                {
                    if (_serialPortWatchdogCleared == false && _serialPortConnected)
                    {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine("!!NiVek QC Disconnected!!");
                        Console.ForegroundColor = ConsoleColor.White;
                        _serialPortConnected = false;
                    }

                    _serialPortWatchdogCleared = false;

                }, null, 0, 2000);


            var keepRunning = true;
            while (keepRunning)
            {
                Thread.Sleep(1);
                if (Console.KeyAvailable)
                {
                    _server.Stop();
                    _rcvr.Close();
                    _rcvr.Dispose();
                    keepRunning = false;
                }
            }

            
        }

    }
}
