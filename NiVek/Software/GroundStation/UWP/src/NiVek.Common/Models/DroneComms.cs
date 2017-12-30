using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using Newtonsoft.Json.Converters;

namespace NiVek.Common.Models
{
    [DataContract(Name="drone_comms")]
    public class DroneComms : ModelBase
    {
        public DroneComms()
        {

        }

        [JsonIgnore()]
        public bool IsEditing
        {
            get;
            set;
        }

        [JsonProperty(PropertyName="id")]
        public Guid Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "drone_id")]
        public Guid DroneId
        {
            get;
            set;
        }


        private String _name;
        [JsonProperty(PropertyName = "name")]
        public String Name
        {
            get { return _name; }
            set { Set(ref _name, value); }
        }

        private String _ssid = String.Empty;
        [JsonProperty(PropertyName = "ssid")]
        public String SSID
        {
            get { return _ssid; }
            set { Set(ref _ssid, value); }
        }

        private String _ip = String.Empty;
        [JsonProperty(PropertyName = "ip")]
        public String Address
        {
            get { return _ip; }
            set { Set(ref _ip, value); }
        }

        private String _host;
        [JsonProperty(PropertyName = "host")]
        public String Host
        {
            get { return _host; }
            set { Set(ref _host, value); }
        }

        private String _apName;
        [JsonProperty(PropertyName = "ap_name")]
        public String APName
        {
            get { return _apName; }
            set { Set(ref _apName, value); }
        }

        private short _port;
        [JsonProperty(PropertyName = "port")]
        public short Port
        {
            get { return _port; }
            set { Set(ref _port, value); }
        }

        public enum Modes
        {
            Unknown = 0,
            Infrastructure = 1,
            AdHoc = 7
        }

        private Modes _mode;
        [JsonProperty(PropertyName = "mode")]
        public Byte Mode
        {
            get { return (byte)_mode; }
            set { Set(ref _mode, (Modes)value); }
        }

        private String _passphrase;
        [JsonProperty(PropertyName = "passphrase")]
        public String Passphrase
        {
            get { return _passphrase; }
            set { Set(ref _passphrase, value); }
        }

        public enum ProtocolTypes
        {
            Unknown = 0,
            UDP = 10,
            TCP = 11,
            Serial = 12
        }

        ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue> _modeList = new ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue>()
            {
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="None", Value=(byte)NiVek.Common.Models.DroneComms.Modes.Unknown },
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="AdHoc", Value=(byte)NiVek.Common.Models.DroneComms.Modes.AdHoc },
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="Infrastrcuture", Value=(byte)NiVek.Common.Models.DroneComms.Modes.Infrastructure },
            };

        [JsonIgnore]
        public ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue> ModeList { get { return _modeList; } }

        ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue> _protocolList = new ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue>()
            {
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="None", Value=(byte)NiVek.Common.Models.DroneComms.ProtocolTypes.Unknown },
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="TCP", Value=(byte)NiVek.Common.Models.DroneComms.ProtocolTypes.TCP },
                new NiVek.Common.Models.ModelBase.SettingsValue() { Display ="UDP", Value=(byte)NiVek.Common.Models.DroneComms.ProtocolTypes.UDP },
            };

        [JsonIgnore]
        public ObservableCollection<NiVek.Common.Models.ModelBase.SettingsValue> ProtocolList { get { return _protocolList; } }


        private ProtocolTypes _protocolType;
        [JsonProperty(PropertyName = "protocol_type")]
        public Byte ProtocolType
        {
            get { return (Byte)_protocolType;}
            set { Set(ref _protocolType, (ProtocolTypes)value); }
        }


        public List<String> GetCommands()
        {
            var script = new List<String>()
            {

                "comms\r",
                "$$$\r",
            };

            if(Mode == (byte)Modes.AdHoc)
            {
                script.Add("set w j 7\r");
                script.Add("set i d 4\r");
                script.Add("set i a 1.2.3.4\r");
                script.Add("set i h 0.0.0.0\r");
                script.Add("set w s " + APName + "\r");
                script.Add("set o d " + APName + "\r");
            }
            else if(Mode == (byte)Modes.Infrastructure)
            {
                script.Add("set w j 1 \r");
                script.Add("set w s " + SSID + "\r");
                script.Add("set w p " + Passphrase + "\r");
                script.Add("set i h " + Host + "\r");
                script.Add("set i d 0\r");
                script.Add("set i a " + Address  + "\r");                
            }

            script.Add("set i r " + Port + "\r");
            script.Add("set i l " + Port + "\r");               

            var ipMode = (ProtocolType == (byte)ProtocolTypes.UDP) ? "1" : "2";
            
            script.Add("set i p " + ipMode + "\r");


            script.Add("save\r");
            script.Add("reboot\r");
            script.Add("exit\r");

            return script;
        }

        public void ProcessResponse(String msg)
        {
            if (msg.Trim().Length > 0)
            {
                Debug.WriteLine(msg);

                var cmdParts = msg.Split('=');
                if (cmdParts.Count() == 2)
                {
                    switch (cmdParts[0].ToLower())
                    {
                        case "ip":
                            {
                                var ipParts = cmdParts[1].Split(':');
                                Address = ipParts[0];
                                Port = Convert.ToInt16(ipParts[1]);
                            }
                            break;
                        case "flags":
                            break;
                        case "deviceid":
                            APName = cmdParts[1];
                            break;
                        case "proto":
                            if (cmdParts[1].ToLower().StartsWith("udp"))
                                ProtocolType = (Byte)ProtocolTypes.UDP;
                            else if (cmdParts[1].ToLower().StartsWith("tcp"))
                                ProtocolType = (Byte)ProtocolTypes.TCP;
                            else
                                ProtocolType = (Byte)ProtocolTypes.Unknown;

                            break;
                        case "join":
                            switch (cmdParts[1])
                            {
                                case "1": Mode = (Byte)Modes.Infrastructure; break;
                                case "7": Mode = (Byte)Modes.AdHoc; break;

                            }
                            break;
                        case "host":
                            {
                                var ipParts = cmdParts[1].Split(':');
                                Host = ipParts[0];
                            }
                            break;
                        case "ssid":
                            SSID = cmdParts[1];
                            break;

                        case "passphrase":
                            Passphrase = cmdParts[1];
                            break;
                    }
                }
            }
        }
    }
}
