using NiVek.Common.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.Common
{
    public class UIThread : IUIThread
    {
        Windows.UI.Core.CoreDispatcher _dispatcher;
        public UIThread(Windows.UI.Core.CoreDispatcher dispatcher)
        {
            _dispatcher = dispatcher;
        }

        public bool InvokeRequired
        {
            get { return !_dispatcher.HasThreadAccess; }
        }

        public async void Invoke(Action action)
        {
            await _dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => action());
        }
    }
}
