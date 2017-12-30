using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common
{
    public class MessageParser
    {
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

        public event EventHandler<IncomingMessage> MessageReady;

        MessageStates _messageState = MessageStates.SOH;
        IChannel _channel;
        private StringBuilder _textMessageBuilder = new StringBuilder();

        private int _errorMessages;
        private int _successMessageCount;
        private int _totalMessageCount;

        private int _payloadBufferIndex;

        public MessageParser(IChannel channel)
        {
            _channel = channel;
        }

        public void ParseCh(byte ch, ref IncomingMessage currentMessage)
        {
            switch (_messageState)
            {
                case MessageStates.SOH:
                    if (ch == Common.Comms.Common.SOH)
                    {
                        currentMessage = new IncomingMessage();

                        _channel.IncrementTotalCount();

                        if (_textMessageBuilder != null)
                        {
                            Debug.WriteLine(_textMessageBuilder.ToString());
                            _textMessageBuilder = null;
                        }
                        _messageState = MessageStates.ExpectACK;
                    }

                    break;

                case MessageStates.ExpectACK:
                    currentMessage.ClearCheckSum();
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
                    currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.MsgID;
                    break;
                case MessageStates.MsgID:
                    currentMessage.MessageId = ch;
                    currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SerialNumberMSB;
                    break;
                case MessageStates.SerialNumberMSB:
                    currentMessage.SerialNumber = (UInt16)(ch << 8);
                    currentMessage.XORCheckSum(ch);
                    _messageState = MessageStates.SerialNumberLSB;
                    break;
                case MessageStates.SerialNumberLSB:
                    currentMessage.SerialNumber |= ch;
                    currentMessage.XORCheckSum(ch);
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
                        _channel.SendNak(currentMessage.SerialNumber, NiVek.Common.Comms.Common.ErrorCodes.MessageTooLarge);
                        currentMessage = null;
                        _channel.IncrementErrorCount();
                        _errorMessages++;
                    }
                    else
                        _messageState = MessageStates.STX;

                    break;
                case MessageStates.STX:
                    if (ch != Common.Comms.Common.STX)
                    {
                        _channel.SendNak(currentMessage.SerialNumber, NiVek.Common.Comms.Common.ErrorCodes.MissingETX);
                        currentMessage = null;
                        _messageState = MessageStates.SOH;
                        _channel.IncrementErrorCount();
                        _errorMessages++;
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
                    currentMessage.XORCheckSum(ch);

                    if (_payloadBufferIndex == currentMessage.PayloadSize)
                        _messageState = MessageStates.ETX;

                    break;

                case MessageStates.ETX:
                    if (ch != Common.Comms.Common.ETX)
                    {
                        _messageState = MessageStates.SOH;
                        _channel.IncrementErrorCount();
                        _channel.SendNak(currentMessage.SerialNumber, NiVek.Common.Comms.Common.ErrorCodes.MissingETX);
                        _errorMessages++;
                        currentMessage = null;
                    }
                    else
                        _messageState = MessageStates.CheckSum;

                    break;

                case MessageStates.CheckSum:
                    currentMessage.TxChecksum = ch;

                    _messageState = MessageStates.EOT;
                    break;

                case MessageStates.EOT:
                    _totalMessageCount++;
                    

                    if (ch == Common.Comms.Common.EOT && currentMessage.ChecksumValid)
                    {
                        _successMessageCount++;

                        var percentError = _errorMessages * 100.0f / _totalMessageCount * 1.0f;                        

                        currentMessage.ErrorMessageCount = _errorMessages;
                        currentMessage.TotalMessageCount = _successMessageCount;
                        currentMessage.ErrorRate = percentError;
                        _channel.IncrementTotalCount();
                        MessageReady(this, currentMessage);
                        if(currentMessage.ExpectACK)
                            _channel.SendAck(currentMessage.SerialNumber);
                    }
                    else
                    {
                        _errorMessages++;

                        _channel.IncrementErrorCount();

                        if(!currentMessage.ChecksumValid)
                        {
                            if(currentMessage.ExpectACK)
                                _channel.SendNak(currentMessage.SerialNumber, NiVek.Common.Comms.Common.ErrorCodes.InvalidCRC);

                            Debug.WriteLine("INVALID Check Sum {0} - Calc - {1}", currentMessage.TxChecksum, currentMessage.CalcCheckSum);
                        }                        
                        else{
                            if(currentMessage.ExpectACK)
                                _channel.SendNak(currentMessage.SerialNumber, NiVek.Common.Comms.Common.ErrorCodes.MissingEOT);
                        }
                    }

                    currentMessage = null;

                    _messageState = MessageStates.SOH;
                    break;
            }
        }
    
    }
}
