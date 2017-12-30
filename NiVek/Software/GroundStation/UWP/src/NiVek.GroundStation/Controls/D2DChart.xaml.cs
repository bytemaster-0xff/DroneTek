using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.ApplicationModel.Core;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public sealed partial class D2DChart : UserControl
    {
//        D2DWrapper _wrapper;
        
        public D2DChart()
        {
            this.InitializeComponent();

            if (!Windows.ApplicationModel.DesignMode.DesignModeEnabled)
            {
  //              _wrapper = new D2DWrapper();

    //            _wrapper.Initialize(CoreApplication.MainView.CoreWindow);

                RenderSomething();
            }
        }

        public static byte[] ConvertStreamTobyte(Stream input)
        {
            byte[] buffer = new byte[16 * 1024];

            using (MemoryStream ms = new MemoryStream())
            {
                int read;

                while ((read = input.Read(buffer, 0, buffer.Length)) > 0)
                {
                    ms.Write(buffer, 0, read);
                }

                return ms.ToArray();
            }
        } 

        private void RenderSomething()
        {
            var points = new float[] 
            {
                10,10,
                11,12,
                12,14,
                13,18,
                14,20,
                15,40,
                200,40,
                200,200,
                40,200,
                15,15
            };

            /*_wrapper.BeginDrawChart(800,800);
            _wrapper.DrawLine(points, points.Length, Colors.Red);
            _wrapper.EndDrawChart();*/

            /*using (var randStream = _wrapper.GetImageStream().CloneStream())
            {
                var bmp = new BitmapImage();
                bmp.SetSource(randStream);

                loImage.Source = bmp;
            }*/
        }
    }
}
