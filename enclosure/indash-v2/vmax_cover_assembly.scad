// VMAX matched cover V2. All dimensions provisional, units mm.
// Match enclosure/STL/in-dash-v1/vmax_indash_mount.scad
$fn=48;
part="assembly"; // front, rear, coupon, assembly
module_w=137; module_h=86; fit_clearance=1; bezel_border=12;
bezel_thick=5; corner_r=9; snap_offset_y=23;
side_clearance=.45; cover_wall=2.8; cover_depth=9;
front_thick=2.8; adhesive_flange=5;
clip_height=8; clip_nub_z=2; clip_nub_depth=1; clip_relief_y=1.1;
outer_w=module_w+2*(fit_clearance+bezel_border);
outer_h=module_h+2*(fit_clearance+bezel_border);
inner_w=outer_w+2*side_clearance; inner_h=outer_h+2*side_clearance;
cover_w=inner_w+2*cover_wall; cover_h=inner_h+2*cover_wall;
top_z=bezel_thick+front_thick+1.2;
rear_top_z=top_z-front_thick; rear_bottom_z=rear_top_z-cover_depth;
module rr(w,h,r){offset(r=r)square([w-2*r,h-2*r],center=true);}
module ring(ow,oh,iw,ih,r,ir,z,h){
 translate([0,0,z])linear_extrude(height=h)
 difference(){rr(ow,oh,r);rr(iw,ih,ir);}
}
module front(){
 difference(){
  union(){
   translate([0,0,rear_top_z])linear_extrude(height=front_thick)
    rr(cover_w,cover_h,corner_r+cover_wall);
   translate([0,0,top_z])linear_extrude(height=.65)
    difference(){rr(cover_w-7,cover_h-7,corner_r);
                 rr(cover_w-13,cover_h-13,corner_r-2);}
   for(s=[-1,1])translate([s*(cover_w/2+3),0,rear_top_z+.7])
    cube([9,28,2.4],center=true);
  }
  for(x=[-1,1],y=[-1,1])
   translate([x*(cover_w/2-7),y*(cover_h/2-7),rear_top_z-.01])
    cylinder(d=3,h=1.5);
 }
}
module rear(){
 difference(){
  union(){
   ring(cover_w,cover_h,inner_w,inner_h,corner_r+cover_wall,corner_r,rear_bottom_z,cover_depth);
   ring(cover_w,cover_h,inner_w-2*adhesive_flange,inner_h-2*adhesive_flange,
        corner_r+cover_wall,corner_r-2,rear_top_z-1.6,1.6);
   for(x=[-1,1],y=[-1,1])
    translate([x*(cover_w/2-7),y*(cover_h/2-7),rear_top_z])
     cylinder(d=2.55,h=1.2);
   for(s=[-1,1],yy=[-snap_offset_y,snap_offset_y])
    translate([s*(inner_w/2-clip_nub_depth/2),yy,bezel_thick/2])
     cube([clip_nub_depth,clip_height,clip_nub_z],center=true);
  }
  for(s=[-1,1],yy=[-snap_offset_y,snap_offset_y],edge=[-1,1])
   translate([s*(cover_w/2-cover_wall/2),
              yy+edge*(clip_height/2+clip_relief_y/2),rear_bottom_z+2])
    cube([cover_wall+1,clip_relief_y,cover_depth-1.9],center=true);
 }
}
module coupon(){
 translate([-13,0,0])difference(){
  cube([8,24,5]);translate([-.1,7,1])cube([1.7,11,3.4]);
 }
 translate([8,0,0])difference(){
  union(){cube([3,24,8]);translate([-1,8,1.5])cube([1.1,8,2]);}
  for(y=[6.9,17.1])translate([-.1,y,1.5])cube([3.3,1.1,6.6]);
 }
}
if(part=="front")front();
else if(part=="rear")rear();
else if(part=="coupon")coupon();
else {
 color("#51565e")linear_extrude(height=bezel_thick)rr(outer_w,outer_h,corner_r);
 color("#1a1d23",.8)rear();
 color("#2c3038",.85)front();
}
