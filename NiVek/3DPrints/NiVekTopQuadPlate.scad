screw_radius = 2.8 / 2;

panel_width = 123.7;
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
	difference() {
		union() {
			translate([x, y, 4 + panel_thickness/2])   cylinder(r=4, 8, 6, $fn=20,center=true);

			translate([x, y, 9 + panel_thickness / 2])   cylinder(r=2.5, 3, $fn=20,center=true);
			translate([x, y, 12.5 + panel_thickness / 2])   cylinder(r=4, 4, $fn=20,center=true);
			translate([x, y, 15.5 + panel_thickness / 2])   cylinder(r=2.5, 3, $fn=20,center=true);
			translate([x, y, 19])   cylinder(r=4, 2, $fn=20,center=true);
		}
		translate([x, y, 15])   cylinder(r=1.55, 30, $fn=20,center=true);
		translate([x, y, 21])   cylinder(r=2.8, 4.5, $fn=20,center=true);
	}
}

module top_plate() {
	union() {
	difference(){
		union(){
			difference() {
				cube([panel_width,panel_width,panel_thickness],center=true);
				translate([outside_cylinder_cutout_offset,-outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);
			}
			rotate([0,0,-45]) translate([0,0,0]) cylinder(r=60, panel_thickness, $fn=50, center=true);
			difference() {
				rotate([0,0,-45]) translate([53,10,panel_thickness / 2 + 5]) cube([12,4,10], center=true);
				rotate([0,0,-45]) translate([53,10,7]) rotate([90,0,0]) cylinder(r=antenna_hole_radius, 100, $fn=20, center=true);
			}
		}

		for(idx=[0:3]){


			translate([
				(idx == 1) ? -mount_offset : (idx==3) ? mount_offset : 0, 
				(idx == 0) ? mount_offset : (idx==2) ? -mount_offset : 0, 
				0])
			 rotate([0,0,idx * 90 ]) 
			 union(){
				translate([-outside_mount_hole_offset,mount_hole_y_offset,0]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	
				translate([outside_mount_hole_offset,mount_hole_y_offset,0]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	

				translate([-inside_mount_hole_offset,-mount_hole_y_offset,0]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	
				translate([inside_mount_hole_offset,-mount_hole_y_offset,0]) cylinder(r=screw_radius,panel_thickness,$fn=50, center=true);	

//				cube([10,12,panel_thickness], center=true);
			}
		}


		rotate([0,0,45]) cube([30,50,panel_thickness], center=true);
		rotate([0,0,45]) cube([50,30,panel_thickness], center=true);
//		rotate([0,0,-45]) translate([0,33,0]) cube([20,5,panel_thickness], center=true);
//		rotate([0,0,135]) translate([-8,33,0]) cube([8,5,panel_thickness], center=true);






//		cylinder(r=33, panel_thickness, center=true);
		translate([outside_cylinder_cutout_offset,outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);

		translate([-outside_cylinder_cutout_offset,outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);
		translate([-outside_cylinder_cutout_offset,-outside_cylinder_cutout_offset,0]) cylinder(r=outside_cylinder_cutout_radius, panel_thickness, $fn=50, center=true);

		rotate([0,0,45]){
			translate([39,20,0]) cylinder(r=3.5,24, $fn=6, center=true);
			translate([-39,20,0]) cylinder(r=3.5,24, $fn=6, center=true);
			translate([39,-20,0]) cylinder(r=3.5,24, $fn=6, center=true);			
			translate([-39,-20,0]) cylinder(r=3.5,24, $fn=6, center=true);
		}

	}

	rotate([0,0,45]){
		mount(39,20);
		mount(-39,20);
		mount(39,-20);
		mount(-39,-20);
		}
	}
}

top_plate();




