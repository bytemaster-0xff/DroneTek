using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Comms
{
    //Will define and handle access the connects to external hardware that handles and communicates with multiple drones
    public class DroneProxy
    {
        public event EventHandler<Drone> DroneConnected;
        public event EventHandler<Drone> DroneDisconnected;
    }
}
