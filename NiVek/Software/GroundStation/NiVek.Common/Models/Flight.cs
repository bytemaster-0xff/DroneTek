using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    [DataContract(Name="flight")]
    public class Flight : GalaSoft.MvvmLight.ViewModelBase
    {
   
        [DataMember(Name="id")]
        public Guid Id { get; set; }

        [DataMember(Name = "start")]
        public DateTime Start { get; set; }

        [DataMember(Name = "end")]
        public DateTime End { get; set; }
    }
}
