using LagoVista.Core.IOC;
using NiVek.Common.Services;
using NiVek.FlightControls.Services;
using NiVek.GroundStation.Inputs;
using NiVek.GroundStation.Services;
using System;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Data.Xml.Dom;
using Windows.UI.Notifications;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=234227

namespace NiVek.FlightControls
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        public static Window MainWindow;

        FlightController _flightController;
        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            this.InitializeComponent();
            this.Suspending += OnSuspending;
            _theApp = this;

            _flightController = new FlightController();

            SLWIOC.RegisterSingleton<ITimerManager>(new TimerManager());
            SLWIOC.RegisterSingleton<IUIThread>(new UIThread());
            SLWIOC.RegisterSingleton<IStorageContext>(new StorageContext());
        }

        public FlightController FlightController
        {
            get { return _flightController; }
        }

        void _nivekConsole_DroneConnected(object sender, NiVek.Common.Comms.Drone e)
        {
            var notifications = Windows.UI.Notifications.ToastNotificationManager.CreateToastNotifier();

            XmlDocument toastXml = ToastNotificationManager.GetTemplateContent(ToastTemplateType.ToastImageAndText01);
            XmlNodeList toastTextElements = toastXml.GetElementsByTagName("text");
            toastTextElements[0].AppendChild(toastXml.CreateTextNode("Hello World!"));
            XmlNodeList toastImageAttributes = toastXml.GetElementsByTagName("image");
            ((XmlElement)toastImageAttributes[0]).SetAttribute("src", "ms-appx:///images/QCTop.png");
            ((XmlElement)toastImageAttributes[0]).SetAttribute("alt", "red graphic");
            IXmlNode toastNode = toastXml.SelectSingleNode("/toast");
            XmlElement audio = toastXml.CreateElement("audio");
            audio.SetAttribute("src", "ms-winsoundevent:Notification.IM");
            ToastNotification toast = new ToastNotification(toastXml);
            ToastNotificationManager.CreateToastNotifier().Show(toast);

            // Get the toast notification manager for the current app.
            //var notificationManager = notifications.ToastNotificationManager;

        }


        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used when the application is launched to open a specific file, to display
        /// search results, and so forth.
        /// </summary>
        /// <param name="args">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs args)
        {
            _theApp = this;
            //if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("isAdHoc"))
            //    _nivekConsole.IsAdhoc = (bool)Windows.Storage.ApplicationData.Current.LocalSettings.Values["isAdHoc"];
            //else
            //    _nivekConsole.IsAdhoc = false;

            // Do not repeat app initialization when already running, just ensure that
            // the window is active
            if (args.PreviousExecutionState == ApplicationExecutionState.Running)
            {
                Window.Current.Activate();
                //     await _nivekConsole.ConnectUDP();
                //await _nivekConsole.Connect();
                return;
            }

            if (args.PreviousExecutionState == ApplicationExecutionState.Terminated)
            {
                //TODO: Load state from previously suspended application
            }

            // Create a Frame to act navigation context and navigate to the first page
            _frame = new Frame();
            if (!_frame.Navigate(typeof(Launcher)))
            {
                throw new Exception("Failed to create initial page");
            }


            // Place the frame in the current Window and ensure that it is active
            Window.Current.Content = _frame;
            Window.Current.Activate();
            App.MainWindow = Window.Current;
        }

        public static App _theApp;

        public static App TheApp
        {
            get { return _theApp; }
        }

        private Frame _frame;
        public Frame Frame
        {
            get
            {
                return _frame;
            }
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {

            var deferral = e.SuspendingOperation.GetDeferral();
            deferral.Complete();
        }
    }
}
