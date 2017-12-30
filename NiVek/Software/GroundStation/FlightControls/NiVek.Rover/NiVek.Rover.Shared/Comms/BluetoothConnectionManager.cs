using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Popups;

namespace NiVek.Rover.Comms
{
    public class BluetoothConnectionManager : PropertyChangedBase
    {
        #region Events
        //OnExceptionOccured
        public delegate void AddOnExceptionOccuredDelegate(object sender, Exception ex);
        public event AddOnExceptionOccuredDelegate ExceptionOccured;
        private void OnExceptionOccuredEvent(object sender, Exception ex)
        {
            if (ExceptionOccured != null)
                ExceptionOccured(sender, ex);
        }
        //OnMessageReceived
        public delegate void AddOnMessageReceivedDelegate(object sender, string message);
        public event AddOnMessageReceivedDelegate MessageReceived;
        private void OnMessageReceivedEvent(object sender, string message)
        {
            if (MessageReceived != null)
                MessageReceived(sender, message);
        }
        #endregion

        #region Commands
        public RelayCommand BluetoothCancelCommand { get; private set; }
        public RelayCommand BluetoothDisconnectCommand { get; private set; }
        #endregion

        #region Variables
        private IAsyncOperation<RfcommDeviceService> _connectService;
        private IAsyncAction _connectAction;
        private RfcommDeviceService _rfcommService;
        private StreamSocket _socket;
        private DataReader _reader;
        private DataWriter _writer;

        private BluetoothConnectionState _State;
        public BluetoothConnectionState State { get { return _State; } set { _State = value; OnPropertyChanged(); } }
        #endregion

        #region Lifecycle
        public BluetoothConnectionManager()
        {
            BluetoothCancelCommand = new RelayCommand(AbortConnection);
            BluetoothDisconnectCommand = new RelayCommand(Disconnect);
        }

        /// <summary>
        /// Displays a PopupMenu for selection of the other Bluetooth device.
        /// Continues by establishing a connection to the selected device.
        /// </summary>
        /// <param name="invokerRect">for example: connectButton.GetElementRect();</param>
        public async Task EnumerateDevicesAsync(Rect invokerRect)
        {
            this.State = BluetoothConnectionState.Enumerating;

            var serviceInfoCollection = await DeviceInformation.FindAllAsync(RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort));
            PopupMenu menu = new PopupMenu();
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
                // Initialize the target Bluetooth RFCOMM device service
                _connectService = RfcommDeviceService.FromIdAsync(serviceInfo.Id);
                _rfcommService = await _connectService;
                if (_rfcommService != null)
                {
                    // Create a socket and connect to the target 
                    _socket = new StreamSocket();
                    _connectAction = _socket.ConnectAsync(_rfcommService.ConnectionHostName, _rfcommService.ConnectionServiceName, SocketProtectionLevel.BluetoothEncryptionAllowNullAuthentication);
                    await _connectAction;//to make it cancellable
                    _writer = new DataWriter(_socket.OutputStream);
                    _reader = new DataReader(_socket.InputStream) { InputStreamOptions = InputStreamOptions.Partial };


                    Task listen = ListenForMessagesAsync();
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

        /// <summary>
        /// Abort the connection attempt.
        /// </summary>
        public void AbortConnection()
        {
            if (_connectService != null && _connectService.Status == AsyncStatus.Started)
                _connectService.Cancel();

            if (_connectAction != null && _connectAction.Status == AsyncStatus.Started)
                _connectAction.Cancel();
        }
        /// <summary>
        /// Terminate an connection.
        /// </summary>
        public void Disconnect()
        {
            if (_reader != null)
                _reader = null;

            if (_writer != null)
            {
                _writer.DetachStream();
                _writer = null;
            }
            if (_socket != null)
            {
                _socket.Dispose();
                _socket = null;
            }
            if (_rfcommService != null)
                _rfcommService = null;

            this.State = BluetoothConnectionState.Disconnected;
        }
        #endregion

        #region Send & Receive
        public async Task<uint> SendMessageAsync(string message)
        {
            uint sentMessageSize = 0;
            if (_writer != null)
            {
                sentMessageSize = _writer.WriteString(message);
                await _writer.StoreAsync();
            }
            return sentMessageSize;
        }

        private async Task ListenForMessagesAsync()
        {
            while (_reader != null)
            {
                try
                {
                    // Read first byte (length of the subsequent message, 255 or less). 
                    var sizeFieldCount = await _reader.LoadAsync(255);
                    var buffer = new byte[sizeFieldCount];
                    _reader.ReadBytes(buffer);

                    Debug.WriteLine("RECIEVED {0} BYTES", sizeFieldCount) ;
                    
                }
                catch (Exception ex)
                {
                    if (_reader != null)
                        OnExceptionOccuredEvent(this, ex);
                }
            }
        }
        #endregion
    }
    public enum BluetoothConnectionState
    {
        Disconnected,
        Connected,
        Enumerating,
        Connecting
    }
}