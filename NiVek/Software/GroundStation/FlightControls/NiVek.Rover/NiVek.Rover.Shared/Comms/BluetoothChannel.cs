using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Popups;


namespace NiVek.Rover.Comms
{
    public class BluetoothChannel : Channel
    {
        public enum BluetoothConnectionState
        {
            Disconnected,
            Connected,
            Enumerating,
            Connecting,
            Closing

        }

        public delegate void AddOnExceptionOccuredDelegate(object sender, Exception ex);
        public event EventHandler<Exception> OnExceptionOccurred;
        private void OnExceptionOccuredEvent(object sender, Exception ex)
        {
            if (OnExceptionOccurred != null)
                OnExceptionOccurred(sender, ex);
        }
        //OnMessageReceived

        #region Fields
        public RelayCommand BluetoothCancelCommand { get; private set; }
        public RelayCommand BluetoothDisconnectCommand { get; private set; }

        private RfcommDeviceService _rfcommService;
        private StreamSocket _socket;
        private DataReader _reader;
        private DataWriter _writer;
        #endregion Fields

        private BluetoothConnectionState _state = BluetoothConnectionState.Disconnected;
        public BluetoothConnectionState State
        {
            get { return _state; }
            set { Set(ref _state, value); }
        }


        public async Task EnumerateDevicesAsync(Rect invokerRect)
        {
            this.State = BluetoothConnectionState.Enumerating;

            var serviceInfoCollection = await DeviceInformation.FindAllAsync(RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort));
            var menu = new PopupMenu();
            foreach (var serviceInfo in serviceInfoCollection)
                menu.Commands.Add(new UICommand(serviceInfo.Name, new UICommandInvokedHandler(delegate(IUICommand command) { Task connect = ConnectToServiceAsync(command); }), serviceInfo));

            var result = await menu.ShowForSelectionAsync(invokerRect);
            if (result == null)
                this.State = BluetoothConnectionState.Disconnected;
        }

        private async Task ConnectToServiceAsync(IUICommand command)
        {
            var serviceInfo = (DeviceInformation)command.Id;
            this.State = BluetoothConnectionState.Connecting;

            try
            {
                _rfcommService = await RfcommDeviceService.FromIdAsync(serviceInfo.Id);
                if (_rfcommService != null)
                {
                    _socket = new StreamSocket();
                    await _socket.ConnectAsync(_rfcommService.ConnectionHostName, _rfcommService.ConnectionServiceName, SocketProtectionLevel.BluetoothEncryptionAllowNullAuthentication);
                    _writer = new DataWriter(_socket.OutputStream);
                    _reader = new DataReader(_socket.InputStream) { InputStreamOptions = InputStreamOptions.Partial };

                    ListenForMessagesAsync();
                    this.State = BluetoothConnectionState.Connected;
                }
                else
                    OnExceptionOccuredEvent(this, new Exception("Unable to create service.\nMake sure that the 'bluetooth.rfcomm' capability is declared with a function of type 'name:serialPort' in Package.appxmanifest."));
            }
            catch (TaskCanceledException)
            {
                this.State = BluetoothConnectionState.Disconnected;
            }
            catch (Exception ex)
            {
                this.State = BluetoothConnectionState.Disconnected;
                OnExceptionOccuredEvent(this, ex);
            }
        }

        protected override async void Send(byte[] buffer, int offset, int len)
        {
            if (_writer != null)
            {
                _writer.WriteBuffer(buffer.AsBuffer(0, 0, len), 0, (uint)len);
                await _writer.StoreAsync();
            }

        }

        private async void ListenForMessagesAsync()
        {
            await Task.Run(async () =>
            {
                while (_reader != null)
                {
                    try
                    {
                        // Read first byte (length of the subsequent message, 255 or less). 
                        var sizeFieldCount = await _reader.LoadAsync(255);
                        var buffer = new byte[sizeFieldCount];
                        _reader.ReadBytes(buffer);
                        Parse(buffer, 0, (int)sizeFieldCount);

                    }
                    catch (Exception ex)
                    {
                        if (_reader != null)
                            OnExceptionOccuredEvent(this, ex);
                    }
                }
            });
        }


        public override void ConnectAsync(string address, short port)
        {

        }

        public override void Close()
        {
            Debug.WriteLine("Closing Connection");

            if (State != BluetoothConnectionState.Connected)
                return;

            State = BluetoothConnectionState.Closing;

            if (_reader != null)
            {
                _reader.Dispose();
                _reader = null;
            }

            if (_writer != null)
            {
                _writer.Dispose();
                _writer = null;
            }

            if (_socket != null)
            {
                _socket.Dispose();
                _socket = null;
            }

            State = BluetoothConnectionState.Disconnected;
        }
    }
}
