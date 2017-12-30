using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public interface IUIThread
    {
        bool InvokeRequired { get; }

        void Invoke(Action action);
    }
}
