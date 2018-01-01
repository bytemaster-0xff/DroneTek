using NiVek.Common.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace NiVek.FlightControls.Views
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    /// 
    public sealed partial class BlackBoxPlayback : NiVekPage
    {
        //NiVek.GameWrapper _renderer;
        //NiVek.BasicTimer _timer;
        private bool _isPaused = false;

        int _index;

        DispatcherTimer _playbackTimer;

        public BlackBoxPlayback()
        {
            this.InitializeComponent();

          //  _renderer = new GameWrapper();
//            _renderer.Initialize(Window.Current.CoreWindow, SwapChainPanel, DisplayProperties.LogicalDpi);
            CompositionTarget.Rendering += CompositionTarget_Rendering;
            this.Loaded += BlackBoxPlayback_Loaded;
  //          _timer = new BasicTimer();

            _playbackTimer = new DispatcherTimer();
            _playbackTimer.Interval = TimeSpan.FromMilliseconds(20);
            _playbackTimer.Tick += _playbackTimer_Tick;
        }

        void _playbackTimer_Tick(object sender, object e)
        {
            if (_index < _telemetryHistory.Count)
            {
                var item = _telemetryHistory[_index++];

                Debug.WriteLine(DateTime.Now + " - Print -> " + item.TimeStamp + ", " + item.Pitch + "," + item.Roll + ", " + item.Altitude);
            }
            else
                _playbackTimer.Stop();

        }

        async void BlackBoxPlayback_Loaded(object sender, RoutedEventArgs e)
        {
            var files = await Windows.Storage.ApplicationData.Current.RoamingFolder.GetFilesAsync();
            BBDataFiles.ItemsSource = files;
        }

        void CompositionTarget_Rendering(object sender, object e)
        {
            /*
            int tenths = (int)((_timer.Total % 1.0) * 10);
            if (tenths > 9)
                tenths = 0;

            TimeStamp.Text = String.Format("{0:00}:{1:00}.{2}", _timer.Total / 60.0, _timer.Total % 60.0, tenths);

            if (!_isPaused)
                _timer.Update();

            _renderer.Update(_timer.Total, _timer.Delta);
            _renderer.Render();
            _renderer.Present();
             */
        }

      

        private void Pause_Click(object sender, RoutedEventArgs e)
        {
            _isPaused = !_isPaused;
            Pause.Content = _isPaused ? "Continue" : "Pause";
        }

        private List<BlackBoxRecord> _telemetryHistory;

        private async void DataFiles_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
       
            if (e.AddedItems.Count == 1)
            {
                _telemetryHistory = new List<BlackBoxRecord>();
                
                FileLoading.Visibility = Windows.UI.Xaml.Visibility.Visible;
                FileLoading.IsIndeterminate = true;
                var file = e.AddedItems[0] as StorageFile;

                using (var stream = await file.OpenReadAsync())
                using (var reader = new StreamReader(stream.AsStreamForRead()))
                {
                    while (!reader.EndOfStream)
                    {
                        var line = reader.ReadLine();
                        BlackBoxRecord item = BlackBoxRecord.Create(line);
                        if (item != null && item.Motor1 > 0)
                            _telemetryHistory.Add(item);
                    }
                }

                DateCreated.Text = file.DateCreated.ToString();
                if (_telemetryHistory.Count > 0)
                {
                    FileSize.Text = _telemetryHistory.Last().TimeStamp.ToString() + "(" + _telemetryHistory.Count + ")";
                    _playbackTimer.Start();
                }
                else
                    FileSize.Text = "NO SAMPLE POINTS";

                FileLoading.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                FileLoading.IsIndeterminate = false;
               
                _index = 0;
            }

            BBDataFiles.SelectedIndex = -1;
        }

        private void backButton_Click_1(object sender, RoutedEventArgs e)
        {
            Window.Current.Content = App.TheApp.Frame;
        }

        protected override void MessageArrived(NiVek.Common.Comms.IncomingMessage msg)
        {
         
        }

        protected override void SetConnectionStatus(NiVek.Common.Comms.Common.ConnectionStates status)
        {
         
        }
    }
}
