using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.ViewModels
{
    public class Console : ViewModelBase
    {
        private Models.DroneComms _comms;

        public Models.DroneComms Comms
        {
            get { return _comms; }
        }

    }
}
