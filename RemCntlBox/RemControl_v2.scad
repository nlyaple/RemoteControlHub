//Procedural Project Box Screws Remix
//CudaTox, 2017
//Corrected issues when thickness is changed screw columns are not located correctly.
//Corrected issue whtn thickness is greater than 2 and lip_tol is used created offset in screw posts in lid.
//Added options for lid reinforcment and lid screw stud length
//Cleaned up code buy calculating global variables to use in place of formulas.
//Ron Shenk 2019
//Revision 1
//Added variables to allow generating each element seperate or together
// Nelson Yaple 2021
// Revision 2
// Created a purpose built box to house OLED display and BME sensor alog with ESP32 Relay board.
//  Added comments to parametrics and code

//include <OLED-Angled-Mount.scad>;

// true generates box false skips generating the box
generate_box = true;            
// true generates lid, false skips generating the lid
generate_lid = true;            
// true generates mounting tab to the lid, false skips generating the tabs
generate_mount = true;            
// True generate counter sunk lib screws, false gens rescessed holes
counter_sunk = true;
// True generate PC studs in box bottom
use_ic_studs = true;           
// True to create slotted side vents
vent_slots = true;           
// True to create OLED Window
oled_window = true;           
// Thermal sensor box (BMP or BME280)
temp_sensor = true;           

/* [Box Inside Dimensions] */
inside_width = 90;
inside_length = 100;
inside_height = 25;
//Wall thickness
thickness = 3;                   
//Fillet radius. Should not be larger than thickness
radius =3;              		
//Diameter of the screw you intend to use
screw_dia = 3;                  
screw_loose_dia = 3.5;
/*  [Lid Dimensions] */
//Extra lid thickness above thickness. 
extra_lid_thickness = 0;        
                                //You may want to tweak this to account for large chamfer radius.
//this tolerence helps the lid fit
lip_tol = .5;					
//Length of studs on lid
lid_stud_height = 4;			
//Lid reinforcement lip height
lid_lip_height = 2;    			
//Width of lid reinforcement lip
lid_lip_width =4;	

/* [PCB Dimensions] */
//PC Board stud height
ic_stud_height = 16;             
//Diameter of screw needed
ic_stud_id = 1;                 
//you can set this manually to a fixed value in place of the formula
ic_stud_od = ic_stud_id*4; //2.5;    
//decreases stud id when using threaded fastners so they have purchase
ic_stud_multiplier = .9;       
//PC Board stud starting X position
ic_stud_start_x = 8;           
//PC stud starting Y position
ic_stud_start_y = 15;            
//Width between centers of PC Board studs
ic_stud_width = 46;             
//Length between centers of PC Board studs
ic_stud_length = 66;            
/* [Mounting tab Dimensions] */
// Width of each mounting tab on Lid
baseOffset = 20;
// Hole size of mounting tab holes
mount_screw_dia = 6.5;

// Computation of variables used in box creation do not modify values below this line
od = screw_dia * 2.5;
outside_width = inside_width + (thickness * 2);
outside_length = inside_length + (thickness * 2);
outside_height = inside_height + (thickness * 2);
box_stud_height = inside_height - lid_stud_height;
column_offset = (od/2)+thickness;
column_radius = od/2;
lid_startpos = (outside_width*-1)-2;
lid_lengthoffset = inside_length - column_radius + thickness;
lid_widthoffset = inside_width - column_radius + thickness;
ic_stud_real_id = ic_stud_id * ic_stud_multiplier;

module box_ic_stud(height,id,od){
    difference(){
        cylinder(d=od, h=height, $fn=100);
        cylinder(d=id * ic_stud_multiplier, h=height, $fn=100);
    }
}

module box_screw(id, od, height){
    difference(){
        union(){
            cylinder(d=od, h=height, $fs=0.2);
            translate([-(od/2), -(od/2), 0])
                cube([(od/2),od,height], false);
            translate([-od/2, -(od/2), 0])
                cube([od,(od/2),height], false);
        }
        cylinder(d=id, h=height, $fn=6);
    }
}

module rounded_box(x,y,z,r)
{
    translate([r,r,r])
    minkowski(){
        cube([x-r*2,y-r*2,z-r*2]);
        sphere(r=r, $fs=0.1);
    }
}

