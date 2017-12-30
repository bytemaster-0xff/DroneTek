fullView=true;

include <FrontSides.scad>
include <Front.scad>
include <RoverTurretMountV1.scad>
include <BackRightSide.scad>
include <BackLeftSide.scad>

include <BackLCDMount.scad>

$fn=30;
BOX_WIDTH = 120;
BOX_HEIGHT = 180;

THICKNESS=1.6;
POST_HEIGHT = 5;

MOTOR_CTL_WIDTH = 88.8;
MOTOR_CTL_HEIGHT = 61.8;

GALILEO_WIDTH = 97;
GALILEO_HEIGHT = 64;

SIDE_HEIGHT = 7.5;

module post(x,y) {
	difference() {
		translate([x,y,POST_HEIGHT / 2.0] ) cylinder(r=3,POST_HEIGHT, center=true);
		translate([x,y,(POST_HEIGHT / 2.0)]) cylinder(r=1.66,POST_HEIGHT * 4, center=true);
	}
}

module postsForTopSides(x,y) {
	difference() {
		union(){
			if(y > 0)
				translate([x,y-3,(SIDE_HEIGHT + THICKNESS / 2) / 2.0]) cube([6,7,(SIDE_HEIGHT + THICKNESS / 2)],center=true);
			else
				translate([x,y+3,(SIDE_HEIGHT + THICKNESS  / 2) / 2.0]) cube([6,7,(SIDE_HEIGHT + THICKNESS / 2)],center=true);

			translate([x,y,(SIDE_HEIGHT + THICKNESS / 2) / 2.0 ]) cylinder(r=3, SIDE_HEIGHT + THICKNESS / 2, center=true);
		}

		translate([x,y,10]) cylinder(r=1.25,SIDE_HEIGHT + THICKNESS / 2, center=true);
	}
}

module postsForTopFrontBack(x,y) {
	difference() {
		union(){
			if(x > 0)
				translate([x-3,y,(SIDE_HEIGHT + THICKNESS  / 2) / 2.0]) cube([7,6,(SIDE_HEIGHT + THICKNESS / 2)],center=true);
			else
				translate([x+3,y,(SIDE_HEIGHT + THICKNESS  / 2) / 2.0]) cube([7,6,(SIDE_HEIGHT + THICKNESS / 2)],center=true);


			translate([x,y,(SIDE_HEIGHT + THICKNESS / 2) / 2.0 ]) cylinder(r=3, SIDE_HEIGHT + THICKNESS / 2, center=true);
		}

		translate([x,y,10]) cylinder(r=1.25,SIDE_HEIGHT + THICKNESS / 2, center=true);
	}
}



module motorMounts(x)  {
	post(x + MOTOR_CTL_HEIGHT / 2,-MOTOR_CTL_WIDTH/2);		
	post(x + MOTOR_CTL_HEIGHT / 2,MOTOR_CTL_WIDTH/2);
	post(x - MOTOR_CTL_HEIGHT / 2,-MOTOR_CTL_WIDTH/2);		
	post(x - MOTOR_CTL_HEIGHT / 2,MOTOR_CTL_WIDTH/2);
}

module galileoMounts(x)  {
	post(x + GALILEO_HEIGHT / 2,-GALILEO_WIDTH/2);		
	post(x + GALILEO_HEIGHT / 2,GALILEO_WIDTH/2);
	post(x - GALILEO_HEIGHT / 2,-GALILEO_WIDTH/2);		
	post(x - GALILEO_HEIGHT / 2,GALILEO_WIDTH/2);
}


difference() {
	union() {

		difference() {
			union() {
			color("Yellow") translate([BOX_HEIGHT/2 + 10,0,SIDE_HEIGHT/2]) cube([20,16 + THICKNESS * 2,SIDE_HEIGHT],center=true);
			translate([0,0,SIDE_HEIGHT / 2 + THICKNESS / 2]) cube ([BOX_HEIGHT,BOX_WIDTH,SIDE_HEIGHT], center =true);
			}
			translate([BOX_HEIGHT/2 + 10 - THICKNESS,0,SIDE_HEIGHT/2 + THICKNESS]) cube([20,13.5,SIDE_HEIGHT],center=true);

			translate([0,0,SIDE_HEIGHT / 2 + THICKNESS / 2]) cube ([BOX_HEIGHT-THICKNESS * 2,BOX_WIDTH - THICKNESS * 2,SIDE_HEIGHT + 1], center =true);
			translate([-(BOX_HEIGHT/2)+52,-(BOX_WIDTH/2),8]) cube([10,4,10],center=true);
			translate([-(BOX_HEIGHT/2),+(BOX_WIDTH/2)-34,8]) cube([4,16,10],center=true);		
			translate([-(BOX_HEIGHT/2),-30,12]) cube([4,16,16],center=true);		
		}



		postsForTopSides(85.4,66);
		postsForTopSides(85.4,-66);
		postsForTopSides(49.4,66);
		postsForTopSides(49.4,-66);


		postsForTopSides(-86,66);
		postsForTopSides(-86,-66);
		postsForTopSides(-30,66);
		postsForTopSides(-30,-66);

	
		postsForTopFrontBack(BOX_HEIGHT / 2 + 6, 40);
		postsForTopFrontBack(BOX_HEIGHT / 2 + 6, -40);

//		postsForTopFrontBack(-(BOX_HEIGHT / 2 + 6), 40);
//		postsForTopFrontBack(-(BOX_HEIGHT / 2 + 6), -40);

	}
}




difference() {
	translate([0,0,THICKNESS / 2])cube ([180,120,THICKNESS], center =true);
	translate([0,0,THICKNESS / 2])cube ([140,75,THICKNESS * 2], center =true);
	translate([78.5,33,0]) cylinder(r=1.5,THICKNESS);
	translate([78.5,-33,0]) cylinder(r=1.5,THICKNESS);
	translate([-78.5,33,0]) cylinder(r=1.5,THICKNESS);
	translate([-78.5,-33,0]) cylinder(r=1.5,THICKNESS);
}
if(true)
{
//color("Blue") translate([60, BOX_WIDTH / 2 - (THICKNESS / 2),SIDE_HEIGHT + 25 ]) cube([60,THICKNESS,50], center=true);
//color("Blue") translate([60, -(BOX_WIDTH / 2 - (THICKNESS / 2)),SIDE_HEIGHT + 25 ]) cube([60,THICKNESS,50], center=true);

color("Brown") translate([100-THICKNESS ,-BOX_WIDTH / 2, SIDE_HEIGHT]) rotate([0,0,180]) drawLeftSide();
color("Brown") translate([100-THICKNESS ,BOX_WIDTH / 2, SIDE_HEIGHT]) drawRightSide();

color("Green") translate([BOX_HEIGHT / 2, 0,SIDE_HEIGHT])  drawFront();
color("Blue") translate([-44,-BOX_WIDTH / 2,SIDE_HEIGHT + THICKNESS / 2]) backRightSide();
color("Blue") translate([-44,BOX_WIDTH / 2,SIDE_HEIGHT + THICKNESS / 2]) backLeftSide();

color("Red") translate([60,0,SIDE_HEIGHT + SIDES_HEIGHT]) rotate([0,0,-90]) turretMount();
translate([-48,0,56.5 + SIDE_HEIGHT + THICKNESS / 2]) rotate([180,160,0]) lcdMount();
}


galileoMounts(-40);

motorMounts(37);

