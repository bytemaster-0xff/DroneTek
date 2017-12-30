using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class BlackBoxFile
    {
        public class PID
        {
            public double P { get; set; }
            public double I { get; set; }
            public double D { get; set; }
            public string DFilter { get; set; }
            public double IMax { get; set; }
            public double IDecay { get; set; }

            static string resolveFilter(int filter)
            {
                switch (filter)
                {
                    case 0: return "NONE";
                    case 1: return "10Hz";
                    case 2: return "15Hz";
                    case 3: return "20Hz";
                    case 4: return "25Hz";
                    case 5: return "30Hz";
                    case 6: return "35Hz";
                    case 7: return "40Hz";
                }

                return "?";
            }

            public static PID Create(String[] parts)
            {
                var pid = new PID();
                pid.P = Convert.ToDouble(parts[1]);
                pid.I = Convert.ToDouble(parts[2]);
                pid.D = Convert.ToDouble(parts[3]);
                pid.DFilter = resolveFilter(Convert.ToInt32(parts[4]));
                pid.IMax = Convert.ToDouble(parts[5]);
                pid.IDecay = Convert.ToDouble(parts[6]);

                return pid;
            }
        }

        public BlackBoxFile()
        {
            Records = new List<BlackBoxRecord>();
        }

        public int Rating { get; set; }

        public DateTime DateStamp { get; private set; }
        public TimeSpan Length { get; private set; }

        public double ESC1 { get; private set; }
        public double ESC2 { get; private set; }
        public double ESC3 { get; private set; }
        public double ESC4 { get; private set; }

        public double InitialAltitude { get; private set; }
        public double InitialThrottle { get; private set; }

        public PID PitchPID { get; private set; }
        public PID RollPID { get; private set; }
        public PID YawPID { get; private set; }
        public PID AltitudePID { get; private set; }

        public List<BlackBoxRecord> Records { get; private set; }

        public List<string> Notes { get; set; }

        public int RecordCount { get { return Records.Count; } }

        public void ParseLine(string line)
        {
            var parts = line.Split(',');
            switch (parts[0])
            {
                case "1": DateStamp = DateTime.Parse(parts[1]); break;
                case "10": Records.Add(BlackBoxRecord.Create(line)); break;
                case "50": PitchPID = PID.Create(parts); break;
                case "51": RollPID = PID.Create(parts); break;
                case "52": YawPID = PID.Create(parts); break;
                case "53": AltitudePID = PID.Create(parts); break;
                case "54":
                    InitialAltitude = Convert.ToDouble(parts[1]);
                    InitialThrottle = Convert.ToDouble(parts[2]);
                    break;
                case "55":
                    ESC1 = Convert.ToDouble(parts[1]);
                    ESC2 = Convert.ToDouble(parts[2]);
                    ESC3 = Convert.ToDouble(parts[3]);
                    ESC4 = Convert.ToDouble(parts[4]);
                    break;
                
            }
        }
    }
}
