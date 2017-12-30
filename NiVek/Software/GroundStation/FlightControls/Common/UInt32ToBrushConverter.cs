using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Media;

namespace NiVek.FlightControls.Common
{
    public sealed class UInt32ToBrushConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            UInt32 colorValue = (UInt32)value;
            if (colorValue > 0)
            {
                byte a = (byte)((colorValue >> 24) & 0xFF);
                byte r = (byte)((colorValue >> 16) & 0xFF);
                byte g = (byte)((colorValue >> 8) & 0xFF);
                byte b = (byte)(colorValue & 0xFF);

                var clr = Color.FromArgb(a, r, g, b);

                return new SolidColorBrush(clr);
            }
            else
                return new SolidColorBrush(Colors.Transparent);
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }
    }
}
