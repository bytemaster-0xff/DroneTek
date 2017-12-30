use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\obiscad\attach.scad>;
use <K:\Electronics\OpenSCAD\openscad-2013.06\libraries\bevel-library-openscad\obiscad\bevel.scad>;

screw_radius = 2.8 / 2;

panel_width = 123.7;
panel_thickness = 3;

//mount_offset = 32;


outside_cylinder_cutout_offset = 100;
outside_cylinder_cutout_radius = 95;

outside_chop_offset = 55;

inner_mount_hole_radius = 36.1;

mount_hole_y_offset = 9.8;
outside_mount_hole_offset = 10.15;
inside_mount_hole_offset = 8;


mount_offset = 55.5;

track_height = 6;

batt_width = 35;
batt_height = 23;
batt_len = 107;

batt_support_height = 10;

ebc_l = 50;
ebc_w = 22;
ebc_h = 16;

module battery() {
	translate([0,0,track_height+panel_thickness + (batt_height / 2)]) rotate([0,0,-45]) color("blue") cube([batt_len,batt_width,batt_height],center=true);
}

module ebc() {
	rotate([0,0,-45]) translate([0,45,10]) color("red") cube([ebc_l,ebc_w,ebc_h],center=true);
}

module track() {
		union(){
			difference() {
				union(){
					translate([0,0,panel_thickness + track_height / 2] ) color("black") cube([12,90, track_height], center=true);
					translate([0,0,panel_thickness + track_height / 2] ) color("black") cube([90,12, track_height], center=true);
					rotate([0,0,-45]) translate([-25,0,panel_thickness + track_height / 2] ) color("black")  cube([50,14, track_height], center=true);
				}

				translate([0,0,panel_thickness + track_height / 2] ) cube([8,90,track_height], center=true);
				translate([0,0,panel_thickness + track_height / 2] ) cube([90,8,track_height], center=true);
				rotate([0,0,-45]) translate([-25,0,panel_thickness + track_height / 2] ) cube([50,10, track_height], center=true);
			}

			difference() {
				union() {
					translate([0,-15,panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
					translate([0,-35,panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
				}

				translate([0,-15,panel_thickness + track_height - 0.5] ) color("red") cube([4,10, 1], center=true);
				translate([0,-35,panel_thickness + track_height - 0.5] ) color("red") cube([4,10, 1], center=true);
			}

			difference() {
				union() {
					translate([0,15,panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
					translate([0,35,panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
				}

				translate([0,15,panel_thickness + track_height - 0.5] ) color("red") cube([2,10, 1], center=true);
				translate([0,35,panel_thickness + track_height - 0.5] ) color("red") cube([4,10, 1], center=true);
			}

			difference() {
				union() {
					translate([-15, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
					translate([-35, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
				}

				translate([-15,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
				translate([-35,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
			}

			difference() {
				union() {
					translate([15, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
					translate([35, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
				}

				translate([15,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
				translate([35,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
			}

			rotate([0,0,135]) difference() {
				union() {
					translate([15, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
					translate([35, 0, panel_thickness + track_height - 0.5] ) color("red") cube([10,10, 1], center=true);
				}

				translate([15,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
				translate([35,0,panel_thickness + track_height - 0.5] ) color("red") cube([10,4, 1], center=true);
			}

		}
}

module top_plate() {
	union() {
	difference(){
		union(){
			difference() {
				translate([0,0,panel_thickness / 2]) cube([panel_width,panel_width,panel_thickness],center=true);
				translate([outside_cylinder_cutout_offset,-outside_cylinder_cutout_offset,panel_thickness / 2]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);
			}

			rotate([0,0,-45]) translate([0,0,panel_thickness/2]) cylinder(r=65, panel_thickness, $fn=50, center=true);

			track();
	
			

//			battery();

			rotate([0,0,-45]) translate([(batt_len / 2 + 3),0, 1.5 + panel_thickness]) color("red") rotate([90,0,0])	bconcave_corner(cr=4,th=2,cres=10,l=30);
			rotate([0,0,-45]) translate([(batt_len / 2 + 1.5),0,(batt_support_height + track_height ) /  2 + panel_thickness])  color("green") cube([3,30, batt_support_height + track_height], center=true);

			difference() {
				union() {
					rotate([0,0,135]) translate([(batt_len / 2 + 3),0, 1.5 + panel_thickness]) color("red") rotate([90,0,0])	bconcave_corner(cr=4,th=2,cres=10,l=30);
					rotate([0,0,135]) translate([(batt_len / 2 + 1.5),0,(batt_support_height + track_height ) /  2 + panel_thickness])  color("green") cube([3,30, batt_support_height + track_height], center=true);
				}

				rotate([0,0,135]) translate([(batt_len / 2 + 1.5),0,(batt_support_height + track_height ) /  2 + panel_thickness])  color("blue") cube([10,6, batt_support_height + track_height], center=true);

			}


//			ebc();
		}

		for(idx=[0:3]){
			translate([
				(idx == 1) ? -mount_offset : (idx==3) ? mount_offset : 0, 
				(idx == 0) ? mount_offset : (idx==2) ? -mount_offset : 0, 
				0])
			 rotate([0,0,idx * 90 ]) 
			 union(){
				translate([-outside_mount_hole_offset,0,panel_thickness/2]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	
				translate([outside_mount_hole_offset,0,panel_thickness/2]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	
			}
		}

		rotate([0,0,45]) translate([(batt_width / 2) + 3, 0,panel_thickness/2]) cube([3,22,panel_thickness], center=true);
		rotate([0,0,45]) translate([-((batt_width / 2) + 3), 0,panel_thickness/2]) cube([3,22,panel_thickness], center=true);

//		cylinder(r=33, panel_thickness, center=true);
//		translate([outside_cylinder_cutout_offset,outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);

		translate([panel_width/2,panel_width/2,panel_thickness/2]) rotate([0,0,45]) cube([60,60,panel_thickness], center=true);
		translate([-panel_width/2,panel_width/2,panel_thickness/2]) rotate([0,0,45]) cube([60,53,panel_thickness], center=true);
		translate([-panel_width/2,-panel_width/2,panel_thickness/2]) rotate([0,0,45]) cube([60,60,panel_thickness], center=true);


	}

	}
}

top_plate();




