using NiVek.Common.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.FlightControls.Controls
{
    public sealed partial class Attitude : UserControl
    {
//        D3DImageSource _aoaImage;
        //NiVek.D3DPanel _panel;

        float toRadians(float angle)
        {
            return (float)(angle * (Math.PI / 180.0f));
        }

        public Attitude()
        {
            this.InitializeComponent();

            AOAIndicator.StartRenderLoop();
            AOAIndicator.LoadModels();
            AOAIndicator.SetRotation(0, toRadians(90.0f), 0);

            //_aoaImage = new D3DImageSource((int)this.ActualWidth, (int)this.ActualHeight, true);
  //          _aoaImage = new D3DImageSource(400, 400, true,"BuildCrew.cmo");
    //        AOASphere.Source = _aoaImage;

          /*  _aoaImage.BeginDraw();
            _aoaImage.Clear(Colors.Black);
           * 
           * 
           * 
            _aoaImage.Render(0.0f,toRadians(90.0f),0.0f);
            _aoaImage.EndDraw();*/
        }

        public ushort AltitudeCM
        {
            set
            {
                AltitudeIndicator.AltitudeCM = value;
            }
        }

        public double Heading
        {
            set
            {
                CompassIndicator.Heading = value;
            }
        }

        public void SetPitchRoll(double pitch, double roll)
        {
            AOAIndicator.SetRotation(toRadians((float)roll), toRadians(90.0f), toRadians((float)pitch));

        /*    _aoaImage.BeginDraw();
            _aoaImage.Clear(Colors.Black);
            _aoaImage.Render(toRadians((float)roll), toRadians(90.0f), toRadians((float)pitch));
            _aoaImage.EndDraw();*/
        }

        SensorUpdate _sensorData;
        public SensorUpdate SensorData
        {
            get { return _sensorData; }
            set
            {
                _sensorData = value;
                AltitudeIndicator.AltitudeCM = value.AltitudeM;
                CompassIndicator.Heading = value.Heading;
                DataContext = value;
                AOAIndicator.SetRotation(toRadians((float)_sensorData.AttitudeRollAngle), toRadians(90.0f), toRadians((float)value.AttitudePitchAngle));
            }
        }
    }
}
