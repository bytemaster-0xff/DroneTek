using Microsoft.Practices.ServiceLocation;
using NiVek.Common.Services;
using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public class AppServices
    {
        private static Dictionary<byte, ushort> _serialNumbers = new Dictionary<byte,ushort>();

        public static byte LocalAddress { get { return 1;  } }

        public static IUIThread UIThread { get { return ServiceLocator.Current.GetInstance<IUIThread>(); } }
        public static ITimerManager TimerManager { get { return ServiceLocator.Current.GetInstance<ITimerManager>(); } }
    
    
        public static ushort GetNextSerialNumber(byte droneID)
        {
            if (_serialNumbers.ContainsKey(droneID))
                _serialNumbers[droneID]++;
            else
                _serialNumbers.Add(droneID, 1);

            return _serialNumbers[droneID];
        }
    }
}