module squared_off_box(x,y,z,r)
{
    translate([r,r,z])
    minkowski(){
        cube([x-r*2, y-r*2, r]);
        cylinder(d=radius, h=r, $fn=100);
    }
}


// <- Square hole ->  
// Sx=Square X position | Sy=Square Y position | Sl= Square Length | Sw=Square Width | Filet = Round corner
module SquareHole(Sx,Sy,Sl,Sw,Filet)
{
     minkowski()
    {
        translate([Sx+Filet/2, Sy+Filet/2,-1])
            cube([Sl-Filet, Sw-Filet, 10]);
            cylinder(d=Filet, h=thickness, $fn=100);
    }
}


// <- Circle hole -> 
// Cx=Hole X position | Cy=Hole Y position | Cdia= Hole dia | Cheight=Cyl height
module CylinderHole(Cx, Cy, Cdia)
{
    translate([Cx,Cy,-(thickness+1)])
        cylinder(d=Cdia, h=(thickness +1), $fn=50);
}

module main_box(){
    difference()
    {
        // Build the rough out box
        difference(){
            rounded_box(outside_width, outside_length, outside_height, radius);
            // Removes the rounded edge and make the lib mating surface
            translate([0,0,inside_height + thickness])
                cube([outside_width, outside_length, outside_height]);
        }
        // Hollow out the interior of the box
        translate([thickness, thickness, thickness])
            cube([inside_width, inside_length, inside_height + thickness]);
        if (oled_window)
        {
            // Hole for OLED display mount
            SquareHole(ic_stud_start_x+ 2, ic_stud_start_y+4, 29, 31, 1);
        }
        // Create rectangle hole for OLED screen
        SquareHole((inside_width / 2) - 1 , 35, 14, 25, 1);
        // Optional hole for external BME sensor cable
        //rotate([90,0,0])
        //color("Orange")        
        //CylinderHole(outside_width / 2, outside_height-5, 8);

        // Side Hole for USB power 
        translate([0, 3, 0])
        {
            rotate([90,0,0])
            color("Red")        
            SquareHole((inside_width / 2) - 2.5, (inside_height / 3) - 2, 12, 7, 0);
        }

        // Box and BME sensor vent holes
        union()
        {
            for(i=[0: thickness: inside_length / 4])
            {
                if (vent_slots)
                {
                    translate([12 + i, 0, 1])
                        cube([1.5, thickness + 1, inside_height / 3]);
                    translate([(inside_length - 16) - i, 0, 1])
                        cube([1.5, thickness + 1, inside_height / 3]);
                    translate([(inside_length-16)-i, outside_width + (thickness * 2), 1])
                        cube([1.5, thickness + 1, outside_height / 3]);
                    translate([12 + i, outside_width + (thickness * 2), 1])
                        cube([1.5, thickness + 1, outside_height / 3]);
                }
                if (temp_sensor)
                {
                    // BME sensor vent slots
                    translate([(inside_length - 8), (thickness + 32) - i, 2])
                        cube([thickness + 2, 1.5, 12]);
                }
            }// for
        }// union decoration
    }
    if (oled_window)
    {
        // OLED Display Mount Angled up for easier viewing
        union()
        {
            translate([40, 50, 0.025])
                rotate([-14, 180, 90])
                // Changed dimensions for different OLED 0.96 boards,
                //   board height, board width, combined glass and board thickness
                OLED_Display(28, 27.5, 3.75);
        }
    }
    // Box and mount for BME sensor
    if (temp_sensor)
    {
        difference()
        {
            // Solid box
            translate([(inside_length - (thickness * 2)) - 20, thickness + .01, thickness + -0.001])
                cube([20, 36, 8]);
            // delete the box interior except mounting stud area
            translate([(inside_length - (thickness * 2)) - 18, thickness + 9, thickness + 0.001])
                cube([22, 25, 9]);
            // delete the box interior above mounting stud
            translate([(inside_length - (thickness * 2)) - 18, thickness + -.01, thickness + .001])
                cube([14, 10, 9]);
            // cut out for wires
            translate([(inside_length - (thickness * 2)) - 22, thickness + 33, thickness + 6])
                cube([14, thickness + .1, 10]);
        }
        // Mount hole for BME sensor
        translate([inside_length - (thickness * 2) - 14, thickness + 12, thickness + -0.001])
            box_ic_stud(4, 1, 4); // ic_stud_real_id , ic_stud_od); //2, 4);
    }

