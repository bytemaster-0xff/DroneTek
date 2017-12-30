using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Utils
{
    public  class ColorManager
    {
        public class Color
        {
            public Byte Alpha { get; set; }
            public Byte Red { get; set; }
            public Byte Green { get; set; }
            public Byte Blue { get; set; }            
        }

        public static Color Red { get { return new Color() { Alpha = 255, Red = 255, Blue = 0, Green = 0 }; }}
        public static Color Blue { get { return new Color() { Alpha = 255, Red = 0, Blue = 255, Green = 0 }; } }
        public static Color Green { get { return new Color() { Alpha = 255, Red = 0, Blue = 0, Green = 255 }; } }
        public static Color Yellow { get { return new Color() { Alpha = 255, Red = 0, Blue = 255, Green = 255 }; } }

        public static Color White { get { return new Color() { Alpha = 255, Red = 255, Blue = 255, Green = 255 }; } }

        public static Color Black { get { return new Color() { Alpha = 255, Red = 0, Blue = 0, Green = 0 }; } }
        public static Color FromARGB(Byte alpha, Byte red, Byte blue, Byte green)
        {
            return new Color()
            {
                Alpha = alpha,
                Red = red,
                Blue = blue,
                Green = green
            };
        }
    }
}
