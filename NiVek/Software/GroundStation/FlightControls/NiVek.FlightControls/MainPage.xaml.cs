using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using NiVek.FlightControls.Commo;
using NiVek.FlightControls.Models;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.UI.Xaml.Shapes;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace NiVek.FlightControls
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        NiVekConsole _common;

        List<CalibrationPoint> _calibrationPoints = CalibrationPoint.Get();

        public MainPage()
        {
            this.InitializeComponent();

            _common = new NiVekConsole();
            _common.MsgReceived += _common_MsgReceived;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            _common.Connect();
        }

        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            _common.Close();
        }

        

        async void _common_MsgReceived(object sender, Models.GPSUpdatedEventArgs e)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                DataContext = e.GPSData;
                GraphData(e.GPSData);
            });
        }

        int x = 0;
        Models.SensorUpdate _lastUpdate;

        public void GraphData(Models.SensorUpdate update)
        {
            DataContext = update;

            x = x + 2;

            update.X = x;

            if (_lastUpdate != null)
            {
                Line angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AngX * 2 + 75;
                angleLine.Y2 = update.AngX * 2 + 75;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightYellow);

                XAxis.Children.Add(angleLine);

                angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AccX * 2 + 225;
                angleLine.Y2 = update.AccX * 2 + 225;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightGreen);

                XAxis.Children.Add(angleLine);

                angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AngXRate + 150;
                angleLine.Y2 = update.AngXRate + 150;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightCyan);

                XAxis.Children.Add(angleLine);

                angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AngY * 2 + 75;
                angleLine.Y2 = update.AngY * 2 + 75;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightYellow);

                YAxis.Children.Add(angleLine);

                angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AccY * 2 + 225;
                angleLine.Y2 = update.AccY * 2 + 225;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightGreen);

                YAxis.Children.Add(angleLine);

                angleLine = new Line();
                angleLine.X1 = _lastUpdate.X;
                angleLine.X2 = update.X + 1.0f;
                angleLine.Y1 = _lastUpdate.AngYRate + 150;
                angleLine.Y2 = update.AngYRate + 150;
                angleLine.StrokeThickness = 1;
                angleLine.Stroke = new SolidColorBrush(Colors.LightCyan);

                YAxis.Children.Add(angleLine);
            }

            if (x > XAxis.ActualWidth)
            {
                _lastUpdate = null;
                YAxis.Children.Clear();
                XAxis.Children.Clear();
                x = 0;
            }
            else
                _lastUpdate = update;
        }


        int _calibrationIndex;
        private void Capture_Click_1(object sender, RoutedEventArgs e)
        {
            if (_calibrationIndex > 0)
                _common.Send("{'msgType':'gpio','action':'calibrateStick-" + _calibrationPoints[_calibrationIndex - 1].Code + "'}");              

            if (_calibrationIndex > _calibrationPoints.Count - 1)
            {
                CalibrateStick.Content = "Begin";
                _calibrationIndex = 0;
            }
            CalibrateStick.Content = "Capture";
            CurrentFunction.Text = _calibrationPoints[_calibrationIndex++].Name;
        }

    }
}
