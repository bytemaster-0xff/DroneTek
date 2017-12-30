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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public sealed partial class CompassHeader : UserControl
    {
        public CompassHeader()
        {
            this.InitializeComponent();
        }

        public double Heading
        {
            set
            {
                CurrentHeading.Text = String.Format("{0:000.0}",value);
                
                Canvas.SetLeft(CompassScale, -250);// + (value * 2) );                
                Canvas.SetLeft(CompassScale, -70);// + (value * 2) );
                Canvas.SetLeft(CompassScale, -250 - 180);// + (value * 2) )
                Canvas.SetLeft(CompassScale, -250 - 360);// + (value * 2) )
                Canvas.SetLeft(CompassScale, -70 - value * 2);// + (value * 2) );
            }
        }
    }
}
