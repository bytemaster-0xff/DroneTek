using NiVek.Rover.Comms;
using System;
using System.Collections.Generic;
using System.Text;
using Windows.UI.Xaml.Data;

namespace NiVek.Rover
{
    public class UIConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string culture)
        {
            switch (parameter as string)//parameter tells us who's calling 
            {
                //BluetoothConnectionState 
                case "BluetoothConnect": return ((BluetoothConnectionState)value == BluetoothConnectionState.Disconnected);
                case "BluetoothInProgress":
                case "BluetoothConnecting": return ((BluetoothConnectionState)value == BluetoothConnectionState.Connecting);
                case "BluetoothDisconnect": return ((BluetoothConnectionState)value == BluetoothConnectionState.Connected);
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string culture)
        {
            return null;
        }
    } 

}
