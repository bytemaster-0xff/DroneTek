use </obiscad/bevel.scad>
use <obiscad/attach.scad>

$fn = 30;
SIDE_POST_HEIGHT = 3;
THICKNESS=1.6;

SIDES_HEIGHT = 50;

module frontSide(leftSide){
module postsForTop(x,y,z, threads) {
	difference() {
		union(){
			translate([x,3,z]) cube([6,7,SIDE_POST_HEIGHT],center=true);
			translate([x,y,z]) cylinder(r=3, SIDE_POST_HEIGHT, center=true);
		}
		color("red") translate([x,y,z ]) cylinder(r=(threads ? 1.25 : 1.75),SIDE_POST_HEIGHT * 2, center=true);
	}

}


		WIDTH = 60;

		translate([WIDTH / 2,-0.75,25]) cube([WIDTH,THICKNESS,SIDES_HEIGHT],center=true);


		size = [WIDTH,2,4];
		ec1 = [ [0, 3, -3], [1,0,0], 0];
		en1 = [ ec1[0], [0,1,-1], 0];		

		difference() {
			union() {
				translate([WIDTH / 2,5,45]) cube(size,center=true);
				translate([WIDTH / 2,2,45.5]) cube([WIDTH,5,3],center=true);
			}
			translate([WIDTH / 2,5,45]) bevel(ec1, en1, cr = 8, cres=10, l=size[0]+2);
		}

		rotate([0,180,0]) difference() {
			union() {
				translate([-WIDTH / 2,5,-24]) cube(size,center=true);
				translate([-WIDTH / 2,2,-23.5]) cube([WIDTH,5,3],center=true);
			}
			translate([-WIDTH / 2,5,-24]) bevel(ec1, en1, cr = 8, cres=10, l=size[0]+2);
		}
		
		postsForTop(leftSide ? 3 : 57,6.0,SIDE_POST_HEIGHT / 2.0);
		postsForTop(leftSide ? 39 : 21 ,6.0,SIDE_POST_HEIGHT / 2.0);

		postsForTop(leftSide ? 3 : 57,6.0,SIDES_HEIGHT - (SIDE_POST_HEIGHT / 2.0), true);
		postsForTop(leftSide ? 39 : 21 ,6.0,SIDES_HEIGHT - (SIDE_POST_HEIGHT / 2.0), true);
}

module drawLeftSide(){
	translate([10,0,0]) frontSide(true);
}

module drawRightSide(){
	translate([-70,0,0]) frontSide(false);
}

if(fullView != true){
//	drawLeftSide();
	drawRightSide();
}
/*		postsForTop(50,-61.8);
		postsForTop(86,-61.8);*/