using LagoVista.Core.Commanding;
using LagoVista.Core.IOC;
using Newtonsoft.Json;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    [DataContract(Name = "drone")]
    public class Drone : ModelBase
    {
        private byte _droneAddress;
        private String _droneName;
        private IChannel _channel;

        //TODO: May want to add watchdog
        //private Watchdog _watchDog;


        public event EventHandler OnConnect;
        public event EventHandler<String> OnDisconnect;

        //TODO: May want to support on disconnecting event
        //public event EventHandler<String> OnDisconnecting;

        private ITimer _timer;

        private List<DroneComms> _commSettings;

        [JsonProperty]
        public List<DroneComms> CommSettings
        {
            get
            {
                if (_commSettings == null)
                    _commSettings = new List<DroneComms>();

                return _commSettings;
            }
        }

        public event EventHandler<IncomingMessage> MessageReceived;

        public async Task<bool> SendMessageAsync(OutgoingMessage message)
        {
            message.DestinationAddress = _droneAddress;
            return await _channel.SendMessageAsync(this, message);
        }

        public Drone()
        {
            DateEstablished = DateTime.Now;
            DroneName = "Notset";
            _commSettings = new List<DroneComms>();

            _status = new SystemStatus();
            _battery = new BatteryCondition();
            _targets = new Models.Targets();
            _gpsData = new Models.GPSData();
            _motors = new MotorStatus();
        }

        [JsonIgnore]
        public DateTime DateEstablished { get; private set; }

        public DateTime _lastContactDateStamp;
        [JsonIgnore]
        public DateTime LastContactDateStamp
        {
            get { return _lastContactDateStamp; }
            set
            {
                AppServices.UIThread.Invoke(() =>
                {
                    Set(ref _lastContactDateStamp, value);
                });
            }
        }


        public async Task Connect()
        {
            await Channel.ConnectAsync(CurrentComms.Address, CurrentComms.Port);
        }

        public void Disconnect(String reason)
        {
            Channel.Disconnect(reason);
        }

        [JsonIgnore]
        public IChannel Channel
        {
            get { return _channel; }
            set
            {
                _channel = value;
                _channel.MessageReady += _channel_MessageReady;
                _channel.OnConnect += _channel_OnConnect;
                _channel.OnDisconnect += _channel_OnDisconnect;
                _channel.OnDisconnecting +=_channel_OnDisconnecting;
            }
        }

        void _channel_OnDisconnect(object sender, String reason)
        {
            if (OnDisconnect != null)
                OnDisconnect(sender, reason);

            AppServices.UIThread.Invoke(() =>
            {
                Connected = false;
            });
        }

        void _channel_OnDisconnecting(object sender, String reason)
        {
            OnDisconnect?.Invoke(sender, reason);

            AppServices.UIThread.Invoke(() =>
            {
                Connected = false;
            });
        }

        void _channel_OnConnect(object sender, EventArgs e)
        {
            OnConnect?.Invoke(sender, e);

            //            Config = await GetAsync<GPIOConfig>(Common.ModuleTypes.GPIO, Modules.GPIOModule.CMD_ReadAllPIDConstants, IncomingMessage.GPIOPidConstants);
            AppServices.UIThread.Invoke(() =>
            {
                Connected = true;
            });
        }

        void _channel_MessageReady(object sender, IncomingMessage e)
        {
            MessageReceived?.Invoke(sender, e);

            HandleMessage(e);
        }

        [JsonProperty(PropertyName = "id")]
        public Guid Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "user_id")]
        public String UserId
        {
            get;
            set;
        }

        public enum ProductTypes
        {
            Micro,
            Mini,
            Full
        }

        [JsonProperty(PropertyName = "product_type")]
        public ProductTypes ProductType
        {
            get;
            set;
        }

        private Int32 _droneSN;
        [JsonProperty(PropertyName = "drone_sn")]
        public Int32 DroneSN
        {
            get { return _droneSN; }
            set { Set(ref _droneSN, value); }
        }

        [JsonProperty(PropertyName = "drone_address")]
        public byte DroneAddress
        {
            get { return _droneAddress; }
            set { Set(ref _droneAddress, value); }
        }

        public String DisplayName { get { return (_droneAddress + " " + _droneName).ToString(); } }

        [JsonProperty(PropertyName = "drone_name")]
        public String DroneName
        {
            get { return _droneName; }
            set
            {
                if (_droneName != value)
                {
                    _droneName = value;
                    AppServices.UIThread.Invoke(() =>
                     {
                         RaisePropertyChanged(() => DroneName);
                         RaisePropertyChanged(() => DisplayName);
                     });
                }
            }
        }

        #region Drone State Variables
        SensorUpdate _sensorData;
        [JsonIgnore]
        public SensorUpdate SensorData
        {
            get
            {
                if (_sensorData == null)
                    _sensorData = new SensorUpdate();
                return _sensorData;
            }
            set { Set(ref  _sensorData, value); }
        }

        private SystemStatus _status;
        [JsonIgnore]
        public SystemStatus Status
        {
            get
            {
                if (_status == null)
                    _status = new SystemStatus();
                return _status;
            }
            set { Set(ref _status, value); }
        }

        private Targets _targets;
        [JsonIgnore]
        public Targets Targets
        {
            get
            {
                if (_targets == null)
                    _targets = new Targets();
                return _targets;
            }
            set { Set(ref _targets, value); }
        }

        GPIOConfig _config;
        [JsonIgnore]
        public GPIOConfig Config
        {
            get { return _config; }
            set
            {
                AppServices.UIThread.Invoke(() =>
                {
                    Set(ref _config, value);
                    _config.Drone = this;
                });
            }
        }

        GPSData _gpsData = new GPSData();
        [JsonIgnore]
        public GPSData GPSData
        {
            get
            {
                if (_gpsData == null)
                    _gpsData = new GPSData() { ValidFix = false, Latitude = 35.0, Longitude = -80.0 };

                return _gpsData;
            }
            set { Set(ref _gpsData, value); }
        }

        BatteryCondition _battery = new BatteryCondition();
        [JsonIgnore]
        public BatteryCondition Battery
        {
            get
            {
                if (_battery == null)
                    _battery = new BatteryCondition();

                return _battery;
            }
            set { Set(ref _battery, value); }
        }

        MotorStatus _motors = new MotorStatus();
        [JsonIgnore]
        public MotorStatus Motors
        {
            get
            {
                if (_motors == null)
                    _motors = new MotorStatus();
                return _motors;
            }
            set { Set(ref _motors, value); }
        }

        [JsonIgnore]
        public bool IsCrossFrame
        {
            get { return _config.FrameConfig == (byte)GPIOConfig.FrameConfigTypes.Cross; }
        }

        [JsonIgnore]
        public bool IsXFrame
        {
            get { return _config.FrameConfig == (byte)GPIOConfig.FrameConfigTypes.X; }
        }
        #endregion

        #region Drone Control Variables
        [JsonIgnore]
        public byte ThrottleInput { get; set; }
        [JsonIgnore]
        public sbyte YawInput { get; set; }
        [JsonIgnore]
        public sbyte RollInput { get; set; }
        [JsonIgnore]
        public sbyte PitchInput { get; set; }


        private bool _isConnected = false;
        [JsonIgnore]
        public bool Connected
        {
            get { return _isConnected; }
            set{Set(ref _isConnected, value);}
        }

        private bool _isNotConnected = true;

        [JsonIgnore]
        public bool IsNotConencted
        {
            get { return _isNotConnected; }
            set { Set(ref _isNotConnected, value); }
        }

        #endregion

        [JsonProperty(PropertyName = "current_comms")]
        public DroneComms CurrentComms { get; set; }

        public void StartPingTimer()
        {
            if (_timer == null)
            {
                _timer = SLWIOC.Get<ITimerManager>().CreateTimer(TimeSpan.FromSeconds(0.1250));
                _timer.Elapsed += (sndr, args) =>
                {
                    Debug.WriteLine("x");
                    SendPing(NivekSystem.Ping);
                    if ((DateTime.Now - LastContactDateStamp).Seconds > 3)
                    {
                        if (Connected)
                        {
                            AppServices.UIThread.Invoke(() =>
                                {
                                    Connected = false;
                                    IsNotConencted = true;
                                });
                        }
                    }
                };
            }
            _timer.Start();
        }

        public void StopPingTimer()
        {
            if (_timer != null)
                _timer.Stop();
        }

        public override string ToString()
        {
            return (_droneAddress + " " + _droneName).ToString();
        }

        public void HandleMessage(IncomingMessage msg)
        {
            LastContactDateStamp = DateTime.Now;

            if (IsNotConencted)
            {
                AppServices.UIThread.Invoke(() =>
                {
                    Connected = true;
                    IsNotConencted = false;
                });
            }

            AppServices.UIThread.Invoke(() =>
            {
                try
                {
                    switch (msg.MessageId)
                    {
                        case IncomingMessage.SystemStatus:

                            RaisePropertyChanged(() => ArmedColor);
                            Status.Update(msg.Payload);
                            break;
                        case IncomingMessage.SensorGps:
                            GPSData.Update(msg.Payload);
                            RaisePropertyChanged(() => GPSData);
                            break;
                        case IncomingMessage.SensorBattery: Battery.Update(msg.Payload); break;
                        case IncomingMessage.MotorStatus: Motors.Update(msg.Payload); break;
                        case IncomingMessage.GPIOPidConstants:
                            Config = NiVek.Common.Models.GPIOConfig.Create(msg.Payload);
                            RaisePropertyChanged(() => IsCrossFrame);
                            RaisePropertyChanged(() => IsXFrame);
                            break;
                        case IncomingMessage.Targets: Targets.Update(msg.Payload); break;
                    }
                }
                catch (Exception)
                {
                    Debugger.Break();
                }
            });
        }

        public async void PopulateDroneName()
        {
            var buffer = await _channel.GetBufferAsync(this, NiVek.Common.Comms.Common.ModuleTypes.System, NivekSystem.GetName, NiVek.Common.Comms.IncomingMessage.NiVekName);
            if (buffer != null)
            {
                DroneName = System.Text.Encoding.UTF8.GetString(buffer, 0, buffer.Length).Replace((char)0x00, ' ').Trim();
                Debug.WriteLine("Drone connected {0} Name {1}", DroneAddress, DroneName);
            }
            else
            {
                DroneName = "??????";
                Debug.WriteLine("INVALID MESSAGE!");
            }
        }

        #region Sending messages to drone
        public void SendControlUpdate()
        {
            if (_channel != null)
            {
                var msg = new OutgoingMessage() { DestinationAddress = DroneAddress, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_ThrottleYawRollThrottle };
                msg.SourceAddress = AppServices.LocalAddress;
                msg.AddByte(ThrottleInput);
                msg.AddSByte(PitchInput);
                msg.AddSByte(RollInput);
                msg.AddSByte(YawInput);
                _channel.SendMessageAsync(this, msg);
            }
        }

        public void SendPing(byte pingId)
        {
            if (_channel != null)
            {
                var msg = new OutgoingMessage() { DestinationAddress = DroneAddress, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.System, MessageId = pingId };
                msg.ExpectACK = false;
                msg.SourceAddress = AppServices.LocalAddress;
                msg.AddByte(ThrottleInput);
                msg.AddSByte(PitchInput);
                msg.AddSByte(RollInput);
                msg.AddSByte(YawInput);

                _channel.SendMessageAsync(this, msg);
            }
        }

        public async void Safe()
        {
            ThrottleInput = 0;
            for (var idx = 0; idx < 5; idx++)
            {
                await SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_SafeNiVek, ExpectACK = true });
                await Task.Delay(20);
            }
        }

        public async void Kill()
        {
            ThrottleInput = 0;
            for (var idx = 0; idx < 5; idx++)
            {
                await SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_Kill, ExpectACK = true });
                await Task.Delay(20);
            }
        }

        public async void Arm()
        {
            await SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_ArmNiVek, ExpectACK = true });
        }

        public void Launch()
        {

        }

        public void Land()
        {

        }

        public async void SendName()
        {
            var msg = new NiVek.Common.OutgoingMessage
            {
                ExpectACK = true,
                ModuleType = NiVek.Common.Comms.Common.ModuleTypes.System,
                MessageId = NivekSystem.SetName
            };
            var nameBuffer = new Byte[NivekSystem.NiVekNameFixedSize];
            for (var idx = 0; idx < NivekSystem.NiVekNameFixedSize; idx++)
                nameBuffer[idx] = 0x00;

            msg.PayloadSize = NivekSystem.NiVekNameFixedSize;
            var encodedName = System.Text.Encoding.UTF8.GetBytes(DroneName);

            for (var idx = 0; idx < encodedName.Length; ++idx)
                nameBuffer[idx] = encodedName[idx];

            msg.Payload = nameBuffer;

            await SendMessageAsync(msg);
        }

        public async void ZeroSensors()
        {
            await SendMessageAsync(new OutgoingMessage() { ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor, MessageId = SensorModule.CMD_SensorZero, ExpectACK = true });
        }
        #endregion

        public void StartRecording()
        {

        }

        public void StopRecording()
        {

        }

        public void ToggleAltitudeHold()
        {

        }


        #region Throttle Commands
        public async void ThrottleDown()
        {
            if (Status.AltitudeHold == SystemStatus.AltitudeHoldEnum.On)
            {
                if (Status.GPIOState != SystemStatus.GPIOStates.Launching && Status.GPIOState != SystemStatus.GPIOStates.Landing)
                {
                    var msg = new OutgoingMessage() { DestinationAddress = DroneAddress, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_SetAltitude };

                    _targets.TargetAltitude -= 1;
                    msg.Add(_targets.TargetAltitude);
                    await SendMessageAsync(msg);
                }
            }
            else
            {
                if (ThrottleInput > 0)
                    ThrottleInput -= 2;
                else
                    ThrottleInput = 0;

                Debug.WriteLine("Throttle : " + ThrottleInput);

                SendControlUpdate();
            }
        }

        public async void ThrottleUp()
        {
            if (Status.IsArmed == SystemStatus.IsArmedStates.Armed)
            {
                if (Status.AltitudeHold == SystemStatus.AltitudeHoldEnum.On)
                {
                    if (Status.GPIOState != SystemStatus.GPIOStates.Launching && Status.GPIOState != SystemStatus.GPIOStates.Landing)
                    {
                        var msg = new OutgoingMessage() { DestinationAddress = DroneAddress, ModuleType = NiVek.Common.Comms.Common.ModuleTypes.GPIO, MessageId = GPIOModule.CMD_SetAltitude };

                        _targets.TargetAltitude += 1;
                        msg.Add(_targets.TargetAltitude);
                        await SendMessageAsync(msg);
                    }
                }
                else
                {
                    if (ThrottleInput < 255)
                        ThrottleInput += 2;
                    else
                        ThrottleInput = 255;

                    SendControlUpdate();
                }
            }
        }
        #endregion

        public async Task<T> GetAsync<T>(NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId, int responseMessageId) where T : IModel, new()
        {
            return await _channel.GetAsync<T>(this, moduleType, requestMessageId, responseMessageId);
        }

        public async Task<Boolean> SendCommandAsync(NiVek.Common.Comms.Common.ModuleTypes moduleType, byte requestMessageId)
        {
            return await _channel.SendCommandAsync(this, moduleType, requestMessageId);
        }

        [JsonIgnore]
        public UInt32 ArmedColor
        {
            get
            {
                if (Status.IsArmed == SystemStatus.IsArmedStates.Armed)
                    return 0xFF800000;
                else
                    return 0xFF008000;
            }
        }

        void ArmButtonClick()
        {
            if (Status.IsArmed == SystemStatus.IsArmedStates.Armed)
                Safe();
            else
                Arm();
        }

        public RelayCommand ArmCommand
        {
            get { return new RelayCommand(() => ArmButtonClick()); }
        }

    }
}
