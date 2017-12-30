using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.FlightControls.NiVekMath
{
    public class StandardDeviation : INotifyPropertyChanged
    {
        List<double> _values;
        double _stdDeviation;
        double _mean;

        public StandardDeviation()
        {
            _values = new List<double>();
        }

        public void Add(double newValue)
        {
            _values.Add(newValue);
            if (_values.Count > 0)
            {
                _mean = _values.Average();
                double squaredDeltas = 0.0;
                foreach (var value in _values)
                {
                    var delta = System.Math.Abs(value - _mean);
                    squaredDeltas += delta * delta;
                }

                _stdDeviation = System.Math.Sqrt(squaredDeltas / (_values.Count - 1));
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("StandardDeviation"));
                    PropertyChanged(this, new PropertyChangedEventArgs("Mean"));
                }
            }
        }

        public void Reset()
        {
            Debug.WriteLine("Count before reset {0}", _values.Count);
            _values.Clear();
            Debug.WriteLine("Count After reset {0}", _values.Count);
        }

        public double Mean { get { return _mean; } }

        public double Deviation { get { return _stdDeviation; } }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
