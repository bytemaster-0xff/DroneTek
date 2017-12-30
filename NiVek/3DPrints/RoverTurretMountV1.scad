$fn=20;

THICKNESS = 3;
TOP_WIDTH=120;
TOP_HEIGHT=60;

module topPostsForSides(x,y) {
	difference() {
		union(){
			if(y > 0)
				translate([x,y-3,(THICKNESS / 2)]) cube([6,7,(THICKNESS)],center=true);
			else
				translate([x,y+3,(THICKNESS  / 2)]) cube([6,7,(THICKNESS)],center=true);

			translate([x,y,(THICKNESS / 2)  ]) cylinder(r=3, THICKNESS, center=true);
		}

		translate([x,y,(THICKNESS/2)]) cylinder(r=1.75,THICKNESS * 2, center=true);
	}
}

module topPostsForFront(x,y) {
	difference() {
		union(){
			if(x > 0)
				translate([x-3,y,(THICKNESS  / 2)]) cube([7,6,THICKNESS],center=true);
			else
				translate([x+3,y,(THICKNESS  / 2)]) cube([7,6,THICKNESS],center=true);

			translate([x,y,(THICKNESS / 2) ]) cylinder(r=3, THICKNESS , center=true);
		}

		translate([x,y,THICKNESS/2]) cylinder(r=1.75,THICKNESS, center=true);
	}
}

module turretMount(){
difference() {	
	union(){
		translate([0,-.8,THICKNESS/2]) cube([TOP_WIDTH,TOP_HEIGHT + 1.6,THICKNESS],center=true);
		topPostsForSides(40,6 + TOP_HEIGHT / 2);
		topPostsForSides(-40,6 + TOP_HEIGHT / 2);

		topPostsForFront(TOP_WIDTH/2+6,25.4);
		topPostsForFront(TOP_WIDTH/2+6,-10.6);

		topPostsForFront(-TOP_WIDTH/2-6,25.4);
		topPostsForFront(-TOP_WIDTH/2-6,-10.6);

		difference() {
			translate([40,0,5]) cube([22,TOP_HEIGHT,5],center=true);
			translate([40,-4,5]) cube([17,TOP_HEIGHT,5],center=true);
		}

	}

	rotate(90) {
	translate([0,0,THICKNESS]) cube([42,20.5,15],center=true);


	translate([47/2,5,THICKNESS]) cylinder(r=2.1,3,center=true);
	translate([-47/2,5,THICKNESS]) cylinder(r=2.1,3,center=true);

	translate([47/2,-5,THICKNESS]) cylinder(r=2.1,3,center=true);
	translate([-47/2,-5,THICKNESS]) cylinder(r=2.1,3,center=true);
	}

	
}
}

if(fullView != true)
	turretMount();