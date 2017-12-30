$fn=50;
difference() {
	cube([116,34,38],center=true);
	translate([0,0,3]) cube([110,28,35],center=true);
	translate([0,16,15]) cube([110,6,20],center=true);
	translate([58,02,20]) cube([110,32,20],center=true);
	rotate([90,0,0]) translate([-40,14,0]) cylinder (r=2,20);
	rotate([90,0,0]) translate([40,14,0]) cylinder (r=2,20);
}

difference() {
	translate([0,26,-7]) cube([78,21,24],center=true);
	translate([0,26,-4]) cube([75,18,24],center=true);	
}

//translate([0,26,3]) color("red") cube([74,16.5,38], center=true);