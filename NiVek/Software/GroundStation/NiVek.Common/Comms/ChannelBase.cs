using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using NiVek.Common.Services;
using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace NiVek.Common
{
    public abstract class ChannelBase
    {
        public event EventHandler<Drone> OnDroneConnected;
        public event EventHandler<Drone> OnDroneDisconnected;

        public event EventHandler OnConnect;
        public event EventHandler OnConnecting;
        public event EventHandler OnDisconnect;
        public event EventHandler OnDidNotConnect;

        public event EventHandler<IncomingMessage> MessageReady;

        private List<Drone> ConnectedDrones;
        public ObservableCollection<Drone> ThreadSafeConnectedDrones;

        MessageParser _parser;

        NiVek.Common.Comms.IncomingMessage _currentMessage;

        //private IUIThread _uiThread;

        private int _errorCount;
        private int _totalCount;

        public String IPAddress { get; set; }
        public int Port { get; set; }

        public bool IsConnectionOn { get; set; }

        protected abstract void Send(byte[] buffer, int offset, int len);
        public NiVek.Common.Comms.Common.ConnectionStates ConnectionState { get; set; }

        private DateTime _lastMessgeReceived = DateTime.MinValue;


        public Drone CurrentDrone { get; set; }

        public enum LogLevel
        {
            Verbose,
            Info,
            Warnings,
            Errors
        }

        private ushort _nextSerialNumber = 0;

        protected ushort GetNextSerialNumber()
        {
            return _nextSerialNumber++;
        }


        public void WriteDebugMessage(LogLevel level, String message)
        {

        }

        public abstract void Connect(String host, int port);

        public abstract void Close();

        protected virtual void Connected()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Connected;
            /*
            if (OnConnect != null)
            {
                if (_uiThread != null && _uiThread.InvokeRequired)
                    _uiThread.Invoke(() => OnConnect(this, null));
                else
                    OnConnect(this, null);
            }*/
        }

        protected virtual void Connecting()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Connecting;
            /*
            if (OnConnecting != null)
            {
                if (_uiThread != null && _uiThread.InvokeRequired)
                    _uiThread.Invoke(() => OnConnecting(this, null));
                else
                    OnConnecting(this, null);
            }*/
        }

        protected void DidNotConnect()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;

            /*
            if (OnDidNotConnect != null)
            {
                if (_uiThread != null && _uiThread.InvokeRequired)
                    _uiThread.Invoke(() => OnDidNotConnect(this, null));
                else
                    OnDidNotConnect(this, null);
            }*/
        }

        protected void Disconnected()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;

            Debug.WriteLine("Writing Disconnected In Library.");
            /*
            if (OnDisconnect != null)
            {
                if (_uiThread != null && _uiThread.InvokeRequired)
                    _uiThread.Invoke(() => OnDisconnect(this, null));
                else
                    OnDisconnect(this, null);
            }*/
        }


        protected void CheckWatchdog()
        {
            if (false && ConnectionState == Comms.Common.ConnectionStates.Connected)
            {
                if (_lastMessgeReceived < DateTime.Now.Add(-TimeSpan.FromSeconds(3)))
                {
                    Disconnected();
                 //   foreach (var drone in ConnectedDrones)
                   //     DroneDisconnected(drone);

                    ConnectedDrones.Clear();
                }
                else
                {
                    foreach (var drone in ConnectedDrones)
                    {
                        if ((DateTime.Now - drone.LastContactDateStamp).TotalSeconds > 2)
                        {
                            Debug.WriteLine("looks like drone {0} - {1} was disconnected", drone.DroneName, drone.DroneId);
                            drone.Connected = false;
                        }
                    }

                    var dronesToRemove = ConnectedDrones.Where(drn => drn.Connected == false).ToList();
                    foreach (var droneToRemove in dronesToRemove)
                    {
                        ConnectedDrones.Remove(droneToRemove);
               //         DroneDisconnected(droneToRemove);
                    }
                }
            }
        }

        public async Task<T> GetAsync<T>(byte destAddress, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId) where T : IModel, new()
        {
            T model = new T();

            var messageReceived = new SemaphoreSlim(0);

            EventHandler<IncomingMessage> msgReceivedHander = (sender, msg) =>
            {
                if (msg.MessageId == responseMessageId)
                {
                    model = new T();
                    //model.Channel = this;
                    model.Drone = CurrentDrone;
                    model.Update(new BufferReader(msg.Payload, 0, msg.PayloadSize));
                    messageReceived.Release();
                }
            };

            int retries = 0;

            MessageReady += msgReceivedHander;

            while (retries++ < 5 && model.IsReady == false)
            {
                var start = DateTime.Now;

              //  Send(new OutgoingMessage() { DestinationAddress = destAddress, SourceAddress = LocalAddress, ModuleType = moduleType, MessageId = requestMessageId });
                if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(1500)))
                    Debug.WriteLine("NOPE, NO SUCCESS :(");

                var delta = DateTime.Now - start;

                Debug.WriteLine("Response Time was {0} ms", delta.TotalMilliseconds);
            }

            MessageReady -= msgReceivedHander;

            return model;
        }



        public async Task<byte[]> GetBufferAsync(byte destAddress, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId)
        {
            var messageReceived = new SemaphoreSlim(0);

            byte[] buffer = null;

            EventHandler<IncomingMessage> msgReceivedHander = (sender, msg) =>
            {
                if (msg.MessageId == responseMessageId && msg.SourceAddress == destAddress)
                {
                    buffer = new byte[msg.PayloadSize];
                    msg.Payload.CopyTo(buffer, 0);
                    messageReceived.Release();
                }
            };

            int retries = 0;

            MessageReady += msgReceivedHander;

            while (retries++ < 5 && buffer == null)
            {
                var start = DateTime.Now;
                //Send(new OutgoingMessage() { SourceAddress = AppServices.LocalAddress, DestinationAddress = destAddress, ModuleType = moduleType, MessageId = requestMessageId });
                if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(1500)))
                    Debug.WriteLine("No response, attempt number {0}", retries);

                var delta = DateTime.Now - start;

                //                Debug.WriteLine("Response Time was {0}ms", delta.TotalMilliseconds);
            }

            MessageReady -= msgReceivedHander;

            return buffer;
        }

        public async Task<bool> SendCommandAsync(byte destAddress, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId)
        {
            var msg = new OutgoingMessage()
            {
                SourceAddress = AppServices.LocalAddress,
                DestinationAddress = destAddress,
                ModuleType = moduleType,
                MessageId = requestMessageId,
                ExpectACK = true
            };

            return await SendMessageAsync(msg);
        }


        public async Task<bool> SendMessageAsync(OutgoingMessage msg)
        {
            ushort serialNumber = GetNextSerialNumber();
            msg.SerialNumber = serialNumber;

            if (!msg.SourceAddress.HasValue)
                msg.SourceAddress = AppServices.LocalAddress;

            if (!msg.DestinationAddress.HasValue)
                msg.DestinationAddress = CurrentDrone.DroneId;

            NiVek.Common.Comms.MessageEntry.AckStatusTypes responseStatus = MessageEntry.AckStatusTypes.Pending;

            var messageReceived = new SemaphoreSlim(0);

            EventHandler<IncomingMessage> msgReceivedHander = (sender, incomingMessage) =>
            {
                if (incomingMessage.SystemId == (byte)NiVek.Common.Comms.Common.ModuleTypes.System
                    && (incomingMessage.MessageId == IncomingMessage.Ack || incomingMessage.MessageId == IncomingMessage.Nak))
                {
                    UInt16 responseSerialNumber = (UInt16)(incomingMessage.Payload[0] << 8 | incomingMessage.Payload[1]);

                    if (responseSerialNumber == serialNumber)
                    {
                        responseStatus = (incomingMessage.MessageId == IncomingMessage.Ack) ? NiVek.Common.Comms.MessageEntry.AckStatusTypes.Ack : NiVek.Common.Comms.MessageEntry.AckStatusTypes.NotAck;
                        messageReceived.Release();
                    }
                }
            };

  //          if (msg.ExpectACK)
//                AddMessageEntry(msg);

            var attempts = 0;

            MessageReady += msgReceivedHander;

            while (attempts < 5 && responseStatus != MessageEntry.AckStatusTypes.Ack)
            {
                var start = DateTime.Now;

                var buffer = msg.Buffer;
                Send(buffer, 0, buffer.Length);

                if (!msg.ExpectACK)
                {
                    responseStatus = MessageEntry.AckStatusTypes.Ack;
                    Debug.WriteLine("Fire and forget, not waiting around for an ACK");
                }
                else
                {
                    if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(1000)))
                        Debug.WriteLine("NOPE, NO SUCCESS :( REtry Count {0}", attempts);
                    else
                    {
                        var delta = DateTime.Now - start;
                        Debug.WriteLine("Response Time was {0}ms", delta.TotalMilliseconds);
                    }
                }

                attempts++;
            }

            MessageReady -= msgReceivedHander;

            return responseStatus == MessageEntry.AckStatusTypes.Ack;
        }

        public ObservableCollection<MessageEntry> MessageLog
        {
            get { return null; }// _messageLog; }
        }

        protected void Parse(byte[] buffer, int start, int length)
        {
            for (var idx = 0; idx < length; ++idx)
                _parser.ParseCh(buffer[idx], ref _currentMessage);
        }

        public void ResetErrorStatus()
        {
            _errorCount = 0;
            _totalCount = 0;
        }

        public double ErrorRate
        {
            get
            {
                if (_errorCount + _totalCount > 0)
                    return _errorCount * 100.0f / _totalCount;

                return 0;
            }
        }


        
    }
}