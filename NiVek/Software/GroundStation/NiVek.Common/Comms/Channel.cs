using GalaSoft.MvvmLight;
using NiVek.Common.Models;
using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    public abstract class Channel : ViewModelBase, IChannel
    {
        public event EventHandler OnConnect;
        public event EventHandler OnConnecting;
        public event EventHandler<String> OnDisconnecting;
        public event EventHandler<String> OnDisconnect;
        public event EventHandler<String> OnDidNotConnect;

        public event EventHandler<IncomingMessage> MessageReady;
        MessageParser _parser;

        NiVek.Common.Comms.IncomingMessage _currentMessage;

        protected abstract void Send(byte[] buffer, int offset, int len);

        public abstract Task ConnectAsync(string address, short port);
        public abstract void Disconnect(String reason);


        private int _errorCount;
        private int _totalCount;

        public string Address { get; set; }

        public short Port { get; set; }

        public double ErrorRate
        {
            get { throw new NotImplementedException(); }
        }

        public Channel()
        {
            _parser = new MessageParser(this);
            _messageLog = new System.Collections.ObjectModel.ObservableCollection<MessageEntry>();
            _parser.MessageReady += _parser_MessageReady;
        }

        public void ResetErrorCount() { _errorCount = 0; _totalCount = 0; }
        public void IncrementErrorCount() { ++_errorCount; }
        public void IncrementTotalCount() { ++_totalCount; }


        private Common.ConnectionStates _connectionState = Common.ConnectionStates.Disconnected;
        public Common.ConnectionStates ConnectionState
        {
            get { return _connectionState; }
            set {
                RaisePropertyChanged(() => ConnectionColor);
                Set(ref _connectionState, value); 
            }
        }

        System.Collections.ObjectModel.ObservableCollection<MessageEntry> _messageLog;

        public System.Collections.ObjectModel.ObservableCollection<MessageEntry> MessageLog
        {
            get { return _messageLog; }
        }

        protected void AddMessageEntry(OutgoingMessage msg)
        {
            var msgEntry = new MessageEntry()
            {
                MessageTypeId = msg.MessageId,
                ModuleType = msg.ModuleType,
                SerialNumber = msg.SerialNumber,
                AckStatus = msg.ExpectACK ? MessageEntry.AckStatusTypes.Pending : MessageEntry.AckStatusTypes.NA
            };

            AppServices.UIThread.Invoke(() => MessageLog.Add(msgEntry));
        }

        protected virtual void Connected()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Connected;

            if (OnConnect != null)
                OnConnect(this, null);
        }

        protected virtual void Connecting()
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Connecting;
            if (OnConnecting != null)
                OnConnecting(this, null);
        }

        protected void DidNotConnect(String reason)
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;

            if (OnDidNotConnect != null)
                OnDidNotConnect(this, reason);
        }

        protected void Disconnecting(String reason)
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnecting; 
            
            if (OnDisconnecting != null)
                OnDisconnecting(this, reason);
        }

        protected void Disconnected(String reason)
        {
            ConnectionState = NiVek.Common.Comms.Common.ConnectionStates.Disconnected;

            if (OnDisconnect != null)
                OnDisconnect(this, reason);
        }

        void _parser_MessageReady(object sender, IncomingMessage msg)
        {
            _currentMessage = new IncomingMessage();

            if (ConnectionState != Common.ConnectionStates.Connected)
                Connected();

            MessageReady(this, msg);
        }

        protected void Send(OutgoingMessage msg)
        {
            msg.SerialNumber = AppServices.GetNextSerialNumber(msg.DestinationAddress.Value);

            if (!msg.SourceAddress.HasValue)
                msg.SourceAddress = AppServices.LocalAddress;

            var buffer = msg.Buffer;
            Send(buffer, 0, buffer.Length);

            if (msg.ExpectACK)
                AddMessageEntry(msg);
        }


        public async Task<T> GetAsync<T>(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId) where T : IModel, new()
        {
            T model = new T();

            var messageReceived = new SemaphoreSlim(0);

            EventHandler<IncomingMessage> msgReceivedHander = (sender, msg) =>
            {
                if (msg.MessageId == responseMessageId)
                {
                    model = new T();
                    model.Drone = drone;
                    model.Update(new BufferReader(msg.Payload, 0, msg.PayloadSize));
                    messageReceived.Release();
                }
            };

            int retries = 0;

            MessageReady += msgReceivedHander;

            while (retries++ < 5 && model.IsReady == false)
            {
                var start = DateTime.Now;

                Send(new OutgoingMessage() { DestinationAddress = drone.DroneAddress, SourceAddress = AppServices.LocalAddress, ModuleType = moduleType, MessageId = requestMessageId });
                if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(1500)))
                    Debug.WriteLine("NOPE, NO SUCCESS :(");

                var delta = DateTime.Now - start;

                Debug.WriteLine("Response Time was {0} ms", delta.TotalMilliseconds);
            }

            MessageReady -= msgReceivedHander;

            return model;
        }

        public async Task<byte[]> GetBufferAsync(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId)
        {
            var messageReceived = new SemaphoreSlim(0);

            byte[] buffer = null;

            EventHandler<IncomingMessage> msgReceivedHander = (sender, msg) =>
            {
                //TODO: Need to also check on module type
                if (msg.MessageId == responseMessageId &&
                    msg.SourceAddress == drone.DroneAddress)
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
                Send(new OutgoingMessage() { SourceAddress = AppServices.LocalAddress, DestinationAddress = drone.DroneAddress, ModuleType = moduleType, MessageId = requestMessageId });
                if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(1500)))
                    Debug.WriteLine("No response, attempt number {0}", retries);

                var delta = DateTime.Now - start;

                //                Debug.WriteLine("Response Time was {0}ms", delta.TotalMilliseconds);
            }

            MessageReady -= msgReceivedHander;

            return buffer;
        }

        public async Task<bool> SendCommandAsync(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId)
        {
            var msg = new OutgoingMessage()
            {
                SourceAddress = AppServices.LocalAddress,
                DestinationAddress = drone.DroneAddress,
                ModuleType = moduleType,
                MessageId = requestMessageId,
                ExpectACK = true
            };

            return await SendMessageAsync(drone, msg);
        }


        public async Task<bool> SendMessageAsync(Drone drone, OutgoingMessage msg)
        {
            ushort serialNumber = AppServices.GetNextSerialNumber(msg.DestinationAddress.Value);
            msg.SerialNumber = serialNumber;

            if (!msg.SourceAddress.HasValue)
                msg.SourceAddress = AppServices.LocalAddress;

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

            if (msg.ExpectACK)
                AddMessageEntry(msg);

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
                    //Debug.WriteLine("Fire and forget, not waiting around for an ACK");
                }
                else
                {
                    var startTime = DateTime.Now;
                    if (!await messageReceived.WaitAsync(TimeSpan.FromMilliseconds(2000)))
                    {
                        var delta = DateTime.Now - startTime;
                        Debug.WriteLine("NOPE, NO SUCCESS :( REtry Count {0} {1}", attempts, delta.TotalSeconds);
                    }
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

        public void Parse(byte[] buffer, int start, int size)
        {
            try
            {
                if (_currentMessage == null)
                    _currentMessage = new IncomingMessage(); 
                
                for (var idx = 0; idx < size; idx++)
                    _parser.ParseCh(buffer[idx], ref _currentMessage);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debug.WriteLine(ex.StackTrace);

            }
        }


        public UInt32 ConnectionColor
        {
            get
            {
                switch(ConnectionState)
                {
                    case Common.ConnectionStates.Disconnecting:
                    case Common.ConnectionStates.Connecting:  return 0xFF808000;
                    case Common.ConnectionStates.Disconnected:  return 0xFF800000;
                    case Common.ConnectionStates.Connected:  return 0xFF008000;
                    default: return 0xFFFFFFFF;
                }               
            }
        }



        public void SendAck(int serialNumber)
        {

        }

        public void SendNak(int serialNumber, NiVek.Common.Comms.Common.ErrorCodes errorCode)
        {
            //Debug.WriteLine("Sending invalid message: " + errorCode);
        }
    }
}
