using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.Text;
using Windows.UI.Xaml;

namespace NiVek.FlightControls.Services
{
    public class UIThread : IUIThread
    {

        public bool InvokeRequired
        {
            get { return App.TheApp.Frame.Dispatcher.HasThreadAccess; }
        }

        public async void Invoke(Action action)
        {
            await App.TheApp.Frame.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, ()=> action());
        }
    }
}
