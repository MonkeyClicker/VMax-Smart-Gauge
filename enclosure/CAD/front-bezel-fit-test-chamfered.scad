// VMax / Waveshare ESP32-S3-Touch-LCD-5 capacitive, 800x480.
// Units: mm. Print z=0 face down, glass enters from z=4 side.
// Official drawing: ../drawings/waveshare-touch-dimensions.webp
glass_w = 127.70;
glass_h = 87.45;
glass_r = 4;
clearance = 0.30; // per side; design choice, not vendor tolerance
wall = 6;
lip = 2;
pocket_depth = 2; // shallow locating rim, NOT full glass thickness
window_w = 110.00; // 0.50 beyond active area each side
window_h = 66.80;
window_y = 1.475; // active area is 9.35 from top, 12.30 from bottom
window_r = 0; // square opening preserves rectangular active area
chamfer = 0.8; // 45-degree front window edge
$fn = 96;
module rounded_rect(w,h,r) {
    if (r == 0) square([w,h], center=true);
    else offset(r=r) square([w-2*r,h-2*r], center=true);
}
difference() {
    linear_extrude(lip+pocket_depth)
        rounded_rect(glass_w+2*clearance+2*wall,
                     glass_h+2*clearance+2*wall, glass_r+clearance+3);
    translate([0,window_y,-0.01]) linear_extrude(lip+pocket_depth+0.02)
        rounded_rect(window_w,window_h,window_r);
    // Exact tapered rectangle: enlarged at front, nominal at z=chamfer.
    translate([0,window_y,0])
        linear_extrude(height=chamfer,
            scale=[window_w/(window_w+2*chamfer),window_h/(window_h+2*chamfer)])
            square([window_w+2*chamfer,window_h+2*chamfer],center=true);
    translate([0,0,lip]) linear_extrude(pocket_depth+0.01)
        rounded_rect(glass_w+2*clearance,glass_h+2*clearance,glass_r+clearance);
}
