using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace NiVek.WP8.Common
{
    public class UIThread : IUIThread
    {
        private Dispatcher _dispatcher;

        public UIThread(Dispatcher dispatcher)
        {
            _dispatcher = dispatcher;
        }

        private UIThread()
        {

        }

        public bool InvokeRequired
        {
            get { return true; }
        }

        public void Invoke(Action action)
        {
            _dispatcher.BeginInvoke(action);
        }

    }
}
