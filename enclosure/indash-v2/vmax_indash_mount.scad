/* VMAX in-dash mount V1: provisional mm dimensions; verify actual hardware. */
$fn=48;part="mount";
module_w=137;module_h=86;visible_w=108;visible_h=66;
fit_clearance=1;bezel_border=12;bezel_thick=5;
sleeve_depth=24;sleeve_wall=3.2;corner_r=9;
snap_offset_y=23;snap_slot_height=11;snap_slot_depth=1.6;snap_slot_width=3.4;
mount_hole_d=4.3;mount_inset=6;
outer_w=module_w+2*(fit_clearance+bezel_border);
outer_h=module_h+2*(fit_clearance+bezel_border);
sleeve_w=module_w+2*(fit_clearance+sleeve_wall);
sleeve_h=module_h+2*(fit_clearance+sleeve_wall);
module rr(w,h,r){offset(r=r)square([w-2*r,h-2*r],center=true);}
module mount(){difference(){union(){
 linear_extrude(height=bezel_thick)rr(outer_w,outer_h,corner_r);
 translate([0,0,-sleeve_depth])linear_extrude(height=sleeve_depth)rr(sleeve_w,sleeve_h,5);
}
translate([0,0,-sleeve_depth-.1])linear_extrude(height=sleeve_depth+1.6)
 rr(module_w+2*fit_clearance,module_h+2*fit_clearance,3);
translate([0,0,1.4])linear_extrude(height=bezel_thick+1)
 rr(visible_w,visible_h,2);
for(side=[-1,1],yy=[-snap_offset_y,snap_offset_y])
 translate([side*(outer_w/2-snap_slot_depth/2+.01),yy,bezel_thick/2])
 cube([snap_slot_depth+.3,snap_slot_height,snap_slot_width],center=true);
for(xx=[-1,1],yy=[-1,1])
 translate([xx*(outer_w/2-mount_inset),yy*(outer_h/2-mount_inset),-.1])
 cylinder(h=bezel_thick+.3,d=mount_hole_d);
}}
if(part=="mount")mount();
