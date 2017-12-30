using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class MotorStatus : GalaSoft.MvvmLight.ViewModelBase
    {
        public static MotorStatus Create(byte[] buffer)
        {
            var motorStatus = new MotorStatus();
            motorStatus.Update(buffer);
            return motorStatus;
        }

        public void Update(byte[] buffer)
        {
            try
            {
                var byteIndex = 0;

                PowerPortFront = buffer[byteIndex++];
                PowerPortRear = buffer[byteIndex++];
                PowerStarboardFront = buffer[byteIndex++];
                PowerStarboardRear = buffer[byteIndex++];
                IsDataReady = true;

                RaisePropertyChanged(() => PowerPort);
                RaisePropertyChanged(() => PowerStarboard);
                RaisePropertyChanged(() => PowerRear);
                RaisePropertyChanged(() => PowerFront);

                RaisePropertyChanged(() => PowerPortFront);
                RaisePropertyChanged(() => PowerPortRear);
                RaisePropertyChanged(() => PowerStarboardFront);
                RaisePropertyChanged(() => PowerStarboardRear);

                RaisePropertyChanged(() => PowerPortPct);
                RaisePropertyChanged(() => PowerStarboardPct);
                RaisePropertyChanged(() => PowerRearPct);
                RaisePropertyChanged(() => PowerFrontPct);

                RaisePropertyChanged(() => PowerPortFrontPct);
                RaisePropertyChanged(() => PowerPortRearPct);
                RaisePropertyChanged(() => PowerStarboardFrontPct);
                RaisePropertyChanged(() => PowerStarboardRearPct);
            }
            catch (Exception)
            {
                IsDataReady = false;
            }
        }

        private bool _isDataReady = false;
        public bool IsDataReady { get { return _isDataReady; } set { Set(ref _isDataReady, value); } }

        public short PowerPortFront { get; set; }
        public string PowerPortFrontPct { get { return String.Format("{0}%", PowerPortFront * 100 / 255); } }
        public short PowerPortRear { get; set; }
        public string PowerPortRearPct { get { return String.Format("{0}%", PowerPortRear * 100 / 255); } }
        public short PowerStarboardRear { get; set; }
        public string PowerStarboardRearPct { get { return String.Format("{0}%", PowerStarboardRear * 100 / 255); } }
        public short PowerStarboardFront { get; set; }
        public string PowerStarboardFrontPct { get { return String.Format("{0}%", PowerStarboardFront * 100 / 255); } }


        public short PowerFront { get { return PowerPortFront; } }
        public string PowerFrontPct { get { return String.Format("{0}%", PowerPortFront * 100 / 255); } }        
        public short PowerStarboard { get { return PowerStarboardFront; } }
        public string PowerStarboardPct { get { return String.Format("{0}%", PowerStarboardFront * 100 / 255); } }
        public short PowerPort { get { return PowerPortRear; } }
        public string PowerPortPct { get { return String.Format("{0}%", PowerPortRear * 100 / 255); } }
        public short PowerRear { get { return PowerStarboardRear; } }
        public string PowerRearPct { get { return String.Format("{0}%", PowerStarboardRear * 100 / 255); } }
    }
}
