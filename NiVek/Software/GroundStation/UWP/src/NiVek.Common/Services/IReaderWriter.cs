using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public interface IReaderWriter
    {
        Task<Boolean> Send(byte[] buffer, int offset, int len);

        Task<Boolean> Connect(String host, int port);

        Task Close();
    }
}
