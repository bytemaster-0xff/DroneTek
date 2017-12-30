using NiVek.Common.Models;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Windows.Storage;
using NiVek.WinCommon.Services;

namespace NiVek.GroundStation.FlightRecorder
{
    public class BlackBox
    {
        StorageFile _outputFile;
        DateTime? _startCapture;
        Stream _outputStream;
        StreamWriter _outputStreamWriter;
        int _pointsWritten;

        public GPIOConfig Configuration
        {
            get;
            set;
        }

        public GPSData GpsData
        {
            get;
            set;
        }

        public Targets Targets
        {
            get;
            set;
        }

        public MotorStatus Motors
        {
            get;
            set;
        }

        public String FlightEnd
        {
            get;
            set;
        }

        public async void OpenFile(StorageFile file)
        {
            var blackBoxFile = new BlackBoxFile();

            using (var stream = await file.OpenReadAsync())
            using (var reader = new StreamReader(stream.AsStreamForRead()))
            {
                while (!reader.EndOfStream)
                {
                    blackBoxFile.ParseLine(reader.ReadLine());
                }
            }

            var fileLength = TimeSpan.FromMilliseconds(blackBoxFile.Records.Last().TimeStamp);
            FlightEnd = String.Format("{0:00}:{1:00}.{2:000}", fileLength.Minutes, fileLength.Seconds, fileLength.Milliseconds);
        }

        public async void StartRecording()
        {
            try
            {
                if (_outputFile != null)
                    return;

                _startCapture = DateTime.Now;

                var now = DateTime.Now;

                var nivekName = String.IsNullOrEmpty(DroneHub.Instance.Active.DroneName) ? "unamed" : DroneHub.Instance.Active.DroneName;
                var fileName = String.Format("{6} {0}.{1}.{2} {3}.{4:00}.{5:00}.nivek", now.Month, now.Day, now.Year, now.Hour, now.Minute, now.Second, nivekName);

                Debug.WriteLine("Creating black box output file: " + fileName);

                String frameConfig = "?";
                if (Configuration.FrameConfig == (byte)GPIOConfig.FrameConfigTypes.X)
                    frameConfig = "X";

                if (Configuration.FrameConfig == (byte)GPIOConfig.FrameConfigTypes.Cross)
                    frameConfig = "+";

                _outputFile = await Windows.Storage.ApplicationData.Current.RoamingFolder.CreateFileAsync(fileName);
                _outputStream = await _outputFile.OpenStreamForWriteAsync();
                _outputStreamWriter = new StreamWriter(_outputStream);
                _outputStreamWriter.AutoFlush = false;
                _outputStreamWriter.WriteLine("1," + DateTime.Now);
                _outputStreamWriter.WriteLine("50," + Configuration.PitchRateP + "," + Configuration.PitchRateI + "," + Configuration.PitchRateD + "," + Configuration.PitchDFilter + "," + Configuration.PitchRateIMax + "," + Configuration.PitchRateIDecayFactor);
                _outputStreamWriter.WriteLine("51," + Configuration.RollRateP + "," + Configuration.RollRateI + "," + Configuration.RollRateD + "," + Configuration.RollDFilter + "," + Configuration.RollRateIMax + "," + Configuration.RollRateIDecayFactor);
                _outputStreamWriter.WriteLine("52," + Configuration.YawRateP + "," + Configuration.YawRateI + "," + Configuration.YawRateD + "," + Configuration.YawDFilter + "," + Configuration.YawRateIMax + "," + Configuration.YawRateIDecayFactor);
                _outputStreamWriter.WriteLine("53," + Configuration.AltitudeRateP + "," + Configuration.AltitudeRateI + "," + Configuration.AltitudeRateD + "," + Configuration.AltitudeDFilter + "," + Configuration.AltitudeRateIMax + "," + Configuration.AltitudeRateIDecayFactor);
                _outputStreamWriter.WriteLine("54," + Configuration.InitialAltitude + "," + Configuration.InitialThrottle);
                _outputStreamWriter.WriteLine("55," + Configuration.EscPortFront + "," + Configuration.EscStarboardFront + "," + Configuration.EscStarboardRear + "," + Configuration.EscPortRear);
                _outputStreamWriter.WriteLine("56," + frameConfig);
                _outputStreamWriter.WriteLine("57," + DroneHub.Instance.Active.DroneName);

                Debug.WriteLine("Created black box output file: " + fileName);

                _pointsWritten = 0;
            }
            catch (Exception ex)
            {
                Debug.WriteLine("ERROR OPENING FILE: " + ex.Message);

                if (_outputStream != null)
                    _outputStream.Dispose();

                _outputStream = null;

                if (_outputStreamWriter != null)
                    _outputStreamWriter.Dispose();

                _outputStreamWriter = null;

                _outputFile = null;
            }
        }

        private void WriteData(NiVek.Common.Models.SensorUpdate update)
        {
            if (update != null && Motors != null && _outputStreamWriter != null && _startCapture.HasValue)
            {
                _pointsWritten++;
                var ms = Convert.ToInt32((DateTime.Now - _startCapture.Value).TotalMilliseconds);

                if (GpsData != null && GpsData.ValidFix)
                    _outputStreamWriter.Write(String.Format("10,{0:0.00},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{12},{13},{14},{15}\r\n",
                        ms,
                        update.PitchAngle, update.RollAngle, update.Heading, update.AltitudeM,
                        Targets.PitchIn, Targets.RollIn, Targets.TargetAltitude, Targets.TargetHeading,
                        Targets.ThrottlePct, Motors.PowerFront, Motors.PowerStarboard, Motors.PowerRear, Motors.PowerPort,
                        GpsData.Longitude, GpsData.Longitude));
                else
                    _outputStreamWriter.Write(String.Format("10,{0:0.00},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},?,?\r\n",
                        ms,
                        update.PitchAngle, update.RollAngle, update.Heading, update.AltitudeM,
                        Targets.PitchIn, Targets.RollIn, Targets.TargetAltitude, Targets.TargetHeading,
                        Targets.ThrottlePct, Motors.PowerFront, Motors.PowerStarboard, Motors.PowerRear, Motors.PowerPort));
            }

        }


        public void StopRecording()
        {
            if (_outputFile != null)
                Debug.WriteLine("Done recording " + _outputFile.Name + " with " + _pointsWritten);
            else
                Debug.WriteLine("Done recording, but no file :(");

            if (_outputStreamWriter != null)
            {
                _outputStreamWriter.Flush();
                _outputStreamWriter.Dispose();
                _outputStreamWriter = null;
            }

            if (_outputStream != null)
            {
                _outputStream.Dispose();
                _outputStream = null;
            }
            _outputFile = null;
        }

    }
}
