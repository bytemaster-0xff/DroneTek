using NiVek.Common.Comms;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;
using System.IO;
using System.Runtime.Serialization.Json;
using System.Threading.Tasks;
using Windows.Storage;
using System.ComponentModel;
using NiVek.Common.Models;
using NiVek.Common.Services;

namespace NiVek.FlightControls.Commo
{
    public class DroneHub : ModelBase, INotifyPropertyChanged
    {
        private static DroneHub _droneHub;

        private ObservableCollection<Drone> _drones;

        public event EventHandler<Drone> OnConnect;
        public event EventHandler<Drone> OnDisconnect;

        private DroneHub()
        {
            _drones = new ObservableCollection<Drone>();

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
                        drone = new Drone();
                        drone.DroneAddress = msg.SourceAddress;
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
            catch (Exception ex)
            { }

            if (droneHubFile == null)
                droneHubFile = await Windows.Storage.ApplicationData.Current.RoamingFolder.CreateFileAsync("drones");

            using (var droneHubStream = await droneHubFile.OpenStreamForWriteAsync())
            {
                var serializer = new DataContractJsonSerializer(typeof(ObservableCollection<Drone>));
                serializer.WriteObject(droneHubStream, _drones);
            };
        }

        public async Task Populate()
        {
            try
            {
                var droneHubFile = await Windows.Storage.ApplicationData.Current.RoamingFolder.OpenStreamForReadAsync("drones");
                using (var droneHubStream = droneHubFile.AsInputStream())
                {
                    var serializer = new DataContractJsonSerializer(typeof(ObservableCollection<Drone>));
                    _drones = serializer.ReadObject(droneHubFile) as ObservableCollection<Drone>;
                }
            }
            catch (Exception ex)
            {
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
            if (OnConnect != null)
                OnConnect(this, Active);
        }

        void _active_OnDisconnect(object sender, String message)
        {
            IsDroneConnected = false;
            if (OnDisconnect != null)
                OnDisconnect(_active, Active);
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
