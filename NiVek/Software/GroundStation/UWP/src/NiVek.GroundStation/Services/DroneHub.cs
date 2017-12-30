using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Services;
using System;
using System.IO;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using System.Diagnostics;
using Newtonsoft.Json;

namespace NiVek.WinCommon.Services
{
    public class DroneHub : ModelBase, INotifyPropertyChanged
    {
        private static DroneHub _droneHub;

        private ObservableCollection<Drone> _drones;

        public event EventHandler<Drone> OnConnect;
        public event EventHandler<Drone> OnDisconnect;

        private DroneHub()
        {

        }

        static DroneHub()
        {
            _droneHub = new DroneHub();
        }

        private void MessageReceived(Object sndr, IncomingMessage msg)
        {

            //Debug.WriteLine("Message Received.");

            if (msg.SourceAddress > 19)
            {
                var drone = _drones.Where(drn => drn.DroneAddress == msg.SourceAddress).FirstOrDefault();
                if (drone == null)
                {
                    lock (_drones)
                    {
                        drone = new Drone
                        {
                            DroneAddress = msg.SourceAddress
                        };
                        _drones.Add(drone);

                        NiVek.Common.Services.AppServices.UIThread.Invoke(() =>
                        {
                            _drones.Add(drone);
                        });
                    }

                    drone.PopulateDroneName();

                    OnConnect(this, drone);
                }

                drone.LastContactDateStamp = DateTime.Now;

                _drones.Where(drn => drn.DroneAddress == msg.DestAddress).First().HandleMessage(msg);
            }

        }

        public async Task Save()
        {
            StorageFile droneHubFile = null;
            try
            {

                droneHubFile = await Windows.Storage.ApplicationData.Current.RoamingFolder.GetFileAsync("drones");
            }
            catch (Exception)
            {
                Debug.WriteLine("Could not open current file, will catch exception and create new.");
            }

            if (droneHubFile == null)
                droneHubFile = await Windows.Storage.ApplicationData.Current.RoamingFolder.CreateFileAsync("drones");

            using (var droneHubStream = await droneHubFile.OpenStreamForWriteAsync())
            using (var writer = new StreamWriter(droneHubStream))
            using (var jsonWriter = new JsonTextWriter(writer))
            {
                JsonSerializer ser = new JsonSerializer();
                ser.Serialize(jsonWriter, _drones);
                jsonWriter.Flush();
            }
        }


        public async Task Populate()
        {
            try
            {
                if (_drones == null)
                {
                    using (var droneHubStream = await Windows.Storage.ApplicationData.Current.RoamingFolder.OpenStreamForReadAsync("drones"))
                    using (var streamReader = new StreamReader(droneHubStream))
                    using (var jsonReader = new JsonTextReader(streamReader))
                    {
                        var deserializer = new JsonSerializer();
                        _drones = deserializer.Deserialize<ObservableCollection<Drone>>(jsonReader);
                    }


                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Could not open drone store, will create new one {ex.Message}");
                Debugger.Break();

                _drones = new ObservableCollection<Drone>();
            }
        }

        private Drone _active = null;
        public Drone Active
        {
            get { return _active; }
            set
            {
                if (_active != null)
                {
                    _active.OnConnect -= _active_OnConnect;
                    _active.OnDisconnect -= _active_OnDisconnect;
                }

                HasActiveDrone = (value != null);

                Set(ref _active, value);

                if (_active != null)
                {
                    _active.OnConnect += _active_OnConnect;
                    _active.OnDisconnect += _active_OnDisconnect;
                }
            }
        }

        void _active_OnConnect(object sender, EventArgs e)
        {
            IsDroneConnected = true;
            OnConnect?.Invoke(this, Active);
        }

        void _active_OnDisconnect(object sender, String message)
        {
            IsDroneConnected = false;
            OnDisconnect?.Invoke(_active, Active);
        }

        internal async Task RemoveCurrent()
        {
            Drones.Remove(Active);
            await Save();
            HasActiveDrone = false;
        }

        private bool _hasActiveDrone = false;
        public bool HasActiveDrone
        {
            get { return _hasActiveDrone; }
            set { Set(ref _hasActiveDrone, value); }

        }

        bool _isDroneConnected = false;
        public bool IsDroneConnected
        {
            get { return _isDroneConnected; }
            set
            {
                AppServices.UIThread.Invoke(() =>
                {
                    Set(ref _isDroneConnected, value);
                });
            }
        }

        public static DroneHub Instance { get { return _droneHub; } }

        public ObservableCollection<Drone> Drones { get { return _drones; } }
    }
}
