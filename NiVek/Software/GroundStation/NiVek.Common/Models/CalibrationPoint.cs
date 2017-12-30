using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class CalibrationPoint
    {
        public String Name { get; set; }
        public byte Code { get; set; }

        public static List<CalibrationPoint> Get()
        {
            var points = new List<CalibrationPoint>(){
                new CalibrationPoint() {Code = 1, Name = "Min Throttle" },
                new CalibrationPoint() {Code = 2, Name = "Max Throttle" },

                new CalibrationPoint() {Code = 10, Name = "Pitch Up" },
                new CalibrationPoint() {Code = 11, Name = "Pitch Idle" },
                new CalibrationPoint() {Code = 12, Name = "Pitch Down" },

                new CalibrationPoint() {Code = 20, Name = "Roll Left" },
                new CalibrationPoint() {Code = 21, Name = "Roll Idle" },
                new CalibrationPoint() {Code = 22, Name = "Roll Right" },

                new CalibrationPoint() {Code = 30, Name = "Yaw Port" },
                new CalibrationPoint() {Code = 31, Name = "Yaw Idle" },
                new CalibrationPoint() {Code = 32, Name = "Yaw Starboard" }
            };

            return points;
        }
    }
}