    // Project box mounting screw holes
    od = screw_dia * 2.5;
    
	translate([column_offset, column_offset, thickness])
        box_screw(screw_dia, od, box_stud_height);
    
	translate([outside_width - column_offset, column_offset, thickness])
        rotate([0,0,90])
            box_screw(screw_dia, od, box_stud_height);
    
	translate([outside_width - column_offset, outside_length - column_offset, thickness])
        rotate([0,0,180])
            box_screw(screw_dia, od, box_stud_height);

	translate([column_offset, outside_length - column_offset, thickness])
        rotate([0,0,270])
            box_screw(screw_dia, od, box_stud_height);

    if (use_ic_studs)
    {
        //PC Board Studs
        translate([ic_stud_start_x, ic_stud_start_y, thickness])
            box_ic_stud(ic_stud_height, ic_stud_real_id , ic_stud_od);
        translate([ic_stud_start_x,ic_stud_start_y + ic_stud_length, thickness])
            box_ic_stud(ic_stud_height, ic_stud_real_id , ic_stud_od);
        translate([ic_stud_start_x + ic_stud_width, ic_stud_start_y + ic_stud_length, thickness])
            box_ic_stud(ic_stud_height, ic_stud_real_id , ic_stud_od);
        translate([ic_stud_start_x + ic_stud_width, ic_stud_start_y, thickness])
            box_ic_stud(ic_stud_height, ic_stud_real_id , ic_stud_od);        
    }
    // Reset button 83 y, 36 x
    //translate([36, 83, 0])
    //    box_ic_stud(15, 1, 3);
}

