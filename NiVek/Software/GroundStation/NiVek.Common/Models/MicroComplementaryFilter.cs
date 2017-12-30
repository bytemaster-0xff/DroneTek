using System;


using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;



namespace NiVek.Common.Models
{
    public class MicroComplementaryFilter : ModelBase, IModel
    {
        private bool _isReady = false;

        public const byte CompFilterSetting = 1;

        public void Update(BufferReader rdr)
        {
            rdr.Endian = BufferReader.EndianType.LittleEndian;

            _gyroAccCompFilter = rdr.ReadShort() / 100.0;
            _gyroMagCompFilter = rdr.ReadShort() / 100.0;
            _accBaroCompFilter = rdr.ReadShort() / 100.0;

            _isReady = true;
        }

        public bool IsReady
        {
            get { return _isReady; }
        }

        private double _gyroAccCompFilter;
        private double _gyroMagCompFilter;
        private double _accBaroCompFilter;

        public double GyroAccCompFilter
        {
            get { return _gyroAccCompFilter; }
            set
            {
                if (_gyroAccCompFilter != value)
                {
                    _gyroAccCompFilter = value;
                    Send(SensorIds.IMU_Attitude, CompFilterSetting, (short)(_gyroAccCompFilter * 100));
                    OnPropertyChanged(() => GyroAccCompFilter);
                }
            }
        }

        public double GyroMagCompFilter
        {
            get { return _gyroMagCompFilter; }
            set
            {
                if (_gyroMagCompFilter != value)
                {
                    _gyroMagCompFilter = value;
                    Send(SensorIds.IMU_Heading, CompFilterSetting, (short)(_gyroMagCompFilter * 100));
                    OnPropertyChanged(() => GyroMagCompFilter);
                }
            }
        }

        public double AccBaroCompFilter
        {
            get { return _accBaroCompFilter; }
            set
            {
                if (_accBaroCompFilter != value)
                {
                    _accBaroCompFilter = value;
                    Send(SensorIds.IMU_Altitude, CompFilterSetting, (short)(_accBaroCompFilter * 100));
                    OnPropertyChanged(() => AccBaroCompFilter);
                }
            }
        }
    }
}
