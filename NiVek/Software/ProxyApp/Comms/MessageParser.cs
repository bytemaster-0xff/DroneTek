using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProxyApp.Comms
{
    public class MessageReceivedEventArgs
    {
        public IncomingMessage Msg { get; set; }
    }
    

    public class MessageParser
    {
        public bool _droneHandler;
        public MessageParser(bool droneHandler)
        {
            _droneHandler = droneHandler;
        }

        public const byte SOH = 0x01;
        public const byte STX = 0x02;
        public const byte ETX = 0x03;
        public const byte EOT = 0x04;

        public const byte ACK = 0x06;
        public const byte NAK = 0x15;

        enum MessageStates
        {
            SOH,
            ExpectACK,
            SourceAddress,
            DestAddress,
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

        IncomingMessage _currentMessage = null;

        public event EventHandler<MessageReceivedEventArgs> MessageReceived;

        public event EventHandler<OutgoingMessage> MessageToBeSent;

        MessageStates _messageState = MessageStates.SOH;
        StringBuilder _textMessageBuilder = new StringBuilder();

        int _payloadBufferIndex;

        void SendNak(String error, IncomingMessage msg, bool transmit)
        {
            Debug.WriteLine("{0} Invalid Message: {1}", DateTime.Now, error);

            if (transmit) /* Note in some cases we don't send NAK if message is to garbled to get addresses */
            {
                var outgoingMessage = new OutgoingMessage() { ModuleType = SYSTEM_MODULE_ID, MessageId = ACK, DestinationAddress = msg.SourceAddress, SourceAddress = Common.LocalAddress };
                outgoingMessage.Add(msg.SerialNumber);
                //MessageToBeSent(this, outgoingMessage);
            }
        }

        const int SYSTEM_MODULE_ID = 100;

        void SendAck(IncomingMessage msg)
        {
            var outgoingMessage = new OutgoingMessage() { ModuleType =  SYSTEM_MODULE_ID, MessageId = ACK, DestinationAddress = msg.SourceAddress, SourceAddress = Common.LocalAddress };
            outgoingMessage.Add(msg.SerialNumber);
            //MessageToBeSent(this, outgoingMessage);
        }

        public void HandleMessage(IncomingMessage msg)
        {
            if(MessageReceived != null)
                MessageReceived(this, new MessageReceivedEventArgs() {Msg = msg});
        }

        public void ParseBuffer(byte[] buffer, int size)
        {
            Debug.WriteLine("Gonna parse {0} characters", size);

            for (var idx = 0; idx < size; ++idx)
                ParseCh(buffer[idx]);

            Debug.WriteLine("Did parse {0} characters", size);
        }

        public void ParseCh(byte ch)
        {
            switch (_messageState)
            {
                case MessageStates.SOH:
                    if (ch == MessageParser.SOH)
                    {
                        _currentMessage = new IncomingMessage();
                        _currentMessage.CalcCheckSum = 0;

                        if (_textMessageBuilder != null)
                        {
                     //       Console.WriteLine(_textMessageBuilder.ToString());
                            _textMessageBuilder = null;
                        }

                        _messageState = MessageStates.ExpectACK;
                    }
                    else
                    {                        
                        if (_textMessageBuilder == null)
                            _textMessageBuilder = new StringBuilder();

                        if (ch == '\r')
                        {
                            if (_textMessageBuilder != null)
                            {
                                //Console.WriteLine(_textMessageBuilder);
                                _textMessageBuilder = null;
                            }
                        }
                        else
                        {
                            _textMessageBuilder.Append((char)ch);
                            if (ch == '}')
                            {
                                //Console.WriteLine(_textMessageBuilder);
                                _textMessageBuilder = null;
                            }
                        }
                    }
                    break;

                case MessageStates.ExpectACK:
                    if (ch != 0x00 && ch != 0xFF)
                    {
                        /* Start of sequence is always a 1 (STX) followed by a 0x00 or 0xFF to denote ACK required or Not ACK required */
                        _messageState = MessageStates.SOH;
                        _currentMessage = null;
                        return;
                    }

                    _currentMessage.ExpectACK = ch != 0;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SourceAddress;
                    break;
                case MessageStates.SourceAddress:
                    _currentMessage.SourceAddress = ch;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.DestAddress;
                    break;
                case MessageStates.DestAddress:
                    _currentMessage.DestAddress = ch;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SystemID;
                    break;
                case MessageStates.SystemID:
                    _currentMessage.SystemId = ch;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.MsgID;
                    break;
                case MessageStates.MsgID:
                    _currentMessage.MessageId = ch;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SerialNumberMSB;
                    break;
                case MessageStates.SerialNumberMSB:
                    _currentMessage.SerialNumber = (UInt16)(ch << 8);
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SerialNumberLSB;
                    break;
                case MessageStates.SerialNumberLSB:
                    _currentMessage.SerialNumber |= ch;
                    _currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SizeMSB;
                    break;

                case MessageStates.SizeMSB:
                    _currentMessage.PayloadSize = (UInt16)((ch) << 8);
                    _messageState = MessageStates.SizeLSB;
                    break;
                case MessageStates.SizeLSB:
                    _currentMessage.PayloadSize |= ch;

                    //For now payloads bigger than 255 will be assumed to be an error.
                    if (_currentMessage.PayloadSize > 255)
                    {
                        _messageState = MessageStates.SOH;
                        SendNak("Payload size too large.", _currentMessage, false);
                        _currentMessage = null;                        
                    }
                    else
                        _messageState = MessageStates.STX;

                    break;
                case MessageStates.STX:
                    if (ch != MessageParser.STX)
                    {
                        SendNak("Missig expected STX", _currentMessage, false);
                        _currentMessage = null;
                        _messageState = MessageStates.SOH;
                    }
                    else
                    {
                        _currentMessage.Payload = new byte[_currentMessage.PayloadSize];
                        _payloadBufferIndex = 0;
                        if (_currentMessage.PayloadSize == 0)
                            _messageState = MessageStates.ETX;
                        else
                            _messageState = MessageStates.Payload;
                    }

                    break;


                case MessageStates.Payload:
                    _currentMessage.Payload[_payloadBufferIndex++] = ch;
                    _currentMessage.XORCheckSum(ch);

                    if (_payloadBufferIndex == _currentMessage.PayloadSize) {                        
                        _messageState = MessageStates.ETX;
                    }

                    break;

                case MessageStates.ETX:
                    if (ch != MessageParser.ETX)
                    {
                        _messageState = MessageStates.SOH;
                        SendNak("Missing expected ETX", _currentMessage, false);
                        _currentMessage = null;
                        
                    }
                    else
                        _messageState = MessageStates.CheckSum;

                    break;

                case MessageStates.CheckSum:
                    _currentMessage.TxChecksum = ch;
                    _messageState = MessageStates.EOT;
                    break;

                case MessageStates.EOT:
                    if (ch == MessageParser.EOT)
                    {
                        if (_currentMessage.ChecksumValid)
                        {
                            HandleMessage(_currentMessage);
                            if (_currentMessage.ExpectACK && _currentMessage.DestAddress == Common.LocalAddress)
                                SendAck(_currentMessage);
                        }
                        else
                            Debug.WriteLine("INVALID CHECK SUM {0} - {1}", _currentMessage.CalcCheckSum, _currentMessage.TxChecksum);
                    }
                    else
                        SendNak("Missing expected EOT", _currentMessage, false);

                    _currentMessage = null;

                    _messageState = MessageStates.SOH;
                    break;
            }
        }
    }
}
