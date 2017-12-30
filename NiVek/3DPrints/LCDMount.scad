thickness = 1.5;
POST_WIDTH_X = 42.5;
POST_OFFSET_Y = 24.5;

difference()
{
	translate([0,0, thickness/2]) cube([120,85,thickness],center=true);
	translate([0,0, thickness/2]) cube([70,50,thickness],center=true);
}