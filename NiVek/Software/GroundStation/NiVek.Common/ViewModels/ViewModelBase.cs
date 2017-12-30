using GalaSoft.MvvmLight.Command;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.ViewModels
{
    public class ViewModelBase : GalaSoft.MvvmLight.ViewModelBase
    {
        private Drone _drone;

        private bool _isBusy;
        public bool IsBusy
        {
            get { return _isBusy; }
            set
            {
                _isBusy = value;
                RaisePropertyChanged(() => IsBusy);
            }
        }

        public ViewModelBase()
        {
            if(IsInDesignMode)
            {

            }
        }

        public Drone Drone
        {
            get { return _drone; }
            set { 
                if(_drone != value)
                {
                    _drone = value;
                    RaisePropertyChanged(() => Drone);
                    RaisePropertyChanged(() => Targets);
                    RaisePropertyChanged(() => SensorData);
                    RaisePropertyChanged(() => GPSData);
                    RaisePropertyChanged(() => Motors);
                    RaisePropertyChanged(() => Status);
                    RaisePropertyChanged(() => DroneName);
                    RaisePropertyChanged(() => DisplayName);
                    RaisePropertyChanged(() => IsXFrameConfig);
                    RaisePropertyChanged(() => IsCrossFrameConfig);
                }
            }
        }

        private bool _disconnectedMask;
        public bool DisconnectedMask 
        { 
            get { return _disconnectedMask; }
            set { Set(ref _disconnectedMask, value); }
        }

        #region Passthru-properties on the current drone instance
        public SystemStatus Status
        {
            get { return Drone.Status; }
            set
            {
                if(Drone.Status != value)
                {
                    Drone.Status = value;
                    RaisePropertyChanged(() => Status);
                }
            }
        }


        public GPIOConfig Config
        {
            get { return Drone.Config; }
            set {
                if (Drone.Config != value)
                {
                    Drone.Config = value;
                    RaisePropertyChanged(() => Config);
                    RaisePropertyChanged(() => IsXFrameConfig);
                    RaisePropertyChanged(() => IsCrossFrameConfig);
                }
            }
        }

        public bool IsXFrameConfig
        {
            get { return Config.FrameConfig == (byte)SystemStatus.FrameConfigEnum.X; }
        }

        public bool IsCrossFrameConfig
        {
            get { return Config.FrameConfig == (byte)SystemStatus.FrameConfigEnum.Cross; }
        }

        public GPSData GPSData
        {
            get { return Drone.GPSData; }
            set 
            {
                if(Drone.GPSData != value)
                {
                    Drone.GPSData = value;
                    RaisePropertyChanged(()=>GPSData);
                }
            }
        }

        public BatteryCondition Battery
        {
            get { return Drone.Battery; }
            set {
                if (Drone.Battery != value)
                {
                    Drone.Battery = value;
                    RaisePropertyChanged(() => Battery);
                }
            }
        }

        public MotorStatus Motors
        {
            get { return Drone.Motors;  }
            set { 
                if(Drone.Motors != value)
                {
                    Drone.Motors = value;
                    RaisePropertyChanged(()=>Motors);
                }
            }
        }

        public String DroneName
        {
            get { return Drone.DroneName;  }
            set { 
                if(Drone.DroneName != value)
                {
                    Drone.DroneName = value;
                    RaisePropertyChanged(() => DroneName);
                    RaisePropertyChanged(() => DisplayName);
                }
            }
        }

        public String DisplayName
        {
            get { return Drone.DisplayName;  }
        }

        public Targets Targets
        {
            get { return Drone.Targets; }
            set
            {
                if (Drone.Targets != value)
                {
                    Drone.Targets = value;
                    RaisePropertyChanged(() => Targets);
                }
            }
        }

        public SensorUpdate SensorData
        {
            get { return Drone.SensorData; }
            set
            {
                if(Drone.SensorData != value)
                {
                    Drone.SensorData = value;
                    RaisePropertyChanged(() => SensorData);
                }
            }
        }
       #endregion


        public void ShowNetworkError()
        {

        }

        public void MessageArrived(IncomingMessage msg)
        {
            if (Drone == null)
            {
                DisconnectedMask = true;
                return;
            }

            DisconnectedMask = false;

            switch ((NiVek.Common.Comms.Common.ModuleTypes)msg.SystemId)
            {
                case NiVek.Common.Comms.Common.ModuleTypes.GPIO:
                    {
                        switch (msg.MessageId)
                        {
                            case IncomingMessage.GPIOPidConstants:
                                Config = new NiVek.Common.Models.GPIOConfig(Drone, msg.Payload);
                                break;
                        }
                    }
                    break;

                case NiVek.Common.Comms.Common.ModuleTypes.Sensor:
                    {
                        switch (msg.MessageId)
                        {
                            case IncomingMessage.SensorBattery:
                                if (Battery == null)
                                    Battery = BatteryCondition.Create(msg.Payload);
                                else
                                    Battery.Update(msg.Payload);
                                break;
                            case IncomingMessage.SensorData:
                                if (SensorData == null)
                                    SensorData = SensorUpdate.Create(msg.Payload);
                                else
                                    SensorData.Update(msg.Payload);
                                break;

                            case IncomingMessage.SensorGps:
                                if (GPSData == null)
                                    GPSData = GPSData.Create(msg.Payload);
                                else
                                    GPSData.Update(msg.Payload);
                                break;
                        }
                    }
                    break;

                case NiVek.Common.Comms.Common.ModuleTypes.System:
                    {
                        switch (msg.MessageId)
                        {
                            case NiVek.Common.Comms.IncomingMessage.NiVekName:
                                DroneName = System.Text.Encoding.UTF8.GetString(msg.Payload, 0, msg.PayloadSize).Replace((char)0x00, ' ').Trim();
                                break;


                            case IncomingMessage.SystemStatus:
                                if (Status == null)
                                    Status = SystemStatus.Create(msg.Payload);
                                else
                                    Status.Update(msg.Payload);
                                break;
                            case IncomingMessage.MotorStatus:
                                if (Motors == null)
                                    Motors = MotorStatus.Create(msg.Payload);
                                else
                                    Motors.Update(msg.Payload);
                                break;
                            case IncomingMessage.Targets:
                                if (Targets == null)
                                    Targets = Targets.Create(msg.Payload);
                                else
                                    Targets.Update(msg.Payload);
                                break;
                        }
                    }
                    break;
            }
        }
    }
}
