using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProxyApp
{
    public class Common
    {
        public const int ListenerPort = 9050;

        public static String COMPort { get; set; }

        public static byte LocalAddress { get; set; }

        public static String DeviceName { get; set; }
    }
}
