use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\obiscad\attach.scad>;
use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\bevel-library-openscad\obiscad\bevel.scad>;


board_width = 50.8;
padding = 0.4;
half_board_width = board_width / 2;
mount_hole_center_offset = 3.81;
mount_hole_from_center = half_board_width - mount_hole_center_offset;
mount_hole_diameter = 3.1;
mount_hole_radius = mount_hole_diameter / 2;

tab_width = 7;
tab_height = 8;
tab_length = 13;
thickness = 2;

base_height = 5;
base_width = 48;
base_length = 92.5;


support_height=3;
support_diameter = 8;


platform_base = board_width + padding + thickness * 2;

ridge_height = 2;

corner_bevel =5;
inside_corner_bevel = 3;


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

module mounting_holes(){
	translate([mount_hole_from_center,mount_hole_from_center,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([-mount_hole_from_center,mount_hole_from_center,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([mount_hole_from_center,-mount_hole_from_center,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		
	translate([-mount_hole_from_center,-mount_hole_from_center,thickness/2]) cylinder(r=3.20,thickness,$fn=6, center=true);		

	translate([mount_hole_from_center,mount_hole_from_center,(thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height,$fn=20, center=true);		
	translate([-mount_hole_from_center,mount_hole_from_center,(thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height,$fn=20, center=true);		
	translate([mount_hole_from_center,-mount_hole_from_center, (thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height, $fn=20, center=true);		
	translate([-mount_hole_from_center,-mount_hole_from_center, (thickness + support_height) / 2]) cylinder(r=1.505, thickness + support_height, $fn=20, center=true);		
}

module mounting_supports(){
	translate([mount_hole_from_center,mount_hole_from_center,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([-mount_hole_from_center,mount_hole_from_center,support_height / 2 + thickness ]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([mount_hole_from_center,-mount_hole_from_center,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
	translate([-mount_hole_from_center,-mount_hole_from_center,support_height / 2 + thickness]) cylinder(r=support_diameter / 2, support_height, $fn=20, center=true);		
}

module tabs() {
	tab(base_width / 2 + tab_width / 2 + thickness, 17);
	tab(base_width / 2 + tab_width / 2 + thickness, -17);
	tab(-base_width / 2 - tab_width / 2  - thickness, 17);
	tab(-base_width / 2 -tab_width / 2 - thickness, -17);
}
module topMountOffset() {
	difference(){		
		minkowski(){
			translate([0,0,thickness + base_height + (ridge_height / 2)]) cube([platform_base - thickness - (4 * 2),platform_base - thickness - (4* 2),ridge_height],center=true);
			cylinder(r=4, h=1,$fn=50);
		}

		minkowski() {
					translate([0,0,thickness + base_height + (ridge_height / 2)]) cube([board_width + padding - (inside_corner_bevel * 2),board_width + padding - (inside_corner_bevel * 2),ridge_height],center=true);
					cylinder(r=inside_corner_bevel,h=1,$fn=50);
		}
	}
}

module base() {
	difference() {
		union() {
			minkowski() {
				translate([0,0,thickness / 2 ]) cube([platform_base - (corner_bevel * 2),platform_base - (corner_bevel * 2),thickness],center=true);
				cylinder(r=corner_bevel,h=1,$fn=50);
			}

			mounting_supports();
		}

		mounting_holes();
	}
}

module upper() {
	difference() {
		union(){
			difference(){
				minkowski() {
					translate([0,0,base_height / 2 + thickness]) cube([platform_base - (corner_bevel * 2),platform_base - (corner_bevel * 2),base_height],center=true);
					cylinder(r=corner_bevel,h=1,$fn=50);
				}

				minkowski() {
					translate([0,0,base_height / 2 + thickness]) cube([board_width + padding - (inside_corner_bevel * 2),board_width + padding - (inside_corner_bevel * 2),base_height],center=true);
					cylinder(r=inside_corner_bevel,h=1, $fn=50);
				}
			}

			translate([board_width /2 + padding, board_width / 2 + padding, base_height / 2 + thickness]) rotate([0,0,180]) bconcave_corner(cr=3,th=0.1,cres=10,l=base_height);

			tabs();

			topMountOffset();
		}
	}
}

module nivekMiniBoard() {
	union() {
		base();
		upper();
	}
}

nivekMiniBoard();



