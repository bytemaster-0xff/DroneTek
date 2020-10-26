using NiVek.FlightControls.NiVekMath;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Shapes;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public enum LineOrigin
    {
        Bottom,
        Center,
        BottomQuarter,
        TopQuarter,
    }

    public class Axis
    {
        private int _channelIndex;
        private NiVek.DrawingPanel _drawingPanel;

        private int _width;
        private int _height;
        
        public void Init(int channelIndex, DrawingPanel panel)
        {
            _width = (int)panel.ActualWidth;
            _height = (int)panel.ActualHeight;

            _channelIndex = channelIndex;
            _drawingPanel = panel;

            FormatString = "0.00";

            _visibility = Windows.UI.Xaml.Visibility.Visible;

            _drawingPanel.SetChannelColor(_channelIndex, Color);
        }

        public void RefreshHeight(double height)
        {
            _height = (int)height;
        }

        public Color Color {get; set; }
        public LineOrigin Origin { get; set; }
        public double Scale { get; set; }

        private double _lastPoint;

        private Visibility _visibility;
        public Visibility Visibility
        {
            set
            {
                if(_drawingPanel != null)
                    _drawingPanel.SetChannelVisibility(_channelIndex, value == Windows.UI.Xaml.Visibility.Visible);
                _visibility = value;
            }
            get { return _visibility; }
        }

        Queue<Double> _chartPoints = new Queue<Double>();

        public void AddPoint(Double y)
        {
            var chartMiddle = Height / 2;

            switch (Origin)
            {
                case LineOrigin.Bottom:
                    y = Height - ((y / Scale) * Height);
                    break;
                case LineOrigin.BottomQuarter:
                    y = chartMiddle + (((y / Scale) * (Height / 2)) + (chartMiddle / 2));
                    break;
                case LineOrigin.Center:
                    y = chartMiddle - ((y / Scale) * (Height / 2));
                    break;
                case LineOrigin.TopQuarter:
                    y = chartMiddle - (((y / Scale) * (Height / 2)) - (chartMiddle / 2));
                    break;
            }

            _lastPoint = y;
        }

        public async void UpdateChart(double segmentWidth)
        {
            _chartPoints.Enqueue(_lastPoint);

            using (var stream = new Windows.Storage.Streams.InMemoryRandomAccessStream())
            {
                var numberPoints = 0;

                // Create the data writer object backed by the in-memory stream. 
                using (var dataWriter = new Windows.Storage.Streams.DataWriter(stream))
                {
                    dataWriter.UnicodeEncoding = Windows.Storage.Streams.UnicodeEncoding.Utf8;
                    dataWriter.ByteOrder = Windows.Storage.Streams.ByteOrder.BigEndian;

                    var x = 0.0;
                    foreach (var yPoint in _chartPoints)
                    {
                        x += segmentWidth;

                        dataWriter.WriteDouble(x);
                        dataWriter.WriteDouble(yPoint);

                        numberPoints++;
                    }


                    await dataWriter.StoreAsync();
                    await dataWriter.FlushAsync();

                    dataWriter.DetachStream();
                }

                stream.Seek(0);
                _drawingPanel.LoadFromStream(_channelIndex, stream, numberPoints);
            }

            if (_chartPoints.Count > _width / segmentWidth)
                _chartPoints.Dequeue();
        }
        

        DateTime _lastDraw = DateTime.MinValue;

        public String FormatString { get; set; }
        public String Label { get; set; }
        public double SegmentWidth { get; set; }
        public double Height { get { return _height; } set { _height = (int)value; } }
        public double Width { get { return _width; } set { _width = (int)value; } }
    }


    public partial class Chart : UserControl
    {
        List<Axis> _axis = new List<Axis>();
        TextBlock[] _valueTextBoxes;
        TextBlock[] _labelTextBoxes;
        double?[] _lastValues;
        bool _isInitialized = false;
        String[] _formatStrings;
        public StandardDeviation _stdDeviation;

        public Chart()
        {
            this.InitializeComponent();

            _stdDeviation = new StandardDeviation();

            this.Loaded += Chart_Loaded;
        }

        void Chart_LayoutUpdated(object sender, object e)
        {
            RefreshHeight();
        }

        public void Start()
        {
            ChartDrawingSurface.StartProcessingInput();
            ChartDrawingSurface.StartRenderLoop();
        }


        public void Stop()
        {
            ChartDrawingSurface.StopProcessingInput();
            ChartDrawingSurface.StopRenderLoop();
        }

        public void RefreshHeight()
        {
            foreach (var axis in _axis)
                axis.RefreshHeight(this.ActualHeight);

            Debug.WriteLine("New actual height: " + this.ActualHeight);

            DrawChartBackground();
        }

        #region DrawChartBackground
        public void DrawChartBackground()
        {
            var delta = 12.5f;
            var x = delta + 6;
            bool alt = true;
            while (x < ChartDrawingSurface.ActualWidth)
            {
                var angleLine = new Line() { X1 = x, X2 = x, StrokeThickness = 1, Stroke = new SolidColorBrush(Colors.DarkGray), Opacity = 0.5 };
                if (alt)
                {
                    angleLine.Y1 = 6;
                    angleLine.Y2 = ChartContents.ActualHeight - 6;
                }
                else
                {
                    var offset = ChartContents.ActualHeight / 2 - 20;

                    angleLine.Y1 = offset;
                    angleLine.Y2 = ChartContents.ActualHeight - offset;
                }

                alt = !alt;
                
                ChartContents.Children.Add(angleLine);

                x += delta;
            }

            var centerLine = new Line() { X1 = 6, X2 = ChartDrawingSurface.ActualWidth + 6, Y1 = ChartContents.ActualHeight / 2, Y2 = ChartContents.ActualHeight / 2, Stroke = new SolidColorBrush(Colors.DarkGray), Opacity = 0.8 };
            ChartContents.Children.Add(centerLine);
        }
        #endregion

        public String Caption { get; set; }

        public double SegmentWidth { get; set; }

        SolidColorBrush BlackBrush = new SolidColorBrush(Colors.Black);

        void Chart_Loaded(object sender, RoutedEventArgs e)
        {
            Debug.WriteLine("NUMBER AXIS" + ChartAxis.Count);
            var idx = 0;

            _lastValues = new double?[ChartAxis.Count];
            _valueTextBoxes = new TextBlock[ChartAxis.Count];
            _labelTextBoxes = new TextBlock[ChartAxis.Count];

            _formatStrings = new String[ChartAxis.Count];

            ChartCaption.Text = this.Caption;

            DrawChartBackground();

            if (SegmentWidth < 1)
                SegmentWidth = 1;

            Legend.Children.Clear();

            foreach (var axis in ChartAxis)
            {
                axis.Width = ChartDrawingSurface.ActualWidth - 20;

                var row = new StackPanel();
                row.Orientation = Orientation.Horizontal;

                _labelTextBoxes[idx] = new TextBlock() { Text = axis.Label, Width = 50, FontSize = 15, Foreground = new SolidColorBrush(_axis[idx].Color) };
                row.Children.Add(_labelTextBoxes[idx]);

                _valueTextBoxes[idx] = new TextBlock() { FontSize = 14, Width = 70, Foreground = BlackBrush, TextAlignment = TextAlignment.Right, Text = "-" };
                row.Children.Add(_valueTextBoxes[idx]);

                Legend.Children.Add(row);

                axis.SegmentWidth = SegmentWidth;

                _lastValues[idx] = null;
                _formatStrings[idx] = "{0:" + axis.FormatString + "}";
                axis.Init(idx, ChartDrawingSurface);

                idx++;                
            }

            ChartDrawingSurface.SetNumberChannels(ChartAxis.Count);
            
            _isInitialized = true;
        }

        public void Refresh()
        {
            int idx = 0;
            foreach (var axis in ChartAxis)
            {
                _lastValues[idx] = null;
                _formatStrings[idx] = "{0:" + axis.FormatString + "}";
                _labelTextBoxes[idx].Text = axis.Label;
            }

            _stdDeviation.Reset();
            //ClearChart();
        }

        public List<Axis> ChartAxis { get { return _axis; } set { _axis = value; } }

        public void ClearChart()
        {
            for (var idx = 0; idx < _lastValues.Length; ++idx)
                _lastValues[idx] = null;
        }
        DateTime _lastAdd = DateTime.MinValue;

        public double _lastXValue = 0.0;

        public void AddDataPoint(params double[] args)
        {
            if (!_isInitialized)
                return;

            if (_lastAdd == DateTime.MinValue)
            {
                _lastAdd = DateTime.Now;
                return;
            }

            TimeSpan deltaT = DateTime.Now - _lastAdd;
            _lastAdd = DateTime.Now;

            _stdDeviation.Add(args[0]);

            StdDeviation.Text = String.Format("StdDev = {0:0.00}", _stdDeviation.Deviation);

            for (var idx = 0; idx < args.Length; ++idx)
            {
                Debug.WriteLine($"VALUE => {args[idx]} - {SegmentWidth}");

                _valueTextBoxes[idx].Text = String.Format(_formatStrings[idx], args[idx]);
                ChartAxis[idx].AddPoint(args[idx]);
                ChartAxis[idx].UpdateChart(SegmentWidth);
            }
        }

        private void ResetSTDDev_Click_1(object sender, RoutedEventArgs e)
        {
            _stdDeviation.Reset();
        }

    }
}
