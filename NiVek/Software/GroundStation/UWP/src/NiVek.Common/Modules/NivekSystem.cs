using System;
using System.Collections.Generic;
using System.Linq;

namespace NiVek.Common.Modules
{
    public class NivekSystem
    {
        public const int NiVekNameFixedSize = 24;

        public const int Ping = 100;
        public const int WelcomePing = 101;
        public const int Pong = 110;
        
        public const int SetName = 200;
        public const int GetName = 201;
    }
}