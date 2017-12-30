using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace NiVek.FlightControls.Services
{
    public class TimerManager : ITimerManager
    {

        public ITimer CreateTimer(TimeSpan interval)
        {
            var timer = new Timer();
            timer.Interval = interval;

            return timer;
        }
    }

    public class Timer : ITimer
    {
        public System.Threading.Timer _timer;

        public Timer()
        {
            _timer = new System.Threading.Timer((obj) =>
            {
                Elapsed?.Invoke(this, null);

            }, null, 0, Timeout.Infinite);
        }

        public event EventHandler Elapsed;

        public void Start()
        {
            _timer.Change(TimeSpan.FromMilliseconds(0), Interval);
        }

        public void Stop()
        {
            _timer.Change(0, Timeout.Infinite);
        }

        public TimeSpan Interval {get; set;}
    }
}
