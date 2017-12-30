using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class PIDTuningDetails
    {
        public enum AxisTypes
        {
            Pitch,
            Roll,
            Yaw,
            Altitude,
        }

        
        public static PIDTuningDetails Create(byte[] buffer)
        {
            try
            {
                var pid = new PIDTuningDetails();
                var byteIndex = 0;

                pid.ErrorSigma = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0f;
                pid.Derivative = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0f;
                
                pid.TargetRate = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0f;
                pid.Rate = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 100.0f; 
                
                pid.P_SteadyComponent = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;
                pid.I_SteadyComponent = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;

                pid.P_RateComponent = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 1000.0f;
                pid.I_RateComponent = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 1000.0f;
                pid.D_RateComponent = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 1000.0f;
                pid.Angle = (short)(buffer[byteIndex++] | buffer[byteIndex++] << 8) / 10.0f;

                pid.Target = (sbyte)buffer[byteIndex++];


                pid.Throttle = (double)buffer[byteIndex++];
                pid.Power1 = ((double)(buffer[byteIndex++]) / 255.0) * 100.0f;
                pid.Power2 = ((double)(buffer[byteIndex++]) / 255.0) * 100.0f;
                pid.Axis = buffer[byteIndex++] == 0x00 ? AxisTypes.Pitch : AxisTypes.Roll;

                return pid;
            }
            catch (Exception)
            {
                return null;
            }
        }

        public double TargetRate { get; set; }
        public double Rate { get; set;}

        public double ErrorSigma {get; set;}
        public double Derivative {get; set;}

        public double P_SteadyComponent { get; set; }
        public double I_SteadyComponent { get; set; }

        public double P_RateComponent {get; set;}
        public double I_RateComponent {get; set;}
        public double D_RateComponent {get; set;}
        public double Angle {get; set;}

        public double Target {get; set;}
        public double Throttle {get; set;}

        public double Power1 {get; set;}
        public double Power2 {get; set;}

        public AxisTypes Axis { get; set; }

        /* Used to charting */
        public double X { get; set; }
    }
}
