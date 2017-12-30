using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.UI.Xaml.Shapes;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public sealed partial class Altitude : UserControl
    {
        public Altitude()
        {
            this.InitializeComponent();
            BuildScale();
        }

        private void BuildScale()
        {
            for (var idx = 0; idx < 100; ++idx)
                AltitudeScale.RowDefinitions.Add(new RowDefinition() { Height = GridLength.Auto });

            for (var idx = 0; idx < 100; idx++)
            {
                var txt = new TextBlock();
                txt.SetValue(Grid.RowProperty, idx);
                txt.Text = String.Format("{0:0.0}", ((100-idx) * 5.0f) / 10.0f);
                txt.Height = 30;

                AltitudeScale.Children.Add(txt);

                var seperator = new Line();
                seperator.X1 = 0; seperator.X2 = 35;
                seperator.Y1 = 0; seperator.Y2 = 0;
                seperator.SetValue(Grid.RowProperty, idx);
            }
        }

        public float AltitudeCM
        {
            set
            {
                if(value < 10.0)
                    AltitudeText.Text = String.Format("{0:0.0}m", Math.Round(value * 5.0f, MidpointRounding.AwayFromZero) / 5.0f);
                else
                    AltitudeText.Text = String.Format("{0}m", value);

                Canvas.SetTop(AltitudeScale, -780 + (((30.0f / 50.0f) * (value * 10.0)) - 15.0));
            }
            get
            {
                return Convert.ToUInt16(AltitudeText.Text);
            }
        }
    }
}
