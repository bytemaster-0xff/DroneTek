using System;
using System.Collections.Generic;
using System.Diagnostics;
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
    public sealed partial class Setting : UserControl
    {
        public Setting()
        {
            this.InitializeComponent();
            NumberDecimal = 2;
            DisplayFactor = 1.0;
            this.LostFocus += (a, e) =>
            {
                double newValue = Math.Round(Convert.ToDouble(Value.Text), NumberDecimal);
                {
                    Debug.WriteLine("New Value: " + newValue);
                    if (SettingValue != newValue)
                        SettingValue = newValue;
                }
            };
        }

        void Setting_LostFocus(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        public double IncrementSize { get; set; }

        public double DisplayFactor { get; set; }

        public double SettingValue
        {
            get { return (double)GetValue(SettingValueProperty); }
            set { SetValue(SettingValueProperty, value); }
        }

        public double Min
        {
            get { return (double)GetValue(MinProperty); }
            set { SetValue(MinProperty, value); }
        }

        public double Max
        {
            get { return (double)GetValue(MaxProperty); }
            set { SetValue(MaxProperty, value); }
        }

        public int NumberDecimal
        {
            get { return (int)GetValue(NumberDecimalProperty); }
            set { SetValue(NumberDecimalProperty, value); }
        }

        private void SettingsValueChanged(double value)
        {
            Value.Text = value.ToString("F" + NumberDecimal);
        }

        private string _caption;
        public String CaptionValue
        {
            get { return _caption; }
            set
            {
                Caption.Text = value;
                _caption = value;
            }
        }

        public static readonly DependencyProperty SettingValueProperty = DependencyProperty.Register("SettingValue", typeof(double), typeof(Setting), new PropertyMetadata(0, (sndr, args) =>
        {
            Setting setting = (Setting)sndr;
            setting.SettingsValueChanged((double)args.NewValue);
        }));

        public static readonly DependencyProperty MinProperty = DependencyProperty.Register("Min", typeof(double), typeof(Setting), new PropertyMetadata(0, (sndr, args) =>
        {

        }));

        public static readonly DependencyProperty MaxProperty = DependencyProperty.Register("Max", typeof(double), typeof(Setting), new PropertyMetadata(0, (sndr, args) =>
        {

        }));

        public static readonly DependencyProperty NumberDecimalProperty = DependencyProperty.Register("NumberDecimal", typeof(int), typeof(Setting), new PropertyMetadata(0, (sndr, args) =>
        {

        }));

        private void Decrement_Click_1(object sender, RoutedEventArgs e)
        {
            SettingValue = Math.Round(Convert.ToDouble(SettingValue - IncrementSize), NumberDecimal);
        }

        private void Increment_Click_1(object sender, RoutedEventArgs e)
        {
            SettingValue = Math.Round(Convert.ToDouble(SettingValue + IncrementSize), NumberDecimal);
        }
    }
}
