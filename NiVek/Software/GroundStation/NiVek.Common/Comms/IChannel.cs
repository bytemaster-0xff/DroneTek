using NiVek.Common.Models;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    public interface IChannel
    {
        event EventHandler OnConnect;
        event EventHandler OnConnecting;
        event EventHandler<String> OnDisconnect;
        event EventHandler<String> OnDisconnecting;
        event EventHandler<String> OnDidNotConnect;
        
        String Address { get; set; }
        short Port { get; set; }
        Double ErrorRate { get; }
        void ResetErrorCount();

        void IncrementErrorCount();
        void IncrementTotalCount();

        Task ConnectAsync(String address, short port);

        void Disconnect(String reason);

        Common.ConnectionStates ConnectionState { get; }

        ObservableCollection<MessageEntry> MessageLog { get; }

        event EventHandler<IncomingMessage> MessageReady;

        void Parse(byte[] buffer, int start, int length);

        void SendAck(int serialNumber);
        void SendNak(int serialNumber, NiVek.Common.Comms.Common.ErrorCodes errorCode);

        Task<T> GetAsync<T>(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId) where T : IModel, new();

        Task<byte[]> GetBufferAsync(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId);

        Task<bool> SendCommandAsync(Drone drone, NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId);

        Task<bool> SendMessageAsync(Drone drone, OutgoingMessage msg);
    }
}
