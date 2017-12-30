using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Core;


#if NETFX_CORE
using Windows.UI.Xaml;
using System.Collections.ObjectModel;
#else  //WP8 
using NiVek.WP8;
using System.Windows.Threading;
#endif

namespace NiVek.FlightControls.Commo
{
    public class NiVekConsole
    {
        #region Constants and Defines
        public const byte SOH = 0x01;
        public const byte STX = 0x02;
        public const byte ETX = 0x03;
        public const byte EOT = 0x04;

        public const byte ACK = 0x06;
        public const byte NAK = 0x15;

        ObservableCollection<MessageEntry> _messageLog;
        public ObservableCollection<MessageEntry> MessageLog
        {
            get { return _messageLog; }
        }

        public enum ConnectionStates
        {
            Connecting,
            ConnectionEstablished,
            Connected,
            Disconnecting,
            Disconnected
        }

        enum MessageStates
        {
            SOH,
            ExpectACK,
            SystemID,
            MsgID,
            SerialNumberMSB,
            SerialNumberLSB,
            SizeMSB,
            SizeLSB,
            STX,
            Payload,
            ETX,
            CheckSum,
            EOT
        }
        #endregion

        #region Fields
        StreamSocket _socket;
        DataReader _reader;
        DataWriter _writer;

        DateTime _lastUpdate;

        ConnectionStates _connectionState = ConnectionStates.Disconnected;

        DispatcherTimer _commoTimeout;

        StringBuilder _stringBuffer = new StringBuilder();
        MessageStates _messageState = MessageStates.SOH;

        UInt16 _payloadBufferIndex;
        byte[] _msgBuffer;

        StringBuilder _textMessageBuilder;
        #endregion

        #region Properties
        public ConnectionStates ConnectionStatus { get { return _connectionState; } }
        public bool IsAdhoc { get; set; }
        public bool IsConnectionOn { get; set; }

        public byte Throttle { get; set; }
        public sbyte Yaw { get; set; }
        public sbyte Roll { get; set; }
        public sbyte Pitch { get; set; }
        #endregion

        public void SendControlUpdate()
        {
            var msg = new OutgoingMessage() { ModuleType = OutgoingMessage.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_ThrottleYawRollThrottle };
            msg.AddByte(App.Commo.Throttle);
            msg.AddSByte(App.Commo.Pitch);
            msg.AddSByte(App.Commo.Roll);
            msg.AddSByte(App.Commo.Yaw);
            App.Commo.Send(msg);
        }

        #region Event Handlers
        public event EventHandler<Commo.IncomingMessage> MessageReceived;

        public event EventHandler Connected;
        public event EventHandler ConnectionEstablished;
        public event EventHandler Connecting;
        public event EventHandler Disconnected;
        public event EventHandler FailedToConnect;
        #endregion

        public NiVekConsole()
        {
            _commoTimeout = new DispatcherTimer();
            _commoTimeout.Interval = TimeSpan.FromMilliseconds(250);
            _commoTimeout.Tick += _commoTimeout_Tick;
            _commoTimeout.Stop();

            _messageLog = new ObservableCollection<Commo.MessageEntry>();

            _lastUpdate = DateTime.MinValue;

            _connectionState = ConnectionStates.Disconnected;

            IsConnectionOn = true;

            this.Pitch = 0;
            this.Roll = 0;
            this.Throttle = 0;
            this.Yaw = 0;
        }

        

