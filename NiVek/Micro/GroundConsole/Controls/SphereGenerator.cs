using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Media3D;

namespace GroundConsole.Controls
{
    public class SphereGenerator
    {
        private int _slices = 64;
        private int _stacks = 32;
        private Point3D _center = new Point3D();
        private double _radius = 1;

        public int Slices
        {
            get { return _slices; }
            set { _slices = value; }
        }

        public int Stacks
        {
            get { return _stacks; }
            set { _stacks = value; }
        }

        public Point3D Center
        {
            get { return _center; }
            set { _center = value; }
        }

        public double Radius
        {
            get { return _radius; }
            set { _radius = value; }
        }

        public MeshGeometry3D Geometry
        {
            get
            {
                return CalculateMesh();
            }
        }


        private MeshGeometry3D CalculateMesh()
        {
            MeshGeometry3D mesh = new MeshGeometry3D();

            for (int stack = 0; stack <= Stacks; stack++)
            {
                double phi = Math.PI / 2 - stack * Math.PI / Stacks; // kut koji zamisljeni pravac povucen iz sredista koordinatnog sustava zatvara sa XZ ravninom. 
                double y = _radius * Math.Sin(phi); // Odredi poziciju Y koordinate. 
                double scale = -_radius * Math.Cos(phi);

                for (int slice = 0; slice <= Slices; slice++)
                {
                    double theta = slice * 2 * Math.PI / Slices; // Kada gledamo 2D koordinatni sustav osi X i Z... ovo je kut koji zatvara zamisljeni pravac povucen iz sredista koordinatnog sustava sa Z osi ( Z = Y ). 
                    double x = scale * Math.Sin(theta); // Odredi poziciju X koordinate. Uoči da je scale = -_radius * Math.Cos(phi)
                    double z = scale * Math.Cos(theta); // Odredi poziciju Z koordinate. Uoči da je scale = -_radius * Math.Cos(phi)

                    Vector3D normal = new Vector3D(x, y, z); // Normala je vektor koji je okomit na površinu. U ovom slučaju normala je vektor okomit na trokut plohu trokuta. 
                    mesh.Normals.Add(normal);
                    mesh.Positions.Add(normal + Center);     // Positions dobiva vrhove trokuta. 
                    mesh.TextureCoordinates.Add(new Point((double)slice / Slices, (double)stack / Stacks));
                    // TextureCoordinates kaže gdje će se neka točka iz 2D-a preslikati u 3D svijet. 
                }
            }

            for (int stack = 0; stack <= Stacks; stack++)
            {
                int top = (stack + 0) * (Slices + 1);
                int bot = (stack + 1) * (Slices + 1);

                for (int slice = 0; slice < Slices; slice++)
                {
                    if (stack != 0)
                    {
                        mesh.TriangleIndices.Add(top + slice);
                        mesh.TriangleIndices.Add(bot + slice);
                        mesh.TriangleIndices.Add(top + slice + 1);
                    }

                    if (stack != Stacks - 1)
                    {
                        mesh.TriangleIndices.Add(top + slice + 1);
                        mesh.TriangleIndices.Add(bot + slice);
                        mesh.TriangleIndices.Add(bot + slice + 1);
                    }
                }
            }

            return mesh;
        }
    }
}