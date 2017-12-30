using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SPProxy.Commo
{
    public class MessageReceivedEventArgs
    {
        public IncomingMessage Msg { get; set; }
    }
    

    public class MessageParser
    {
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

        public event EventHandler<MessageReceivedEventArgs> MessageReceived;

        MessageStates _messageState = MessageStates.SOH;
        StringBuilder _textMessageBuilder;

        int _payloadBufferIndex;

        void SendNak(String error)
        {
            Console.WriteLine("{0}Invalid Message: {1}", DateTime.Now, error);
        }

        void SendAck(UInt16 serialNumber)
        {

        }

        public void HandleMessage(IncomingMessage msg)
        {
            MessageReceived(this, new MessageReceivedEventArgs() {Msg = msg});
        }

        public void ParseCh(byte ch, ref IncomingMessage currentMessage)
        {
            switch (_messageState)
            {
                case MessageStates.SOH:
                    if (ch == MessageParser.SOH)
                    {
                        currentMessage = new IncomingMessage();

                        if (_textMessageBuilder != null)
                        {
                            Console.WriteLine(_textMessageBuilder.ToString());
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
                                Console.WriteLine(_textMessageBuilder);
                                _textMessageBuilder = null;
                            }
                        }
                        _textMessageBuilder.Append((char)ch);
                        if (ch == '}')
                        {
                            Console.WriteLine(_textMessageBuilder);
                            _textMessageBuilder = null;
                        }
                    }

                    break;

                case MessageStates.ExpectACK:
                    currentMessage.ExpectACK = ch != 0;
                    currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SourceAddress;
                    break;
                case MessageStates.SourceAddress:
                    currentMessage.SourceAddress = ch;
                    currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.DestAddress;
                    break;
                case MessageStates.DestAddress:
                    currentMessage.DestAddress = ch;
                    currentMessage.XORCheckSum(ch);
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
                    }
                    else
                        _messageState = MessageStates.STX;

                    break;
                case MessageStates.STX:
                    if (ch != MessageParser.STX)
                    {
                        SendNak("Missig expected STX");
                        currentMessage = null;
                        _messageState = MessageStates.SOH;
                    }
                    else
                    {
                        currentMessage.Payload = new byte[currentMessage.PayloadSize];
                    }
                    _payloadBufferIndex = 0;
                    if (currentMessage.PayloadSize == 0)
                        _messageState = MessageStates.ETX;
                    else
                        _messageState = MessageStates.Payload;

                    break;


                case MessageStates.Payload:
                    currentMessage.Payload[_payloadBufferIndex++] = ch;

                    if (_payloadBufferIndex == currentMessage.PayloadSize)
                        _messageState = MessageStates.ETX;

                    break;

                case MessageStates.ETX:
                    if (ch != MessageParser.ETX)
                    {
                        _messageState = MessageStates.SOH;
                        currentMessage = null;
                        SendNak("Missing expected ETX");
                    }
                    else
                        _messageState = MessageStates.CheckSum;

                    break;

                case MessageStates.CheckSum:
                    _messageState = MessageStates.EOT;
                    break;

                case MessageStates.EOT:
                    if (ch == MessageParser.EOT)
                        HandleMessage(currentMessage);

                    SendAck(currentMessage.SerialNumber);

                    currentMessage = null;

                    _messageState = MessageStates.SOH;
                    break;
            }
        }
    }
}
