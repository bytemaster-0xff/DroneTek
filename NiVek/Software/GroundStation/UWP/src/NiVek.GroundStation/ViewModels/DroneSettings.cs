using LagoVista.Core.Commanding;
using NiVek.Common.Comms;
using NiVek.Common.Services;
using NiVek.FlightControls.Commo;
using NiVek.WinCommon.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.ViewModels
{
    public class DroneSettings : ViewModelBase
    {
        IStorageContext _storageContext;
        public event EventHandler<List<String>> SendCommands;
        private bool _isEditing;

        public DroneSettings(IStorageContext storageContext)
        {
            _storageContext = storageContext;
        }

        public void CreateNewDrone()
        {
            Drone = new Comms.Drone();
            Drone.DroneName = "?";
            Drone.DroneAddress = 0;
            Drone.UserId = "bytemaster";

            Drone.CurrentComms = new Models.DroneComms
            {
                IsEditing = false,
                Name = String.Empty,
                Mode = (byte)Models.DroneComms.Modes.Unknown,
                Address = String.Empty,
                Port = 0,
                Passphrase = String.Empty,
                APName = String.Empty,
                SSID = String.Empty,
                Host = String.Empty,
                ProtocolType = (Byte)Models.DroneComms.ProtocolTypes.Unknown
            };

            Comms = Drone.CurrentComms;

            _isEditing = false;
        }

        public void SetDrone(Comms.Drone drone)
        {
            Drone = drone;

            Comms = Drone.CurrentComms;

            _isEditing = true;
        }

        public void ProcessMessage(String message)
        {
            var parts = message.Split('=');
            if (parts.Length == 2)
            {
                var key = parts[0];
                var value = parts[1];

                switch (key.ToLower())
                {
                    case "ip":
                        {
                            var ipParts = value.Split(':');
                            Comms.Address = ipParts[0];
                            Comms.Port = Convert.ToInt16(ipParts[1]);
                        }
                        break;
                    case "flags":
                        break;
                    case "deviceid": Comms.APName = value; break;
                    case "proto":
                        if (value.ToLower().StartsWith("udp"))
                            Comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.UDP;
                        else if (value.ToLower().StartsWith("tcp"))
                            Comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.TCP;
                        else
                            Comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.Unknown;
                        break;
                    case "join":
                        switch (value)
                        {
                            case "1": Comms.Mode = (byte)Models.DroneComms.Modes.Infrastructure; break;
                            case "7": Comms.Mode = (byte)Models.DroneComms.Modes.AdHoc; break;
                            default: Comms.Mode = (byte)Models.DroneComms.Modes.Unknown; break;

                        }
                        break;
                    case "host":
                        {
                            var ipParts = value.Split(':');
                            Comms.Host = ipParts[0];
                        }
                        break;
                    case "ssid": Comms.SSID = value; break;
                    case "passphrase":
                        Comms.Passphrase = value;
                        break;
                    case "a": Drone.DroneAddress = Convert.ToByte(value); break;
                    case "n": Drone.DroneName = value; break;
                    case "s": Drone.DroneSN = Convert.ToInt32(value); break;
                }
            }
        }

        public RelayCommand UpdateDroneComms
        {
            get
            {
                return new RelayCommand(() => { SendCommands(this, Comms.GetCommands()); });
            }
        }

        public RelayCommand SaveDrone
        {
            get
            {
                return new RelayCommand(() =>
                {
                    var cmds = new List<String>()
                    {
                        "set a " + Drone.DroneAddress + "\r",
                        "set n " + Drone.DroneName + "\r"
                    };

                    SendCommands(this, cmds);
                });
            }
        }

        public RelayCommand ExtractComms
        {
            get
            {
                return new RelayCommand(() =>
                {
                    var cmds = new List<String>()
                    {
                        "get c\r",
                        "comms\r",
                        "$$$\r",
                        "get ip \r",
                        "get wlan \r",
                        "get opt \r",
                        "reboot\r",
                        "exit\r"
                    };

                    SendCommands(this, cmds);
                });
            }
        }

        public RelayCommand SaveSettingsCommand
        {
            get
            {
                return new RelayCommand(async () =>
                {
                    IsBusy = true;
                    try
                    {
                        if (!_isEditing)
                        {
                            DroneHub.Instance.Drones.Add(Drone);
                        }

                        await DroneHub.Instance.Save();

                        _isEditing = true;

                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine("MESSAGE: " + ex.Message);
                    }
                    finally
                    {
                        IsBusy = false;
                    }
                });
            }
        }

        private Models.DroneComms _comms;
        public Models.DroneComms Comms
        {
            get { return _comms; }
            set { Set(ref _comms, value); }
        }
    }
}
