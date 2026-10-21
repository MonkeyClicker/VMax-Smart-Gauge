// VMax Smart Gauge / Waveshare ESP32-S3-Touch-LCD-5 (28117)
// Mechanical prototype, mm. See README before printing or cutting a dash.
// OpenSCAD: choose part, F6, export STL. All part outputs face the print bed.
part = "assembly"; // [assembly,body,cover,carrier,clamp,fit_test]
$fn = 64;
glass_w = 127.7;
glass_h = 87.45;
glass_clearance = 0.4; // per side; tune with fit_test
window_w = 110; // manufacturer active area 108 x 64.8
window_h = 66.8;
hole_pitch_x = 111;
hole_pitch_y = 63;
hole_offset_y = 0; // verify relative to glass centre on actual board
glass_to_pcb_back = 11.5; // PROVISIONAL: measure before printing carrier/body
front_seal_compressed = 0.6; // target, depends on selected foam/seal
flange_t = 5;
body_depth = 50; // measured from rear of flange to cover seating plane
// Installation dimensions supplied by owner; do not change STL geometry.
dash_t_min = 25.4/8;
dash_t_max = 25.4/4;
behind_dash_min = 4*25.4;
behind_dash_max = 5*25.4;
dash_seal_compressed = 1; // provisional selected foam compression
cavity_w = 145;
cavity_h = 110;
wall = 14; // leaves material around inserts and outside gasket
flange_border = 12;
cover_t = 3;
carrier_t = 3;
gasket_w = 2.4;
gasket_depth = 1.4; // 2 mm round silicone cord: nominal 30% squeeze
gland_d = 0; // 0 = blank pilot dimples; set to actual gland hole size
vent_d = 0; // 0 = blank pilot dimple; set to actual vent hole size
m3_insert_d = 4.2; // supplier-specific; use coupon or measure
m4_insert_d = 5.6; // supplier-specific
insert_depth = 6;
outer_w = cavity_w + 2*wall;
outer_h = cavity_h + 2*wall;
flange_w = outer_w + 2*flange_border;
flange_h = outer_h + 2*flange_border;
rear_z = flange_t + body_depth;
echo(dash_thickness_mm=[dash_t_min,dash_t_max]);
echo(cover_to_dash_rear_gap_mm=[body_depth+cover_t-dash_seal_compressed-dash_t_max,
                               body_depth+cover_t-dash_seal_compressed-dash_t_min]);
carrier_z = flange_t + front_seal_compressed + glass_to_pcb_back;
post_x = cavity_w/2-4.5;
post_y = cavity_h/2-6;
carrier_posts = [[-post_x,-post_y],[post_x,-post_y],[-post_x,post_y],[post_x,post_y]];
cover_x = outer_w/2-4.5;
cover_y = outer_h/2-5;
cover_screws = [[-cover_x,-cover_y],[cover_x,-cover_y],[-cover_x,cover_y],[cover_x,cover_y],[-cover_x,0],[cover_x,0],[0,-cover_y],[0,cover_y]];
anchor_x = outer_w/2-5;
clamp_anchors = [[-anchor_x,-32],[anchor_x,-32],[-anchor_x,32],[anchor_x,32]];
assert(flange_w < 210 && flange_h < 210, "Allow bed margin on Ender 3");
assert(carrier_z > flange_t+4 && carrier_z < rear_z-10, "Check carrier height");

