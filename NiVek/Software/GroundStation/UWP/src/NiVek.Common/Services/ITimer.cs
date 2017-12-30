using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public interface ITimerManager
    {
        ITimer CreateTimer(TimeSpan interval);
    }

    public interface ITimer
    {
        event EventHandler Elapsed;

        void Start();
        void Stop();

        TimeSpan Interval { get; set; }
    }
}
