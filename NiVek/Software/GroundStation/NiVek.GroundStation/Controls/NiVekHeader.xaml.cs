using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.GroundStation.Controls
{
    public sealed partial class NiVekHeader : UserControl
    {

        public NiVekHeader()
        {
            this.InitializeComponent();

            MessageQueue.DataContextChanged += MessageQueue_DataContextChanged;
        }

        void MessageQueue_DataContextChanged(FrameworkElement sender, DataContextChangedEventArgs args)
        {
            var drone = args.NewValue as Drone;
            if (drone != null)
                drone.Channel.MessageLog.CollectionChanged += MessageLog_CollectionChanged;
        }

        void MessageLog_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            try
            {
                var messageLog = MessageQueue.ItemsSource as ObservableCollection<MessageEntry>;
                MessageQueue.UpdateLayout();
                MessageQueue.ScrollIntoView(messageLog[messageLog.Count - 1]);
            }
            catch (Exception)
            {
                Debug.WriteLine("Minor timing error...");
            }
        }



        private void backButton_Click_1(object sender, RoutedEventArgs e)
        {


            Host.GoBack();
        }

        public Views.NiVekPage Host
        {
            get
            {
                var thisParent = this.Parent as FrameworkElement;
                while (thisParent != null)
                {
                    var host = thisParent as Views.NiVekPage;
                    if (host != null)
                        return host;

                    thisParent = thisParent.Parent as FrameworkElement;
                }

                return null;
            }
        }
    }
}
