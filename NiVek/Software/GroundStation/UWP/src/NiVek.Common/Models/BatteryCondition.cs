using LagoVista.Core.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class BatteryCondition : ViewModelBase
    {
        public static BatteryCondition Create(byte[] buffer)
        {
            var condition = new BatteryCondition();
            condition.Update(buffer);
            return condition;
        }

        public void Update(Byte[] buffer)
        {
            int idx = 0;
            Cell1 = ((UInt16)(buffer[idx++] | buffer[idx++] << 8)) / 1000.0;
            Cell2 = ((UInt16)(buffer[idx++] | buffer[idx++] << 8)) / 1000.0;
            Cell3 = ((UInt16)(buffer[idx++] | buffer[idx++] << 8)) / 1000.0;
            RaisePropertyChanged(() => Cell1);
            RaisePropertyChanged(() => Cell2);
            RaisePropertyChanged(() => Cell3);
            RaisePropertyChanged(() => All);


            RaisePropertyChanged(() => Cell1BatteryDisplay);
            RaisePropertyChanged(() => Cell2BatteryDisplay);
            RaisePropertyChanged(() => Cell3BatteryDisplay);
            RaisePropertyChanged(() => TotalBatteryDisplay);
        }

        public double Cell1 { get; set; }
        public double Cell2 { get; set; }
        public double Cell3 { get; set; }
        public double All { get { return (Cell1 + Cell2 + Cell3); } }


        public String Cell1BatteryDisplay { get { return String.Format("{0:0.0}v", Cell1); } }
        public String Cell2BatteryDisplay { get { return String.Format("{0:0.0}v", Cell2); } }
        public String Cell3BatteryDisplay { get { return String.Format("{0:0.0}v", Cell3); } }
        public String TotalBatteryDisplay { get { return String.Format("{0:0.0}v", All); } }
    }
}
