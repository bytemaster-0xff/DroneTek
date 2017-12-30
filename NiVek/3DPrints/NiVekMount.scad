use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\obiscad\attach.scad>;
use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\bevel-library-openscad\obiscad\bevel.scad>;

post_offset_width = 80.0;
post_offset_height = 40.0;

tab_width = 7;
tab_height = 8;
tab_length = 13;
thickness = 2;

board_width = 48;
board_length = 92.5;

padding = 0.4;

corner_bevel =5;
inside_corner_bevel = 3;

support_height=3;
support_diameter = 8;

half_board_width = board_width / 2;
half_board_length = board_length / 2;

mount_hole_center_offset = 3.81;
mount_hole_x = half_board_width - mount_hole_center_offset;
mount_hole_y = half_board_length - mount_hole_center_offset;
mount_hole_diameter = 3.1;
mount_hole_radius = mount_hole_diameter / 2;

base_height=6;

platform_base_width = board_width + padding + thickness * 2;
platform_base_length = board_length + padding + thickness * 2;

module tab(x, y) {
	difference(){		
		if(x > 0)
			translate([x, y, tab_height / 2 - thickness]) rotate([90,0,0]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);
		else
			translate([x, y, tab_height / 2 - thickness]) rotate([90,0,180]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);

		hull(){
			if(x > 0){
				translate([x-1, y-0.5, tab_height / 2]) cylinder(r=3, tab_height, center=true, $fn=20);
				translate([x-1, y+0.5, tab_height / 2]) cylinder(r=3, tab_height, center=true, $fn=20);

			}
			else{
				translate([x+1, y-0.5, tab_height / 2]) cylinder(r=3, tab_height, center=true, $fn=20);
				translate([x+1, y+0.5, tab_height / 2]) cylinder(r=3, tab_height, center=true, $fn=20);
			}
		}

		if(x > 0){
			translate([x + tab_width / 2 + 0.5, y-tab_length / 2, tab_height / 2]) rotate([0,0,90]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);	
			translate([x + tab_width / 2 + 0.5, y+tab_length / 2, tab_height / 2]) rotate([0,0,180]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);
		}
		else{
			translate([x - tab_width / 2 - 0.5, y-tab_length / 2, tab_height / 2]) rotate([0,0,0]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);	
			translate([x - tab_width / 2 - 0.5, y+tab_length / 2, tab_height / 2]) rotate([0,0,270]) bconcave_corner(cr=4,th=thickness,cres=10,l=tab_length);
		}		
	}
}

module mounting_post(x, y){
	difference() {
		translate([x, y, 1 ]) cylinder(r=3.25,2,$fn=20,center=true);
		translate([x, y, 0 ]) cylinder(r=1.6,4,$fn=20,center=true);
	}
}

module apm_mount(){
	difference() {
		union(){



			difference() {
				translate([0,0,0])cube([53,93,6],center=true);

			}

			difference() {
				translate([0,0,-3])cube([66,90.5,2],center=true);
				translate([28,20,-3]) cylinder(r=3.5,2,$fn=20,center=true);
				translate([-28,20,-3]) cylinder(r=3.5,2,$fn=20,center=true);
				translate([28,-20,-3]) cylinder(r=3.5,2,$fn=20,center=true);
				translate([-28,-20,-3]) cylinder(r=3.5,2,$fn=20,center=true);
			}
		}

		translate([0,0,-2])	     cube([25,64.5,10],center=true);

		translate([-30,0,-3])	cube([10,26,2],center=true);
		translate([30,0,-3])	cube([10,26,2],center=true);

/*		translate([-30,-40,-3])	cube([10,26,2],center=true);
		translate([30,-40,-3])	cube([10,26,2],center=true);
		translate([-30,40,-3])	cube([10,26,2],center=true);
		translate([30,40,-3])	cube([10,26,2],center=true);*/

	}
}


module upper() {
	difference() {
		union(){
			difference(){
				minkowski() {
					translate([0,0,base_height / 2 + thickness]) cube([platform_base_width - (corner_bevel * 2),platform_base_length - (corner_bevel * 2),base_height],center=true);
					cylinder(r=corner_bevel,h=1,$fn=50);
				}

				minkowski() {
					translate([0,0,base_height / 2 + thickness]) cube([board_width + padding - (inside_corner_bevel * 2),board_length + padding - (inside_corner_bevel * 2),base_height],center=true);
					cylinder(r=inside_corner_bevel,h=1, $fn=50);
				}
			}

			translate([board_width /2 + padding, board_width / 2 + padding, base_height / 2 + thickness]) rotate([0,0,180]) bconcave_corner(cr=3,th=0.1,cres=10,l=base_height);

			tabs();

			topMountOffset();
		}
	}
}

module mounting_holes(){
	translate([mount_hole_x,mount_hole_y,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([-mount_hole_x,mount_hole_y,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([mount_hole_x,-mount_hole_y,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([-mount_hole_x,-mount_hole_y,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		

	translate([mount_hole_x,mount_hole_y,(thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height,$fn=20, center=true);		
	translate([-mount_hole_x,mount_hole_y,(thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height,$fn=20, center=true);		
	translate([mount_hole_x,-mount_hole_y, (thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height, $fn=20, center=true);		
	translate([-mount_hole_x,-mount_hole_y, (thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height, $fn=20, center=true);		
}

module mounting_supports(){
	translate([mount_hole_x,mount_hole_y,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([-mount_hole_x,mount_hole_y,support_height / 2 + thickness ]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([mount_hole_x,-mount_hole_y,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([-mount_hole_x,-mount_hole_y,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
}

module tabs() {
	tab(platform_base_width / 2 + tab_width / 2, 20);
	tab(platform_base_width / 2 + tab_width / 2, -20);
	tab(-platform_base_width / 2 - tab_width / 2, 20);
	tab(-platform_base_width / 2 -tab_width / 2, -20);
}

module base() {
	difference() {
		union() {
			minkowski() {
				translate([0,0,thickness / 2 ]) cube([platform_base_width - (corner_bevel * 2),platform_base_length - (corner_bevel * 2),thickness],center=true);
				cylinder(r=corner_bevel,h=1,$fn=50);
			}

			mounting_supports();
		}

		mounting_holes();
	}
}

base();
upper();
tabs();


//apm_mount();
