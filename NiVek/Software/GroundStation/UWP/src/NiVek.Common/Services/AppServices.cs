using LagoVista.Core.IOC;
using System.Collections.Generic;

namespace NiVek.Common.Services
{
    public class AppServices
    {
        private static Dictionary<byte, ushort> _serialNumbers = new Dictionary<byte,ushort>();

        public static byte LocalAddress { get { return 1;  } }

        public static IUIThread UIThread { get { return SLWIOC.Get<IUIThread>(); } }
        public static ITimerManager TimerManager { get { return SLWIOC.Get<ITimerManager>(); } }
    
    
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
