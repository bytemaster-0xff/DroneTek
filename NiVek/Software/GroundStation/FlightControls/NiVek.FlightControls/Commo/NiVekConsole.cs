using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Core;

namespace NiVek.FlightControls.Commo
{
    class NiVekConsole
    {
        StreamSocket _socket;
        DataReader _reader;
        DataWriter _writer;
        bool _running;

        StringBuilder _stringBuffer;

        public event EventHandler<Models.GPSUpdatedEventArgs> MsgReceived;

        public async void Connect()
        {
            _socket = new StreamSocket();

            _stringBuffer = new StringBuilder();

            await _socket.ConnectAsync(new HostName("10.1.1.177"), "8080");
            _writer = new DataWriter(_socket.OutputStream);
            _reader = new DataReader(_socket.InputStream);
            _reader.InputStreamOptions = InputStreamOptions.Partial;
            _running = true;

            while (_running)
            {
                var msgBytes = await _reader.LoadAsync(512);
                var msg = _reader.ReadString(msgBytes);
                var parts = msg.Split(',');
                if (parts.Length == 13)
                {
                    try
                    {
                        var snsrUpdate = new Models.SensorUpdate()
                        {
                            AngX = Convert.ToSingle(parts[2]),
                            AccX = Convert.ToSingle(parts[3]),
                            AngXRate = Convert.ToSingle(parts[4]),
                            AngY = Convert.ToSingle(parts[5]),
                            AccY = Convert.ToSingle(parts[6]),
                            AngYRate = Convert.ToSingle(parts[7]),
                            RCIn1 = Convert.ToInt32(parts[9]),
                            RCIn2 = Convert.ToInt32(parts[10]),
                            RCIn3 = Convert.ToInt32(parts[11]),
                            RCIn4 = Convert.ToInt32(parts[12])
                        };

                        MsgReceived(this, new Models.GPSUpdatedEventArgs(snsrUpdate));
                    }
                    catch (Exception)
                    { }
                }



                Debug.WriteLine(msg);
                
/*                foreach (var ch in msg.ToCharArray())
                {
                    if(ch == '{')
                        _stringBuffer.Clear();

                    _stringBuffer.Append(ch);

                    if(ch == '}')
                    {
                        if (_stringBuffer.ToString().Contains("sensor"))
                        {
                            var record = Parse(_stringBuffer.ToString());
                            if (record != null)
                                MsgReceived(this, new Models.GPSUpdatedEventArgs(record));
                        }
                        else
                            Debug.WriteLine(_stringBuffer.ToString());
                        _stringBuffer.Clear();
                    }
                }*/
            }
        }

        Models.SensorUpdate Parse(String json)
        {
           json = json.Replace('\'', '"');
            var serializer = new System.Runtime.Serialization.Json.DataContractJsonSerializer(typeof(Models.SensorUpdate));
            using(var ms = new MemoryStream(System.Text.UTF8Encoding.UTF8.GetBytes(json)))
            {
                try
                {
                    return serializer.ReadObject(ms) as Models.SensorUpdate;
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }

        public void Close()
        {
            _socket.Dispose();
        }

        public async void Send(String msg)
        {
            _writer.WriteString(msg);
            await _writer.StoreAsync();
        }

    }
}
