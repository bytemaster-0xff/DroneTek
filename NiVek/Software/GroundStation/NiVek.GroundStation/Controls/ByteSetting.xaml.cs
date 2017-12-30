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
    public sealed partial class ByteSetting : UserControl
    {
        public ByteSetting()
        {
            this.InitializeComponent();
            this.LostFocus += (a, e) =>
            {
                var newValue = Convert.ToInt16(Value.Text);
                {
                    if (SettingValue != newValue)
                        SettingValue = newValue;
                }
            };
        }

        public int IncrementSize { get; set; }

        public Int16 SettingValue
        {
            get { return (Int16)GetValue(SettingValueProperty); }
            set { SetValue(SettingValueProperty, value); }
        }

        /*
                public int Min
                {
                    get { return (int)GetValue(MinProperty); }
                    set { SetValue(MinProperty, value); }
                }

                public int Max
                {
                    get { return (int)GetValue(MaxProperty); }
                    set { SetValue(MaxProperty, value); }
                }
        */
        private void SettingsValueChanged(System.Int16 value)
        {
            Value.Text = value.ToString();
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

        public static readonly DependencyProperty SettingValueProperty = DependencyProperty.Register("SettingValue", typeof(System.Int16), typeof(ByteSetting), new PropertyMetadata(0, (sndr, args) =>
        {
            ByteSetting setting = (ByteSetting)sndr;
            setting.SettingsValueChanged((System.Int16)args.NewValue);
        }));
        /*
                public static readonly DependencyProperty MinProperty = DependencyProperty.Register("Min", typeof(int), typeof(ByteSetting), new PropertyMetadata(0, (sndr, args) => 
                {

                }));

                public static readonly DependencyProperty MaxProperty = DependencyProperty.Register("Max", typeof(int), typeof(ByteSetting), new PropertyMetadata(0, (sndr, args) =>
                {

                }));
                */
        private void Decrement_Click_1(object sender, RoutedEventArgs e)
        {
            SettingValue = Convert.ToInt16(SettingValue - IncrementSize);
        }

        private void Increment_Click_1(object sender, RoutedEventArgs e)
        {
            SettingValue = Convert.ToInt16(SettingValue + IncrementSize);
        }
    }
}
