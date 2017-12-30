use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\obiscad\attach.scad>;
use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\bevel-library-openscad\obiscad\bevel.scad>;


post_offset_width = 80.0;
post_offset_height = 40.0;

GPS_CUTOUT_WIDTH = 19;
GPS_CUTOUT_HEIGHT = 19;

tab_width = 7;
tab_length = 16;
thickness = 2;

base_width = 48;
base_length = 92.5;

box_height = 13;
cutout_height = 12;

ridge_height = 2;

board_width = 50.8;
padding = 0.4;
half_board_width = board_width / 2;
mount_hole_center_offset = 3.81;
mount_hole_from_center = half_board_width - mount_hole_center_offset;
mount_hole_diameter = 3.1;
mount_hole_radius = mount_hole_diameter / 2;

base_height = 5;
base_width = 48;
base_length = 92.5;


support_height=3;
support_diameter = 8;


platform_base = board_width + padding + thickness * 2;

ridge_height = 2;

corner_bevel =5;
inside_corner_bevel = 3;

top_height = 4;


module topContainer() {
	difference(){		
		minkowski() {
			translate([0,0,(top_height + thickness) / 2 + thickness ]) cube([platform_base - (corner_bevel * 2),platform_base - (corner_bevel * 2),top_height + thickness],center=true);
			cylinder(r=corner_bevel,h=1,$fn=50);
		}


		minkowski() {
			translate([0,0,top_height / 2 + thickness]) cube([board_width + padding - (inside_corner_bevel * 2),board_width + padding - (inside_corner_bevel * 2),top_height],center=true);
			cylinder(r=inside_corner_bevel,h=1, $fn=50);
		}	
	}
}

module topMountOffset() {
	difference(){		
		minkowski() {
			translate([0,0,thickness / 2 ]) cube([platform_base - (corner_bevel * 2),platform_base - (corner_bevel * 2),thickness],center=true);
			cylinder(r=corner_bevel,h=1,$fn=50);
		}

		minkowski(){
			translate([0,0,(ridge_height / 2)]) cube([platform_base - thickness - (4 * 2),platform_base - thickness - (4* 2),ridge_height],center=true);
			cylinder(r=4, h=1,$fn=50);
		}
	}
}

connector_cutout = 6;

difference() {
	union() {
		topMountOffset();
		topContainer();
	}
//	translate([mount_hole_from_center,mount_hole_from_center,(top_height + thickness) + thickness/2]) cylinder(r=mount_hole_radius,thickness * 2,$fn=20, center=true);
//	translate([-mount_hole_from_center,mount_hole_from_center,(top_height + thickness) + thickness/2]) cylinder(r=mount_hole_radius,thickness * 2,$fn=20, center=true);
//	translate([mount_hole_from_center,-mount_hole_from_center,(top_height + thickness) + thickness/2]) cylinder(r=mount_hole_radius,thickness * 2,$fn=20, center=true);
//	translate([-mount_hole_from_center,-mount_hole_from_center,(top_height + thickness) + thickness/2]) cylinder(r=mount_hole_radius,thickness * 2,$fn=20, center=true);

	//Port	
	translate([half_board_width,-3,cutout_height / 2]) cube([connector_cutout,34,cutout_height], center=true);

	//Starboard
	translate([-half_board_width,-(35.03 - half_board_width),cutout_height / 2]) cube([connector_cutout,9,cutout_height], center=true);

	//Rear
	translate([half_board_width - 18.5,half_board_width,cutout_height / 2]) cube([25,connector_cutout,cutout_height], center=true);

	//Front (antenna)
	translate([26.924 - half_board_width,-29,4.0]) rotate([90,0,0]) cylinder(r=1.5,6,$fn=20, center=true);
	translate([26.924 - half_board_width,-29,2]) cube([3,connector_cutout,4], center=true);

	//Open for Front Servo */
	translate([34.4204 - half_board_width,-half_board_width,cutout_height / 2]) cube([8,connector_cutout,cutout_height], center=true);

	/* Cut out for GPS */		
	translate([-(half_board_width-11.68),(half_board_width-17.5),top_height + thickness]) cube([GPS_CUTOUT_WIDTH,GPS_CUTOUT_HEIGHT,6],center=true);

	/* Cut out for LEDS */
	//GPS & Armed
//	translate([-(half_board_width-12),(half_board_width-2.34),top_height + thickness]) cube([8,4.5,6],center=true);

	//Commo/Status
//	translate([(half_board_width-14),-(half_board_width-6),top_height + thickness]) cube([10,4.5,6],center=true);

	//Power
//	translate([-(half_board_width-26),(half_board_width-10),top_height + thickness]) cube([3,3,6],center=true);
}