screw_radius = 2.8 / 2;

panel_width = 100;
panel_thickness = 3;

//mount_offset = 32;


outside_cylinder_cutout_offset = 100;
outside_cylinder_cutout_radius = 95;

outside_chop_offset = 55;

inner_mount_hole_radius = 36.1;

mount_hole_y_offset = 9.8;
outside_mount_hole_offset = 11.7;
inside_mount_hole_offset = 8;

antenna_hole_diameter = 6.75;
antenna_hole_radius = antenna_hole_diameter / 2;

mount_offset = inner_mount_hole_radius + mount_hole_y_offset;

module mount(x, y) {
		translate([x, y, 7])   cylinder(r=4, 14, 5, $fn=20,center=true);
		translate([x, y, 16])   cylinder(r=2.5, 4, $fn=20,center=true);
		translate([x, y, 19])   cylinder(r=4, 4, $fn=20,center=true);
}

module top_plate() {
	union() {

		difference() {
			cube([panel_width,panel_width,panel_thickness],center=true);

			rotate([0,0,0]) cube([50,70,panel_thickness], center=true);
		}


//		cylinder(r=33, panel_thickness, center=true);
	//	translate([outside_cylinder_cutout_offset,-outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);
	//	translate([-outside_cylinder_cutout_offset,-outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);

	rotate([0,0,0]){
		mount(39,20);
		mount(-39,20);
		mount(39,-20);
		mount(-39,-20);
		}
	}
}

top_plate();




