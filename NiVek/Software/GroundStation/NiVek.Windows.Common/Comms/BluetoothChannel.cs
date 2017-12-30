using GalaSoft.MvvmLight.Command;
using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.UI.Popups;
using System.Diagnostics;

namespace NiVek.WinCommon.Comms
{
    public class BluetoothChannel : Channel
    {

        #region Fields
        private RfcommDeviceService _rfcommService;
        private StreamSocket _socket;
        private DataReader _reader;
        private DataWriter _writer;
        #endregion Fields

        public async override Task ConnectAsync(string name, short port)
        {
            Connecting();

            var serviceInfoCollection = await DeviceInformation.FindAllAsync(RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort));
            var serviceInfo = serviceInfoCollection.Where(srvc => srvc.Name == name).FirstOrDefault();
            if (serviceInfo == null)
            {
                Debug.WriteLine("Cound not find device with: " + name);
                DidNotConnect("Could not find BlueTooth Device: " + name);
            }
            else
                ConnectToServiceAsync(serviceInfo);
        }

        private async void ConnectToServiceAsync(DeviceInformation serviceInfo)
        {
            Debug.WriteLine("Attempting to connect to: " + serviceInfo.Name + " and get service: " + serviceInfo.Id);
            try
            {
                _rfcommService = await RfcommDeviceService.FromIdAsync(serviceInfo.Id);
                if (_rfcommService != null)
                {
                    Debug.WriteLine("Got RF Comm continuing on..");
                    _socket = new StreamSocket();
                    Debug.WriteLine("Got Socket!");
                    await _socket.ConnectAsync(_rfcommService.ConnectionHostName, _rfcommService.ConnectionServiceName, SocketProtectionLevel.BluetoothEncryptionAllowNullAuthentication);
                    Debug.WriteLine("Attempt to Connect!");
                    _writer = new DataWriter(_socket.OutputStream);
                    Debug.WriteLine("Serial");
                    _reader = new DataReader(_socket.InputStream) { InputStreamOptions = InputStreamOptions.Partial };
                    Debug.WriteLine("Reader");
                    ListenForMessagesAsync();
                    Debug.WriteLine("Listening");
                    Connected();
                    Debug.WriteLine("Connected and listenting for messages: " + serviceInfo.Name);
                }
                else
                {
                    Debug.WriteLine("Could not get RF Comm Service.");
                    DidNotConnect("Could not get Service for: " + serviceInfo.Name);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Did not connect to: " + serviceInfo.Name + " " + ex.Message);
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
                        /* Not really sure what to do here, other watchdogs will pickup disconnect events. */
                    }
                }
            });
        }

        public override void Disconnect(String reason = "")
        {
            if (ConnectionState == Common.Comms.Common.ConnectionStates.Connected)
            {
                Disconnecting(String.IsNullOrEmpty(reason) ? "Connection Closed" : reason);

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

                Disconnected(String.IsNullOrEmpty(reason) ? "Connection Closed" : reason);
            }
        }
    }
}