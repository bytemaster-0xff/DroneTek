using System;
using System.Collections.Generic;
using System.Text;
using NiVek.Common.Models;
using System.Threading.Tasks;

namespace NiVek.Common.Services
{
    public interface IStorageContext
    {
        Task InsertCommsAsync(DroneComms comms);
        Task UpdateCommsAsync(DroneComms comms);
    }
}