        #region Watch Dog
        async void _commoTimeout_Tick(object sender, object e)
        {
            Debug.WriteLine("COMMO TICKET" + DateTime.Now);

            if (_connectionState == ConnectionStates.Disconnected)
                await Connect();
            else if (_connectionState == ConnectionStates.ConnectionEstablished)
            {

                var result = SendPing("Welcome Ping", NiVekSys.WelcomePing);
                if (!result)
                {
                    Debug.WriteLine("Failed sending ping.");

                    if (Disconnected != null)
                        Disconnected(this, null);

                    Close();

                    _connectionState = ConnectionStates.Disconnected;
                }
                else
                {
                    _lastUpdate = DateTime.Now;
                    _connectionState = ConnectionStates.Connected;
                    StartWatchDog("Start Watchdog", TimeSpan.FromSeconds(1));
                }
            }
            else
            {
                if (_lastUpdate == DateTime.MinValue)
                    return;

                var deltaT = DateTime.Now - _lastUpdate;

                if (deltaT.TotalSeconds > 5)
                {
                    if (_connectionState == ConnectionStates.Connected)
                    {
                        Debug.WriteLine("{0} Commo Watchdog Timeout", DateTime.Now);

                        _commoTimeout.Stop();

                        Close();

                        _connectionState = ConnectionStates.Disconnected;

                        if (Disconnected != null)
                            Disconnected(this, null);
                    }
                }
                else
                {
                    var result = SendPing("Watch Dog", NiVekSys.Ping);
                    if (!result)
                    {
                        Debug.WriteLine("Failed sending ping.");

                        if (Disconnected != null)
                            Disconnected(this, null);

                        Close();

                        _connectionState = ConnectionStates.Disconnected;
                    }
                }
            }
        }
        #endregion

        void HandleMessageType(IncomingMessage msg)
        {
            if (msg.MessageId == IncomingMessageTypeId.Ping &&
                ((OutgoingMessage.ModuleTypes)msg.SystemId == OutgoingMessage.ModuleTypes.System)
                || (OutgoingMessage.ModuleTypes)msg.SystemId == OutgoingMessage.ModuleTypes.Proxy)
            {
                _lastUpdate = DateTime.Now;

                Debug.WriteLine("{0} Ping Received.", DateTime.Now);
            }

            if (_connectionState == ConnectionStates.ConnectionEstablished || _connectionState == ConnectionStates.Connecting)
            {
                _connectionState = ConnectionStates.Connected;
                Connected(this, null);
            }

            MessageReceived(this, msg);
        }

        void SendNak(String error)
        {
            //Debug.WriteLine("{0}Invalid Message: {1}", DateTime.Now, error);
        }

        void SendAck(UInt16 serialNumber)
        {

        }

        int successMessage = 0;
        int errorMessages = 0;

