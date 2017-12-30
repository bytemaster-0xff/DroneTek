using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    [DataContract(Name="user")]
    public class User
    {
        [JsonProperty(PropertyName="id")]
        public String Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName="user_name")]
        public String UserName
        {
            get;
            set;
        }

        private static User _user;
        public static User Current
        {
            get
            {
                if(_user == null)
                {
                    _user = new User
                    {
                        Id = new Guid("{D2A6858B-7F09-4E6A-9747-42BD5BB7CFC5}").ToString(),
                        UserName = "El_ByteMaster"
                    };
                }

                return _user;
            }
        }
    }
}
