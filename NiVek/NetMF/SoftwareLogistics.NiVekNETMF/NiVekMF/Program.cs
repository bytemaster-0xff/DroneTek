using System;
using Microsoft.SPOT;
using System.Threading;
using SoftwareLogistics.NiVekNETMF;

namespace NiVekMF
{
    public class Program
    {
        public static void Main()
        {
            var led = new LED();

            while (true)
            {
                Debug.Print("TESTING");
                Thread.Sleep(500);
                led.SetLEDState(0, 5);
            }
        }

    }
}
