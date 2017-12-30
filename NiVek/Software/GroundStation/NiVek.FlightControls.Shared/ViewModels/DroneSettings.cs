using GalaSoft.MvvmLight.Command;
using NiVek.Common.Comms;
using NiVek.Common.Services;
using NiVek.FlightControls.Commo;
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

        public event EventHandler<List<String>> SendCommands;

        private Models.DroneComms _comms;

        private bool _isEditing;

        public DroneSettings()
        {
            _comms = new Models.DroneComms();
            _comms.IsEditing = false;
            _comms.Name = String.Empty;
            _comms.Mode = (byte)Models.DroneComms.Modes.Unknown;
            _comms.Address = String.Empty;
            _comms.Port = 0;
            _comms.Passphrase = String.Empty;
            _comms.APName = String.Empty;
            _comms.SSID = String.Empty;
            _comms.Host = String.Empty;
            _comms.ProtocolType = (Byte)Models.DroneComms.ProtocolTypes.Unknown;
        }

        public void CreateNewDrone()
        {
            Drone = new Comms.Drone();
            Drone.DroneName = "?";
            Drone.DroneAddress = 0;
            Drone.UserId = "bytemaster";
            _isEditing = false;
        }

        public void SetDrone(Comms.Drone drone)
        {
            Drone = drone;

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
                            _comms.Address = ipParts[0];
                            _comms.Port = Convert.ToInt16(ipParts[1]);
                        }
                        break;
                    case "flags":
                        break;
                    case "deviceid": _comms.APName = value; break;
                    case "proto":
                        if (value.ToLower().StartsWith("udp"))
                            _comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.UDP;
                        else if (value.ToLower().StartsWith("tcp"))
                            _comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.TCP;
                        else
                            _comms.ProtocolType = (byte)NiVek.Common.Models.DroneComms.ProtocolTypes.Unknown;
                        break;
                    case "join":
                        switch (value)
                        {
                            case "1": _comms.Mode = (byte)Models.DroneComms.Modes.Infrastructure; break;
                            case "7": _comms.Mode = (byte)Models.DroneComms.Modes.AdHoc; break;
                            default: _comms.Mode = (byte)Models.DroneComms.Modes.Unknown; break;

                        }
                        break;
                    case "host":
                        {
                            var ipParts = value.Split(':');
                            _comms.Host = ipParts[0];
                        }
                        break;
                    case "ssid": _comms.SSID = value; break;
                    case "passphrase":
                        _comms.Passphrase = value;
                        break;
                    case "a": Drone.DroneAddress = Convert.ToByte(value); break;
                    case "n": Drone.DroneName = value; break;
                    case "s": Drone.DroneSN = Convert.ToInt32(value); break;
                }
            }
        }


        public RelayCommand SaveServerComms
        {
            get
            {
                return new RelayCommand(async () =>
                {
                    IsBusy = true;
                    try
                    {
                        _comms.DroneId = Drone.Id;

                        if (_comms.IsEditing)
                            await AMSDataContext.Instance.DroneComms.UpdateAsync(_comms);
                        else
                        {
                            await AMSDataContext.Instance.DroneComms.InsertAsync(_comms);
                            Drone.CommSettings.Add(_comms);
                        }

                        Drone.CurrentComms = _comms;

                        await DroneHub.Instance.Save();
                    }
                    catch (Exception ex)
                    {

                    }
                    finally
                    {
                        IsBusy = false;
                    }
                });
            }
        }

        public RelayCommand UpdateDroneComms
        {
            get
            {
                return new RelayCommand(() =>
                {
                    SendCommands(this, Comms.GetCommands());
                });
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

        public RelayCommand SaveServer
        {
            get
            {
                return new RelayCommand(async () =>
                {
                    IsBusy = true;
                    try
                    {
                        if (_isEditing)
                            await AMSDataContext.Instance.Drones.UpdateAsync(Drone);
                        else
                        {
                            await AMSDataContext.Instance.Drones.InsertAsync(Drone);
                            DroneHub.Instance.Drones.Add(Drone);
                        }

                        await DroneHub.Instance.Save();

                        _isEditing = false;

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

        public Models.DroneComms Comms
        {
            get { return _comms; }
        }
    }
}
