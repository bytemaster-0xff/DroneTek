﻿using NiVek.Common;
using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Core;
using Windows.UI.Xaml;

namespace NiVek.FlightControls.Commo
{
    public class TCPChannel : Channel
    {
        enum SocketConnectionStatus
        {
            Disconnecting,
            Disconnected,
            Connected,
            Connecting
        }

        SocketConnectionStatus _socketConnectionStatus = SocketConnectionStatus.Disconnected;

        #region Fields
        DispatcherTimer _commoTimeout; 
        StreamSocket _socket;
        DataReader _reader;
        DataWriter _writer;

        String _address;
        int _port;

        byte[] _msgBuffer;
        #endregion

        public TCPChannel()
        {
            _commoTimeout = new DispatcherTimer
            {
                Interval = TimeSpan.FromMilliseconds(1000)
            };
            _commoTimeout.Tick += _commoTimeout_Tick;
            _commoTimeout.Start();
        }

        async void AttemptConnect()
        {
            await ConnectAsync(Address, Port);
        }
        
        async void _commoTimeout_Tick(object sender, object e)
        {
            if (_socketConnectionStatus == SocketConnectionStatus.Disconnected)
            {
                await ConnectAsync(Address, Port);
            }            
        }

        #region Messageing Stuff
        protected async override void Send(byte[] buffer, int offset, int len)
        {
            if (_writer != null && _socketConnectionStatus == SocketConnectionStatus.Connected)
            {
                try
                {
                    _writer.WriteBytes(buffer);                    
                    await _writer.StoreAsync();
                }
                catch(Exception ex )
                {
                    Debug.WriteLine("COULD NOT CONNECT ATTEMPT TO CLOSE!");

                    Disconnect(ex.Message);
                }
            }
        }
        #endregion

        #region Listen Utility Method
        public async void Listen()
        {
                while (ConnectionState == NiVek.Common.Comms.Common.ConnectionStates.Connected )
                {
                    long msgBytes = 0;
                    try
                    {
                        msgBytes = await _reader.LoadAsync(128);
                        if (msgBytes == -1)
                            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;
                        else
                        {
                            _msgBuffer = new byte[msgBytes];
                            _reader.ReadBytes(_msgBuffer);

                            Parse(_msgBuffer, 0, (int)msgBytes);
                        }
                    }
                    catch (InvalidOperationException)
                    {
                        ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;

                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine("Exception doing something with a message: " + ex.Message);
                        Debug.WriteLine(ex.StackTrace);

                        ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;
                    }
                }

                Debug.WriteLine("Connection Dropped..." + ConnectionState.ToString());

                Disconnected("Connection Dropped...");
        }
       #endregion

        public async override Task ConnectAsync(String host, short port)
        {
            _socketConnectionStatus = SocketConnectionStatus.Connecting;
            _address = host;
            _port = port;

            _reader = null;
            _writer = null;
            _socket = null;            

            if (ConnectionState != NiVek.Common.Comms.Common.ConnectionStates.Connected)
            {

                _socket = new StreamSocket();

                try
                {
                    await _socket.ConnectAsync(new HostName(host), port.ToString());

                    _writer = new DataWriter(_socket.OutputStream);
                    _reader = new DataReader(_socket.InputStream)
                    {
                        InputStreamOptions = InputStreamOptions.Partial
                    };

                    Connected();

                    _socketConnectionStatus = SocketConnectionStatus.Connected;
                }
                catch (Exception ex)
                {
                    if (_socket != null)
                        _socket.Dispose();

                    if (_writer != null)
                        _writer.Dispose();

                    if (_reader != null)
                        _reader.Dispose();

                    _socket = null;
                    _reader = null;
                    _writer = null;

                    _socketConnectionStatus = SocketConnectionStatus.Disconnected;
                    DidNotConnect(ex.Message);
                }

                _commoTimeout.Start();
            }
        }

        public override void Disconnect(String reason)
        {
            Disconnecting(reason);

            try
            {
                if (_writer != null)
                    _writer.Dispose();

                if (_reader != null)
                    _reader.Dispose();

                if (_socket != null)
                    _socket.Dispose();

                _writer = null;
                _reader = null;
                _socket = null;

            }
            catch (Exception){}

            _socketConnectionStatus = SocketConnectionStatus.Disconnected;

            Disconnecting(reason);
        }
    }
}