        #region Parse Characters
        void ParseCh(byte ch, ref IncomingMessage currentMessage)
        {
            switch (_messageState)
            {
                case MessageStates.SOH:
                    if (ch == NiVekConsole.SOH)
                    {
                        currentMessage = new IncomingMessage();

                        if (_textMessageBuilder != null)
                        {
                            Debug.WriteLine(_textMessageBuilder.ToString());
                            _textMessageBuilder = null;
                        }

                        _messageState = MessageStates.ExpectACK;
                    }

                    break;

                case MessageStates.ExpectACK:
                    currentMessage.ExpectACK = ch != 0;
                    _messageState = MessageStates.SystemID;
                    break;
                case MessageStates.SystemID:
                    currentMessage.SystemId = ch;
                    _messageState = MessageStates.MsgID;
                    break;
                case MessageStates.MsgID:
                    currentMessage.MessageId = ch;
                    _messageState = MessageStates.SerialNumberMSB;
                    break;
                case MessageStates.SerialNumberMSB:
                    currentMessage.SerialNumber = (UInt16)(ch << 8);
                    _messageState = MessageStates.SerialNumberLSB;
                    break;
                case MessageStates.SerialNumberLSB:
                    currentMessage.SerialNumber |= ch;
                    _messageState = MessageStates.SizeMSB;
                    break;

                case MessageStates.SizeMSB:
                    currentMessage.PayloadSize = (UInt16)((ch) << 8);
                    _messageState = MessageStates.SizeLSB;
                    break;
                case MessageStates.SizeLSB:
                    currentMessage.PayloadSize |= ch;

                    //For now payloads bigger than 255 will be assumed to be an error.
                    if (currentMessage.PayloadSize > 255)
                    {
                        _messageState = MessageStates.SOH;
                        currentMessage = null;
                        SendNak("Payload size too large.");
                        errorMessages++;
                    }
                    else
                        _messageState = MessageStates.STX;

                    break;
                case MessageStates.STX:
                    if (ch != NiVekConsole.STX)
                    {
                        SendNak("Missing expected STX");
                        currentMessage = null;
                        _messageState = MessageStates.SOH;
                        errorMessages++;
                    }
                    else
                    {
                        currentMessage.Payload = new byte[currentMessage.PayloadSize];
                        _payloadBufferIndex = 0;
                        if (currentMessage.PayloadSize == 0)
                            _messageState = MessageStates.ETX;
                        else
                            _messageState = MessageStates.Payload;
                    }

                    break;


                case MessageStates.Payload:
                    currentMessage.Payload[_payloadBufferIndex++] = ch;

                    if (_payloadBufferIndex == currentMessage.PayloadSize)
                        _messageState = MessageStates.ETX;

                    break;

                case MessageStates.ETX:
                    if (ch != NiVekConsole.ETX)
                    {
                        _messageState = MessageStates.SOH;
                        currentMessage = null;
                        SendNak("Missing expected ETX");
                        errorMessages++;
                    }
                    else
                        _messageState = MessageStates.CheckSum;

                    break;

                case MessageStates.CheckSum:
                    _messageState = MessageStates.EOT;
                    break;

                case MessageStates.EOT:
                    successMessage++;

                    if (ch == NiVekConsole.EOT)
                    {
                        var percentError = errorMessages * 100.0f / successMessage * 1.0f;
                        if (successMessage % 50 == 0)
                        {
                            //Debug.WriteLine("Total {0} Errors {1}  {2:0.000}% ", successMessage, errorMessages, percentError);
                        }

                        currentMessage.ErrorMessageCount = errorMessages;
                        currentMessage.TotalMessageCount = successMessage;
                        currentMessage.ErrorRate = percentError;

                        HandleMessageType(currentMessage);
                        SendAck(currentMessage.SerialNumber);
                    }
                    else
                        errorMessages++;

                    currentMessage = null;

                    _messageState = MessageStates.SOH;
                    break;
            }
        }
        #endregion

        public async void Listen()
        {
            _stringBuffer.Clear();
            _lastUpdate = DateTime.Now;

            IncomingMessage currentMessage = null;
            Debug.WriteLine("{0} Beginning Listening", DateTime.Now + " " + _connectionState.ToString());

            while (_connectionState == ConnectionStates.Connected || _connectionState == ConnectionStates.ConnectionEstablished)
            {
                long msgBytes = 0;
                try
                {
                    msgBytes = await _reader.LoadAsync(128);
                    if (msgBytes == -1)
                    {
                        Debug.WriteLine("{0} Message bytes is -1 after _reader.LoadAsync, disconnecting", DateTime.Now);
                        _connectionState = ConnectionStates.Disconnected;
                    }
                    else
                    {

                        //Debug.WriteLine("{0} Read {1} characters", DateTime.Now, msgBytes);
                        _msgBuffer = new byte[msgBytes];
                        _reader.ReadBytes(_msgBuffer);

                        _lastUpdate = DateTime.Now;

                        for (var idx = 0; idx < msgBytes; ++idx)
                            ParseCh(_msgBuffer[idx], ref currentMessage);
                    }


                }
                catch (InvalidOperationException ex)
                {
                    _connectionState = ConnectionStates.Disconnected;
                    Debug.WriteLine("{0} Invalid operation exception, this is probably because the connection dropped.\r\n{1}\r\n{2}", DateTime.Now, ex.Message, ex.StackTrace);

                }
                catch (Exception ex)
                {
                    if (msgBytes > 0)
                        Debug.WriteLine("{0} - {2}\r\nError parsing {1} bytes into a message\r\nDisconnecting now.\r\n{3}", DateTime.Now, msgBytes, ex.Message, ex.StackTrace);
                    else
                        Debug.WriteLine("{0} - {1}\r\nDisconnecting Now.", DateTime.Now, ex.Message);

                    _messageState = MessageStates.SOH;

                    /* For now, don't disconnect 
                     * _connectionState = ConnectionStates.Disconnected; */
                }
            }

            if (Disconnected != null)
                Disconnected(this, null);
        }

