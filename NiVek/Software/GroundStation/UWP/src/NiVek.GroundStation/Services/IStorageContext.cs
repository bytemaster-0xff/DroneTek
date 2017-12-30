using NiVek.Common.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NiVek.Common.Models;

namespace NiVek.GroundStation.Services
{
    public class StorageContext : IStorageContext
    {
        public Task InsertCommsAsync(DroneComms comms)
        {
            return Task.FromResult(default(object));
        }

        public Task UpdateCommsAsync(DroneComms comms)
        {
            return Task.FromResult(default(object));
        }
    }
}
