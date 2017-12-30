using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Utils
{
    public interface IUIThread
    {
        void Invoke(Action action);
        bool InvokeRequired {get;}
    }
}
