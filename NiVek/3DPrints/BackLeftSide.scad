$fn=30;


BACK_SIDE_POST_HEIGHT = 3;
THICKNESS = 1.6;

BACK_POST_DELTA = 56;

module backLeftSide(){

	module postsForBackRight(x,y,threads) {
		difference() {
			union(){
				translate([x,3,BACK_SIDE_POST_HEIGHT/2]) cube([6,7,BACK_SIDE_POST_HEIGHT],center=true);
				translate([x,y,BACK_SIDE_POST_HEIGHT / 2.0 ]) cylinder(r=3, BACK_SIDE_POST_HEIGHT, center=true);
			}
			translate([x,y,(BACK_SIDE_POST_HEIGHT / 2.0) ]) cylinder(r=threads ? 1.25 : 1.75,BACK_SIDE_POST_HEIGHT, center=true);
		}
	}

	difference() {
		translate([0,-THICKNESS / 2, 40]) cube([90,THICKNESS,80],center=true);
		translate([0,-THICKNESS / 2, 90]) rotate([0,70,0]) cube([60,THICKNESS * 2,150],center=true);
	}


	postsForBackRight(-42,6,false);
	postsForBackRight(-42 + BACK_POST_DELTA,6,false);

	rotate([0,-20,0]) translate([100,0,51.5]) postsForBackRight(-42,6, true);
	rotate([0,-20,0]) translate([20,0,51.5]) postsForBackRight(-42,6, true);

	postsForBackRight(-42 + BACK_POST_DELTA,6);
}

if(fullView != true)
	backLeftSide();





