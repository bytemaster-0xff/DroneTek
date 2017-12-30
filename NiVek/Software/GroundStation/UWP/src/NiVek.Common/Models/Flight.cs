using LagoVista.Core.ViewModels;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    [DataContract(Name="flight")]
    public class Flight : ViewModelBase
    {
   
        [JsonProperty(PropertyName="id")]
        public Guid Id { get; set; }

        [JsonProperty(PropertyName = "start")]
        public DateTime Start { get; set; }

        [JsonProperty(PropertyName = "end")]
        public DateTime End { get; set; }
    }
}
