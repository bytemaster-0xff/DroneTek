using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProxyApp.Server
{
    public class DataReceivedEventArgs
    {
        public byte[] Buffer { get; set; }
        public int BufferLen { get; set; }
    }
}
