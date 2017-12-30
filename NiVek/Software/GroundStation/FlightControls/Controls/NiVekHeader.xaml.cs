using NiVek.Common;
using NiVek.Common.Comms;
using NiVek.Common.Models;
using NiVek.Common.Modules;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.System.Threading;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    /*
     * The goal of this class is to provide an common interface that will appear on all 
     * UI that communicate with the device.
     * 
     * It will:
     * 1)  Display status in a textual way.
     * 2)  Provide a mechanism to make sure stuff that needs to be "acked" are "acked" from the device
     * 3)  Display a message error rate
     * 4)  Interface for switching control method (WiFi/Radio)
     * 5)  Interface for Arm/Safe
     * 6)  Interface for zeroing sensors
     * 
     */
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
