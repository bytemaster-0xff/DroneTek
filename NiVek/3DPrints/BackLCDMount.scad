$fn=30;

THICKNESS = 2.0;
LCD_COVER_WIDTH = 120; 
LCD_COVER_HEIGHT = 97;

LCD_OPENING_WIDTH = 70;
LCD_OPENING_HEIGHT = 52;

	module postsForLCDMount(x,y) {
		difference() {
			union(){				
				translate([x,y + ((y > 0) ? -3 : 3),THICKNESS/2]) cube([6,7,THICKNESS],center=true);

				translate([x,y,THICKNESS / 2.0 ]) cylinder(r=3, THICKNESS, center=true);
			}
			translate([x,y,(THICKNESS / 2.0) ]) cylinder(r=1.75,THICKNESS, center=true);
		}
	}

module lcdModule() {
	color("Red") translate([0,-10,0]) translate([0,0,THICKNESS / 2]) cube([LCD_OPENING_HEIGHT -2, LCD_OPENING_WIDTH -2, THICKNESS*4], center=true);
	color("Red") translate([0,3,-17.5]) cube([65, 105, 35], center=true);
	color("Green") translate([-3,58,-27.5]) cube([45, 18, 12], center=true);
	color("Green") translate([0,-58,-7.5]) cube([30, 10, 3], center=true);
}

module lcdMountPost(x,y){
		difference() {
			translate([x,y,-2.5]) cylinder(r=2.25,5,center=true);
			translate([x,y,-2.5]) cylinder(r=1.25,5,center=true);
		}
}

module lcdMount() {
	difference() {
		color("silver") translate([-4,0,THICKNESS / 2]) cube([LCD_COVER_HEIGHT, LCD_COVER_WIDTH, THICKNESS], center=true);
		translate([0,-10,0]) translate([0,0,THICKNESS / 2]) cube([LCD_OPENING_HEIGHT, LCD_OPENING_WIDTH, THICKNESS], center=true);
	}

//	translate([0,(LCD_COVER_WIDTH/2) - (THICKNESS / 2),-1.5]) cube([LCD_COVER_HEIGHT, THICKNESS, 3], center=true);
//	translate([0,-((LCD_COVER_WIDTH/2) - (THICKNESS / 2)),-1.5]) cube([LCD_COVER_HEIGHT, THICKNESS, 3], center=true);

	lcdMountPost(30,35.5);
	lcdMountPost(30,-49.5);
	lcdMountPost(-30,35.5);
	lcdMountPost(-30,-49.5);

	postsForLCDMount(-42.4,66);
	postsForLCDMount(37.6,66);

	postsForLCDMount(-42.4,-66);
	postsForLCDMount(37.6,-66);

//	lcdModule();
}




if(fullView != true)
	lcdMount();