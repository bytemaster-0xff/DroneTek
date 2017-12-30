wallthickness = 1.5;
bottom_height = 5;
board_width = 38.3;
board_length = 38.3;
mounthole_radius = 0.6;
mount_hole_offset = 23;
difference(){
	union(){
		translate([0,0,wallthickness / 2 + bottom_height / 2]) cube([board_width + wallthickness * 2,board_length + wallthickness * 2,bottom_height + wallthickness],center=true);
	minkowski() {
		translate([0,0,wallthickness / 4]) cube([board_width - 17,board_length - 17, wallthickness/2],center=true);
		cylinder(h=.25,r=15);
	}
	}
	translate([0,0,wallthickness + bottom_height / 2]) cube([board_width,board_length,bottom_height],center=true);
	translate([0,0,wallthickness / 2]) cube([24,24,wallthickness],center=true);

	translate([20,20,wallthickness + bottom_height / 2]) cube([5,5,bottom_height], center=true);
	translate([-20,-20,wallthickness + bottom_height / 2]) cube([5,5,bottom_height], center=true);
	translate([20,-20,wallthickness + bottom_height / 2]) cube([5,5,bottom_height], center=true);
	translate([-20,20,wallthickness + bottom_height / 2]) cube([5,5,bottom_height], center=true);

	translate([board_width / 2,0 ,wallthickness + bottom_height / 2]) cube([5,14,bottom_height], center=true);
	translate([(board_width / 2 - 17.0) + (6.3/2),-board_width / 2,wallthickness + bottom_height / 2]) cube([6.3,5,bottom_height], center=true);

	translate([mount_hole_offset,0,wallthickness/2]) cylinder(h=wallthickness, r=mounthole_radius,$fn=20,center=true);
	translate([0,mount_hole_offset,wallthickness/2]) cylinder(h=wallthickness, r=mounthole_radius,$fn=20,center=true);
	translate([-mount_hole_offset,0,wallthickness/2]) cylinder(h=wallthickness, r=mounthole_radius,$fn=20,center=true);
	translate([0,-mount_hole_offset,wallthickness/2]) cylinder(h=wallthickness, r=mounthole_radius,$fn=20,center=true);

}