        public void StartWatchDog(String msg, TimeSpan interval)
        {
            Debug.WriteLine("{0} Starting Watchdog: {1}", DateTime.Now, msg);
            _commoTimeout.Interval = interval;
            _commoTimeout.Start();

        }

        public void StopWatchDog()
        {
            _commoTimeout.Stop();
            Debug.WriteLine("{0} Stopping Watchdog.", DateTime.Now);
        }

        public void ConnectAfterDelay(TimeSpan ts)
        {
            Debug.WriteLine("{0} Attempting to reconnect after {1}ms", DateTime.Now, ts.TotalMilliseconds);
            StopWatchDog();
            StartWatchDog("Reconnecting", ts);
        }

        DatagramSocket _incomingSocket;


        

        Models.SensorUpdate Parse(String json)
        {
            json = json.Replace('\'', '"');
            var serializer = new System.Runtime.Serialization.Json.DataContractJsonSerializer(typeof(Models.SensorUpdate));
            using (var ms = new MemoryStream(System.Text.UTF8Encoding.UTF8.GetBytes(json)))
            {
                try
                {
                    return serializer.ReadObject(ms) as Models.SensorUpdate;
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }

        public void Close()
        {
            StopWatchDog();

            Debug.WriteLine("{0} Closing Socket", DateTime.Now);

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

                Debug.WriteLine("{0} Socket Closed", DateTime.Now);
                Debug.WriteLine("--------------------------------------");
                Debug.WriteLine("");
            }
            catch (Exception ex)
            {
                Debug.WriteLine("{0} Exception closing socket {1}", ex.Message);
            }


            _connectionState = ConnectionStates.Disconnected;
        }

        UInt16 _serialNumber = 0;

        public async Task<bool> Send(OutgoingMessage msg)
        {
            lock (this)
            {
                msg.SerialNumber = _serialNumber++;

                if (Window.Current != null)
                {
                    if (Window.Current.Dispatcher.HasThreadAccess)
                    {
                        MessageLog.Add(new MessageEntry()
                        {
                            MessageTypeId = msg.MessageId,
                            ModuleType = msg.ModuleType,
                            SerialNumber = msg.SerialNumber,
                            AckStatus = msg.ExpectACK ? MessageEntry.AckStatusTypes.Pending : MessageEntry.AckStatusTypes.NA
                        });
                    }
                    else
                    {
                        Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                        {
                            MessageLog.Add(new MessageEntry()
                            {
                                MessageTypeId = msg.MessageId,
                                ModuleType = msg.ModuleType,
                                SerialNumber = msg.SerialNumber,
                                AckStatus = msg.ExpectACK ? MessageEntry.AckStatusTypes.Pending : MessageEntry.AckStatusTypes.NA
                            });
                        });
                    }
                }
            }
            //byte[] ByteArray = ...
            //IBuffer buffer = ByteArray.AsBuffer();
            //msg.Buffer.AsBuffer();

            if (_connectionState == ConnectionStates.Connected || _connectionState == ConnectionStates.ConnectionEstablished)
            {
                try
                {
                    //byte[] ByteArray = ...
                    //IBuffer buffer = ByteArray.AsBuffer();
                    //msg.Buffer.AsBuffer();

                    //await _outputStream.WriteAsync(msg.Buffer.AsBuffer());

                    //_writer.WriteBytes(msg.Buffer);
                    //_writer.StoreAsync();
                    return true;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("{0} Error sending message {1}", DateTime.Now, ex);

                    return false;
                }
            }
            else
            {
                Debug.WriteLine("{0} Attempt to send a message when not connected", DateTime.Now);
                return false;
            }
        }

    }
}