module rr(w,h,r,t) {
  linear_extrude(t) offset(r=r) square([w-2*r,h-2*r],center=true);
}
module ring(w,h,r,iw,ih,ir,t) {
  difference() { rr(w,h,r,t); translate([0,0,-0.01]) rr(iw,ih,ir,t+0.02); }
}
module at_points(p,z=0) { for(q=p) translate([q[0],q[1],z]) children(); }
module rear_groove() {
  translate([0,0,rear_z-gasket_depth])
    ring(cavity_w+5+gasket_w,cavity_h+5+gasket_w,5+gasket_w/2,
         cavity_w+5-gasket_w,cavity_h+5-gasket_w,5-gasket_w/2,gasket_depth+0.1);
}
module post(x,y) {
  // Legs meet the front web, with ribs joining the outer side wall.
  translate([x,y,flange_t]) cylinder(r=5,h=carrier_z-flange_t);
  translate([x<0 ? -outer_w/2 : x,y-3,flange_t])
    cube([outer_w/2-abs(x),6,carrier_z-flange_t]);
}
module body() {
  difference() {
    union() {
      difference() {
        union() { rr(flange_w,flange_h,10,flange_t); translate([0,0,flange_t]) rr(outer_w,outer_h,6,body_depth); }
        translate([0,0,-0.1]) rr(window_w,window_h,1,flange_t+0.2);
        translate([0,0,flange_t]) rr(glass_w+2*glass_clearance,glass_h+2*glass_clearance,0.8,body_depth+0.2);
        translate([0,0,flange_t+3]) rr(cavity_w,cavity_h,3,body_depth);
      }
      for(p=carrier_posts) post(p[0],p[1]);
    }
    rear_groove();
    at_points(cover_screws,rear_z-insert_depth) cylinder(d=m3_insert_d,h=insert_depth+0.1);
    at_points(clamp_anchors,rear_z-insert_depth) cylinder(d=m4_insert_d,h=insert_depth+0.1);
    at_points(carrier_posts,carrier_z-insert_depth) cylinder(d=m3_insert_d,h=insert_depth+0.1);
  }
}
module cover() {
  difference() {
    rr(outer_w,outer_h,6,cover_t);
    at_points(cover_screws,-0.1) cylinder(d=3.4,h=cover_t+0.2);
    at_points(clamp_anchors,-0.1) cylinder(d=4.5,h=cover_t+0.2);
    // Rear cable glands: pass bare cable through before attaching USB plug internally.
    for(x=[-25,25]) translate([x,-20,-0.1])
      if(gland_d>0) cylinder(d=gland_d,h=cover_t+0.2);
      else cylinder(d=2,h=0.7); // blind inside pilot; cover remains closed
    translate([45,25,-0.1])
      if(vent_d>0) cylinder(d=vent_d,h=cover_t+0.2);
      else cylinder(d=2,h=0.7);
  }
}
module carrier() {
  difference() {
    union() {
      ring(cavity_w-1,cavity_h-1,3,117,69,2,carrier_t);
      for(x=[-hole_pitch_x/2,hole_pitch_x/2]) for(y=[-hole_pitch_y/2,hole_pitch_y/2])
        translate([x,y+hole_offset_y,0]) cylinder(d=12,h=carrier_t);
    }
    // ±1 mm Y slots allow small measured glass-to-PCB offset differences.
    for(x=[-hole_pitch_x/2,hole_pitch_x/2]) for(y=[-hole_pitch_y/2,hole_pitch_y/2])
      hull() for(s=[-1,1]) translate([x,y+hole_offset_y+s,-0.1]) cylinder(d=2.8,h=carrier_t+0.2);
    at_points(carrier_posts,-0.1) cylinder(d=3.4,h=carrier_t+0.2);
    // Terminal-block and USB access; confirm against the actual board revision.
    translate([-52,-46,-0.1]) cube([104,19,carrier_t+0.2]);
    translate([58.5,-22,-0.1]) cube([20,44,carrier_t+0.2]);
  }
}
module clamp() {
  // Print four. Anchor hole at x=0; M4 jack screw at x=16 points toward dash.
  difference() {
    translate([8,0,0]) rr(34,16,3,7);
    translate([0,0,-0.1]) cylinder(d=4.5,h=7.2);
    translate([16,0,-0.1]) cylinder(d=4.5,h=7.2);
    translate([16,0,3.5]) cylinder(d=8.3,h=3.6,$fn=6); // M4 nut pocket
  }
}
module fit_test() {
  // Same face opening and glass pocket as body; short and inexpensive.
  difference() {
    rr(glass_w+12,glass_h+12,3,flange_t+3);
    translate([0,0,-0.1]) rr(window_w,window_h,1,flange_t+3.2);
    translate([0,0,flange_t]) rr(glass_w+2*glass_clearance,glass_h+2*glass_clearance,0.8,3.1);
  }
}
module assembly() {
  color("Gainsboro") body();
  color([0.08,0.12,0.16,0.75]) translate([0,0,flange_t+front_seal_compressed]) rr(glass_w,glass_h,0.8,1.6);
  color("orange") translate([0,0,carrier_z+12]) carrier();
  color("silver") translate([0,0,rear_z+28]) cover();
  for(p=clamp_anchors) translate([p[0],p[1],rear_z+cover_t+43])
    rotate([0,0,p[0]<0?180:0]) color("orange") clamp();
}
if(part=="body") body();
else if(part=="cover") cover();
else if(part=="carrier") carrier();
else if(part=="clamp") clamp();
else if(part=="fit_test") fit_test();
else assembly();
