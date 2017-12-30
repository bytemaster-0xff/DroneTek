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
        [DataMember(Name="id")]
        public String Id
        {
            get;
            set;
        }

        [DataMember(Name="user_name")]
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
                    _user = new User();
                    _user.Id = new Guid("{D2A6858B-7F09-4E6A-9747-42BD5BB7CFC5}").ToString();
                    _user.UserName = "El_ByteMaster";
                }

                return _user;
            }
        }
    }
}
