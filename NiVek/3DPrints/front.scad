use <Text/TextGenerator.scad>
$fn=30;

THICKNESS=1.6;
BOX_WIDTH = 120 - (THICKNESS * 2);
FRONT_POST_HEIGHT = 3;


module postsForTop(x,y,z,threads) {
	difference() {
		union(){
			translate([3,y,z]) cube([7,6,FRONT_POST_HEIGHT],center=true);
			translate([x,y,z]) cylinder(r=3, FRONT_POST_HEIGHT, center=true);
		}
		translate([x,y,z]) cylinder(r=threads ? 1.25 : 1.75,FRONT_POST_HEIGHT, center=true);
	}
}

module drawFront(){
	difference(){
		translate([-THICKNESS/2,0,25]) cube([THICKNESS,BOX_WIDTH,50],center=true);
		translate([-THICKNESS/2,0,2.5]) cube([THICKNESS,13.5,5],center=true);
		translate([-5,-40,30]) rotate([0,90,0]) rotate([0,0,90]) color("red") scale([1.5,1.5,10]) drawtext("ByteMaster");
		translate([-5,-45,15]) rotate([0,90,0]) rotate([0,0,90]) color("red") scale([1.3,1.3,10]) drawtext("NiVek Rover I");
	}

	postsForTop(6.0,-40,FRONT_POST_HEIGHT/2);
	postsForTop(6.0,40,FRONT_POST_HEIGHT/2);

	postsForTop(6.0,-40,50-FRONT_POST_HEIGHT/2,true);
	postsForTop(6.0,40,50-FRONT_POST_HEIGHT/2,true);



}

if(fullView != true)
	drawFront();