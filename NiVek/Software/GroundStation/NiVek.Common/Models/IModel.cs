using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public interface IModel
    {
        Drone Drone { get; set; }

        void Update(BufferReader rdr);

        bool IsReady { get; }
    }
}