module lid()
{
    difference()
    {
        union()
        {
            // Lid
            difference()
            {
                //rounded_box(outside_width, outside_length + (baseOffset * 2), thickness * 4, radius);
                squared_off_box(outside_width, outside_length + (baseOffset * 2), 0, thickness);
                //move slightly to the right to get ride of abandoned surfaces
                translate([-1,0, thickness + extra_lid_thickness]) 
                {
                    //increased width to remove abandoned surfaces
                    cube([outside_width+2, outside_length + (baseOffset * 2), inside_height + thickness * 4]);  
                }
            }
            // Lip
            lip_width = inside_width - lip_tol;
            lip_length = inside_length - lip_tol;
            translate([(outside_width - lip_width)/2, ((outside_length - lip_length)/2) + baseOffset, thickness *.99])
                difference()
                {
                    cube([lip_width, lip_length, lid_lip_height]);
                    translate([lid_lip_width, lid_lip_width, 0])
                    {
                        //added extra z to ensure no remaining surfaces
                        cube([lip_width-(lid_lip_width*2), lip_length-(lid_lip_width*2), lid_lip_height+1]);  
                    }
                }
        
            // Mounting stud columns and holes
            intersection()
            {
                union()
                {
                    translate([column_offset, column_offset + baseOffset, thickness])
                        box_screw(screw_dia, od, lid_stud_height);
                    translate([lid_widthoffset, column_offset + baseOffset, thickness])
                        rotate([0,0,90])
                            box_screw(screw_dia, od, lid_stud_height);
                    translate([lid_widthoffset, lid_lengthoffset + baseOffset, thickness])
                        rotate([0,0,180])
                            box_screw(screw_dia, od, lid_stud_height);
                    translate([column_offset, lid_lengthoffset + baseOffset, thickness])
                        rotate([0,0,270])
                            box_screw(screw_dia, od, lid_stud_height);
                }
                translate([thickness + (lip_tol/2), thickness + (lip_tol/2), 0])
                {
                    cube([lip_width , lip_length + baseOffset, 10]);
                }
            }
        }

        //Holes
        union()
        {
            // Mounting holes
            if (generate_mount)
            {
                MountHole_x = (outside_width / 4);
                MountHole_y = (baseOffset / 2) + (mount_screw_dia / 2);
                translate([MountHole_x, MountHole_y, 0])
                    cylinder(h = (thickness * 2) + 0.100, d = mount_screw_dia, center=true, $fs=0.2);
                translate([MountHole_x * 3, MountHole_y, 0])
                    cylinder(h = (thickness * 2) + 0.100, d = mount_screw_dia, center=true, $fs=0.2);
                translate([MountHole_x, (lid_lengthoffset + baseOffset + 2) + MountHole_y, 0])
                    cylinder(h = (thickness * 2) + 0.100, d = mount_screw_dia, center=true, $fs=0.2);
                translate([MountHole_x * 3, (lid_lengthoffset + baseOffset + 2) + MountHole_y, 0])
                    cylinder(h = (thickness * 2) + 0.100, d = mount_screw_dia, center=true, $fs=0.2);
            }
            
            // Lid mount holes
            translate([column_offset, column_offset+baseOffset, 0])
            {
                if (counter_sunk)
                {
                    cylinder(h = 3, d2=screw_loose_dia, d1=screw_loose_dia * 3, center=true, $fs=0.2);
                }
                else
                    cylinder(h = 2, d = screw_loose_dia * 2, center=true, $fs=0.2);
                cylinder(h = (thickness * 2) + (lid_stud_height * 2) + 0.100, d = screw_loose_dia, center=true, $fs=0.2);
            }
            translate([lid_widthoffset, column_offset+baseOffset, 0])
            {
                if (counter_sunk)
                    cylinder(h = 3, d2=screw_loose_dia, d1=screw_loose_dia * 3, center=true, $fs=0.2);
                else
                    cylinder(h = 2, d = screw_loose_dia * 2, center=true, $fs=0.2);
                cylinder(h = (thickness * 2) + (lid_stud_height * 2) + 0.100, d = screw_loose_dia, center=true, $fs=0.2);
            }
            translate([lid_widthoffset, lid_lengthoffset + baseOffset, 0])
            {
                if (counter_sunk)
                    cylinder(h = 3, d2=screw_loose_dia, d1=screw_loose_dia * 3, center=true, $fs=0.2);
                else
                    cylinder(h = 2, d = screw_loose_dia * 2, center=true, $fs=0.2);
                cylinder(h = (thickness * 2) + (lid_stud_height * 2) + 0.100, d = screw_loose_dia, center=true, $fs=0.2);
            }
            translate([column_offset, lid_lengthoffset + baseOffset, 0])
            {
                if (counter_sunk)
                    cylinder(h = 3, d2=screw_loose_dia, d1=screw_loose_dia * 3, center=true, $fs=0.2);
                else
                    cylinder(h = 2, d = screw_loose_dia * 2, center=true, $fs=0.2);
                cylinder(h = (thickness * 2) + (lid_stud_height * 2) + 0.100, d = screw_loose_dia, center=true, $fs=0.2);
            }
        } // union
        translate([(outside_width / 2) - 2.1, 66+thickness, -0.1])
        {
                cylinder(h=(thickness * 2) + 1.100, d=2.75, center=true, $fs=0.2);                
        }
        // column_offset+baseOffset+16+thickness
        translate([(outside_width / 2) - 2.1, 23+thickness, -0.1])
        {
                cylinder(h=(thickness * 2) + 1.100, d=2.75, center=true, $fs=0.2);                
        }
       
    } // difference
    // PC Board mounting 
    translate([(outside_width / 2) - 2.1, 66+thickness, 0])
    {
        difference()
        {
            cylinder(d=6, h=5, $fn=100);
            cylinder(d=2.75, h=30, $fn=100);
        }        
    } 
    // column_offset+baseOffset+16+thickness
    translate([(outside_width / 2) - 2.1, 23+thickness, 0])
    {
        difference()
        {
            cylinder(d=6, h=5, $fn=100);
            cylinder(d=2.75, h=30, $fn=100);
        }        
    } 
}


if (generate_box)
{
    main_box();
}
if (generate_lid) 
{    
    translate([lid_startpos,0,0])
    lid();
}    


