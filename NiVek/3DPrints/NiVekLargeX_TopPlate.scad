screw_radius = 2.8 / 2;

panel_width = 123.7;
panel_thickness = 1;

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
	difference() {
		union() {
			translate([x, y, 4 + panel_thickness/2])   cylinder(r=4, 8, 6, $fn=20,center=true);

			translate([x, y, 9 + panel_thickness / 2])   cylinder(r=2.5, 3, $fn=20,center=true);
			translate([x, y, 12.5 + panel_thickness / 2])   cylinder(r=4, 4, $fn=20,center=true);
			translate([x, y, 15.5 + panel_thickness / 2])   cylinder(r=2.5, 5, $fn=20,center=true);
			translate([x, y, 20])   cylinder(r=4, 4, $fn=20,center=true);
		}
		translate([x, y, 15])   cylinder(r=1.55, 30, $fn=20,center=true);
		translate([x, y, 22])   cylinder(r=2.8, 4.5, $fn=20,center=true);
	}
}

$fn=50;

module top_plate() {
	difference() {
		union() {


			translate([-47,-42,-panel_thickness / 2]) 	minkowski(			) {
				translate([0,0,0]) cube([75,55,panel_thickness], center=true);	
				translate([47,42,0]) cylinder(r=12, panel_thickness, center=true);
			}

			mount(39,20);
			mount(-39,20);
			mount(39,-20);
			mount(-39,-20);
		}

		translate([39,20,-panel_thickness /2]) cylinder(r=3.5,panel_thickness * 2, $fn=6, center=true);
		translate([-39,20,-panel_thickness /2]) cylinder(r=3.5,panel_thickness * 2, $fn=6, center=true);
		translate([39,-20,-panel_thickness /2]) cylinder(r=3.5,panel_thickness * 2, $fn=6, center=true);			
		translate([-39,-20,-panel_thickness /2]) cylinder(r=3.5,panel_thickness * 2, $fn=6, center=true);
	
		translate([32.5,32.5, -panel_thickness/2]) cylinder(r=2,3, $fn=20, center=true);
		translate([32.5,-32.5, -panel_thickness/2]) cylinder(r=2,3, $fn=20, center=true);
		translate([-32.5,32.5, -panel_thickness/2]) cylinder(r=2,3, $fn=20, center=true);
		translate([-32.5,-32.5, -panel_thickness/2]) cylinder(r=2,3, $fn=20, center=true);

		translate([0,0,0]) cube([40,60,3], center=true);	
		translate([0,0,0]) cube([60,20,3], center=true);	
	}
}

top_plate();


			



