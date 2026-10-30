"""170x130 dash bezel with external-v1 interior, without external pivots.
Reuses original v1 construction functions and internal geometry verbatim.
Outputs a separate revision, leaving original prototype files intact.
"""
from pathlib import Path
import sys, json, struct
from collections import Counter
from bezel_lettering import lettering, HEIGHT as TEXT_HEIGHT, WIDTH as TEXT_WIDTH, RAISE as TEXT_RAISE, CENTER_Y as TEXT_CENTER_Y
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parent/'.tmp-cad-deps'))
source=(ROOT/'CAD'/'external-enclosure-v1.py').read_text(encoding='utf-8')
prefix=source.split('# External pivot bosses:')[0]
prefix=prefix.replace("OUT = ROOT/'STL'/'external-v1'","OUT = ROOT/'STL'/'screwless-v2'")
prefix=prefix.replace("SRC = ROOT/'CAD'/'external-v1'","SRC = ROOT/'CAD'/'screwless-v2'")
prefix=prefix.replace('WINDOW = (110., 66.80)','WINDOW = (116., 72.80)')
prefix=prefix.replace('CHAMFER = .8','CHAMFER = 3.')
prefix=prefix.replace('rounded(*GLASS,DEPTH).move(z=2)','rounded(*GLASS,DEPTH).move(z=4)')
prefix=prefix.replace('rounded(144,104,5,DEPTH).move(z=5)','rounded(144,104,5,DEPTH).move(z=7)')
prefix=prefix.replace('cylinder(4.5,41.2).move(x,y,4.8)','cylinder(4.5,39.2).move(x,y,6.8)')
prefix=prefix.replace('cylinder(4.5,5.2).move(x,y,4.8)','cylinder(4.5,5.2).move(x,y,6.8)')
# Exact throat at Z3 avoids near-coincident cutter intersections at glass seat.
fn_start=prefix.index('def window_chamfer():')
fn_end=prefix.index('# Integral chamfered front',fn_start)
prefix=prefix[:fn_start]+'''def window_chamfer():
    w,h=WINDOW
    pts=[(-w/2-3,-h/2-3),(w/2+3,-h/2-3),(w/2+3,h/2+3),(-w/2-3,h/2+3)]
    scale=(w/(w+6),h/(h+6))
    return S(m.CrossSection([pts]).extrude(3,scale_top=scale),
             'linear_extrude(height=3,scale='+json.dumps(scale)+')polygon(points='+json.dumps(pts)+');').move(y=WINDOW_Y)

'''+prefix[fn_end:]
exec(compile(prefix,str(ROOT/'CAD'/'external-enclosure-v1.py'),'exec'))

# Glass seat Z4; cavity Z7; 3x45 chamfer followed by a 1 mm straight throat.
def clipped(w,h,c,z):
    pts=[(-w/2+c,-h/2),(w/2-c,-h/2),(w/2,-h/2+c),(w/2,h/2-c),
         (w/2-c,h/2),(-w/2+c,h/2),(-w/2,h/2-c),(-w/2,-h/2+c)]
    return S(m.CrossSection([pts]).extrude(z),
             f'linear_extrude(height={z})polygon(points={json.dumps(pts)});')
# Three millimetre 45-degree bevel, measured normal to each perimeter edge.
# True parallel inset at the diagonal corners, not a scaled octagon.
corner_x=75-3*(math.sqrt(2)-1)
corner_y=55-3*(math.sqrt(2)-1)
bottom=[(-corner_x,-62),(corner_x,-62),(82,-corner_y),(82,corner_y),
        (corner_x,62),(-corner_x,62),(-82,corner_y),(-82,-corner_y)]
top=[(-75,-65),(75,-65),(85,-55),(85,55),(75,65),(-75,65),(-85,55),(-85,-55)]
# Use a hull of thin sections to keep the perimeter bevel exact and planar.
lo=m.CrossSection([bottom]).extrude(.001)
hi=m.CrossSection([top]).extrude(.001).translate((0,0,2.999))
bevel=m.Manifold.batch_hull([lo,hi])
code='hull(){linear_extrude(.001)polygon(points='+json.dumps(bottom)+');translate([0,0,2.999])linear_extrude(.001)polygon(points='+json.dumps(top)+');}'
flange=S(bevel,code)+clipped(170,130,10,4).move(z=3)
# Clear added flange material from the original glass pocket and cavity.
flange=flange-rounded(*GLASS,46).move(z=4)-rounded(144,104,5,46).move(z=7)
body=body+flange
body=body-box(*WINDOW,7).move(y=WINDOW_Y,z=-1)-window_chamfer()
# Shift accent inboard of the enlarged bevel, preserving a 2 mm front land.
track=clipped(160,120,10-5*(2-math.sqrt(2)),.61)-clipped(158,118,10-6*(2-math.sqrt(2)),.63).move(z=-.01)
body=body-track.move(z=-.01)
# Mirror front-view X into model X so the raised lettering reads correctly
# when viewed from outside the front (looking along +Z).
def front_to_model(points):
    pts=[(-x,y) for x,y in points]
    area=sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(pts,pts[1:]+pts[:1]))
    return pts if area>0 else list(reversed(pts))
text_solid=None
text_code=[]
for letter in lettering():
    outer=front_to_model(letter['outer']);holes=[front_to_model(h) for h in letter['holes']]
    shape=m.CrossSection([outer])
    for h in holes:shape=shape-m.CrossSection([h])
    solid=shape.extrude(TEXT_RAISE+.05).translate((0,0,-TEXT_RAISE))
    text_solid=solid if text_solid is None else text_solid+solid
    cutters=''.join('polygon(points='+json.dumps(h)+');' for h in holes)
    text_code.append('translate([0,0,-1])linear_extrude(height=1.05)difference(){polygon(points='+json.dumps(outer)+');'+cutters+'}')
assert text_solid is not None
embedded=(text_solid^body.solid).volume()
assert embedded>1,'raised lettering must overlap bezel'
assert abs(text_solid.bounding_box()[5]-.05)<1e-6
body=body+S(text_solid,'union(){'+''.join(text_code)+'}')

# Apply original screw bores, seal track, rear cover, cable panel and retainers.
tail=source[source.index('for x,y in SCREWS:\n    body=body-cylinder'):source.index('# U bracket in assembly coordinates')]
tail=tail.replace('box(64,26,5)','box(84,26,5)')
tail=tail.replace('rounded(80,42,3,3)','rounded(100,42,3,3)')
tail=tail.replace('for x in (-35,35):','for x in (-45,45):')
tail=tail.replace('cylinder(1.3,4.7).move(x,y,5.4)','cylinder(1.3,4.7).move(x,y,7.4)')
exec(compile(tail,str(ROOT/'CAD'/'external-enclosure-v1.py'),'exec'))
constructed_body=body.solid
PORT_PITCH=30.
NMEA_DIAMETER=14.5
ports=[('USB power',-PORT_PITCH,20.,1.),('NMEA 2000',0.,NMEA_DIAMETER,0.),('SD card',PORT_PITCH,20.,1.)]
panel_three=panel
cutters=[]
for name,x,diameter,flat in ports:
    hole=S(m.Manifold.cylinder(5,diameter/2,circular_segments=256),f'cylinder(h=5,d={diameter},$fn=256);').move(z=-1)
    if flat:
        # Keep x >= -radius + flat: a 1 mm deep anti-rotation flat on LEFT.
        hole=hole-box(20,40,7).move(x=-diameter/2+flat-10,z=-2)
    hole=hole.move(x=x)
    cutters.append(hole)
    panel_three=panel_three-hole
    assert abs(x)+diameter/2<42 and diameter/2<13,'cover obstructs cutout'
assert PORT_PITCH-(20+NMEA_DIAMETER)/2>0,'port overlap'
# Alternative cover: uniform 3 mm plate, direct bulkhead cutouts, six rim screws.
# Print local +Y maps to rear-view -Y after the original X180 assembly rotation.
cover_one=rounded(156,116,8,3)
for x,y in SCREWS:cover_one=cover_one-cylinder(1.7,5).move(x,-y,-1)
for hole in cutters:cover_one=cover_one-hole.move(y=22)
parts={'housing':body,'rear-cover':cover,'rear-cover-one-piece-three-port':cover_one,'cable-panel-blank':panel,'cable-panel-three-port':panel_three,'display-retainer-print-4':tab}
report={}
for name,part in parts.items():
    part.solid=part.solid.set_tolerance(.005).simplify(.005)
    mesh=part.solid.to_mesh();v=np.array(mesh.vert_properties)[:,:3];f=np.array(mesh.tri_verts)
    # Binary STL stores float32: weld coincident vertices and remove collapsed
    # triangles before checking the exported mesh, especially at the seat/bevel.
    v,remap=np.unique(v,axis=0,return_inverse=True)
    f=remap[f]
    f=f[(f[:,0]!=f[:,1])&(f[:,1]!=f[:,2])&(f[:,2]!=f[:,0])]
    assert part.solid.status()==m.Error.NoError
    assert len(part.solid.decompose())==1,(name,'disconnected')
    edges=Counter((int(a),int(b)) for tri in f for a,b in zip(tri,np.roll(tri,-1)))
    assert all(edges.get((b,a),0)==n==1 for (a,b),n in edges.items())
    triangles=v[f];norm=np.cross(triangles[:,1]-triangles[:,0],triangles[:,2]-triangles[:,0]);area=np.linalg.norm(norm,axis=1)
    assert area.min()>1e-8,(name,float(area.min()),triangles[area<=1e-8].tolist())
    with (OUT/(name+'.stl')).open('wb') as stream:
        stream.write(f'VMax screwless v2: {name}; mm'.encode().ljust(80,b' '));stream.write(struct.pack('<I',len(f)))
        for n,t in zip(norm/area[:,None],triangles):stream.write(struct.pack('<12fH',*n,*t.flatten(),0))
    (SRC/(name+'.scad')).write_text('// Screwless v2; mm; generated by generate_screwless_v2.py\n'+part.code+'\n')
    report[name]={'bounds_mm':[v.min(0).tolist(),v.max(0).tolist()],'volume_mm3':part.solid.volume(),'triangles':len(f),'closed_manifold':True,'single_connected_part':True}
cover_asm=cover.rotate(x=180).move(z=49);panel_asm=panel_three.move(y=-22,z=49)
tabs=[tab.rotate(z=180 if y<0 else 0).move(x,y,12) for x,y in RETAINERS]
items=[body,cover_asm,panel_asm]+tabs
for i,a in enumerate(items):
    for b in items[i+1:]:assert (a.solid^b.solid).volume()<.01,'assembly interference'
cover_one_asm=cover_one.rotate(x=180).move(z=49)
one_piece_items=[body,cover_one_asm]+tabs
for i,a in enumerate(one_piece_items):
    for b in one_piece_items[i+1:]:assert (a.solid^b.solid).volume()<.01,'one-piece assembly interference'
for hole,port in zip(cutters,ports):
    through=hole.move(y=-22,z=47)
    px=port[1]
    probe=through.solid.translate((-px,22,0)).scale((.999,.999,1)).translate((px,-22,0))
    assert (probe^cover_one_asm.solid).volume()<.001,'one-piece port blocked'
# Glass stack/supports move +2 mm total; compare rear core beyond Z15.001.
old_source=source.split('# External pivot bosses:')[0]
old_source=old_source[old_source.index('# Integral chamfered front'):]
ns=dict(globals());exec(old_source,ns);old=ns['body']
exec(tail,ns);old=ns['body']
# Inset comparison boundary 0.01 to avoid tessellation differences on outer skin.
region=rounded(155.98,115.98,7.99,100).move(z=15.001).solid
delta=((constructed_body^region)-(old.solid^region)).volume()+((old.solid^region)-(constructed_body^region)).volume()
assert delta<.02,('interior mismatch',delta)
# Independently compare interior against the actual existing binary housing STL.
raw=(ROOT/'STL'/'external-v1'/'housing.stl').read_bytes()
count=struct.unpack_from('<I',raw,80)[0]
verts=[]
for i in range(count):
    rec=struct.unpack_from('<12fH',raw,84+50*i)
    verts.extend([rec[3:6],rec[6:9],rec[9:12]])
original=m.Manifold(m.Mesh(np.array(verts,dtype=np.float32),np.arange(len(verts),dtype=np.uint32).reshape(-1,3))).as_original()
# Merge STL duplicate vertices before import if required by the installed kernel.
if original.status()!=m.Error.NoError:
    vertices,indices=np.unique(np.array(verts,dtype=np.float32),axis=0,return_inverse=True)
    original=m.Manifold(m.Mesh(vertices,indices.astype(np.uint32).reshape(-1,3)))
assert original.status()==m.Error.NoError
interior=m.Manifold.cube((144,104,30.999)).translate((-72,-52,15.001))
a=original^interior;b=body.solid^interior
stl_delta=(a-b).volume()+(b-a).volume()
# Export simplification tolerance is 0.005 mm; allow its small aggregate volume.
assert stl_delta<.25,('original STL interior differs',stl_delta)
assert abs((body.solid^m.Manifold.cube((.2,1,8)).translate((80.5,-.5,-.5))).volume()-1.4)<.002,'7 mm flange thickness'
assert abs((body.solid^m.Manifold.cube((.01,1,8)).translate((84.99,-.5,-.5))).volume()-.04)<.0001,'4 mm outer edge residual thickness'
for hole,port in zip(cutters,ports):
    through=hole.move(y=-22,z=47)
    assert (through.solid^cover_asm.solid).volume()<.001,'cover blocks port'
    # Inset probe 0.01 mm radially to allow the 0.005 mm export simplification.
    px=port[1]
    probe=through.solid.translate((-px,22,0)).scale((.999,.999,1)).translate((px,-22,0))
    assert (probe^panel_asm.solid).volume()<.001,'panel port blocked'
report['cable_panel']={'view':'outside rear; left-to-right X increasing','size_mm':[100,42,3],'mounting_hole_centers_mm':[[x,y] for x in [-45,45] for y in [-16,16]],'pitch_mm':PORT_PITCH,'ports':[{'name':n,'center_mm':[x,0],'diameter_mm':dia,'left_flat_inset_mm':flat,'left_flat_x_relative_to_center_mm':-dia/2+flat if flat else None} for n,x,dia,flat in ports],'flat_chord_length_mm':2*math.sqrt(10**2-9**2),'rear_cover_opening_mm':[84,26],'cover_opening_center_in_rear_view_mm':[0,-22],'minimum_port_to_cover_opening_edge_clearance_mm':2,'nmea_diameter_chosen_within_user_range':True,'connector_nut_flange_envelopes_verified':False,'port_clearance_checks':'passed'}
report['one_piece_cover']={'size_mm':[156,116,3],'corner_radius_mm':8,'screws':6,'port_centers_outside_rear_mm':[[-30,-22],[0,-22],[30,-22]],'same_ports_as_removable_panel':True,'panel_joint_and_four_panel_screw_holes_removed':True,'installed_z_mm':[46,49],'pairwise_interference':'passed','port_clearance_checks':'passed','hardware_envelopes_verified':False}
assert WINDOW==(116.,72.8)
# Confirm the specified opening clears through the complete front lip.
opening_probe=box(115.98,72.78,4.01).move(y=WINDOW_Y,z=-.005)
assert (opening_probe.solid^body.solid).volume()<.001,'front window obstructed'
assert CHAMFER==3
# Check chamfer width at front and 45-degree slope at intermediate depth.
for depth,extra in [(0,3),(1,2),(2,1),(2.99,.01)]:
    probe=box(WINDOW[0]+2*extra-.03,WINDOW[1]+2*extra-.03,.002).move(y=WINDOW_Y,z=depth)
    assert (probe.solid^body.solid).volume()<.001,('chamfer obstructed',depth)
seat_probe=box(1,1,.1).move(x=62,y=0,z=3.9).solid
assert abs((body.solid^seat_probe).volume()-.1)<.001,'glass seat is not Z4'
assert (body.solid^box(1,1,.1).move(x=62,y=0,z=4.01).solid).volume()<.001,'glass pocket blocked behind Z4'
report['lettering']={'text':'VMAX','style':'geometric bold polygon outlines','height_mm':TEXT_HEIGHT,'width_mm':TEXT_WIDTH,'raised_mm':TEXT_RAISE,'front_view_center_mm':[0,TEXT_CENTER_Y],'placement':'centered horizontally and vertically in bottom flat land between screen chamfer and accent groove','vertical_land_bounds_mm':[-59,-37.925],'equal_vertical_margins_mm':3.0375,'model_z_mm':[-1,0],'integral_with_housing':True,'front_view_reading_direction_verified_by_coordinate_transform':True}
report['assembly']={'revision':'J','pairwise_interference':'passed','v1_core_difference_behind_z15_001_mm3':delta,'actual_v1_stl_rear_interior_difference_mm3':stl_delta,'window_mm':list(WINDOW),'window_growth_per_edge_mm':3,'window_front_chamfer_mm':3,'window_front_chamfer_degrees':45,'window_front_mouth_mm':[122,78.8],'window_straight_throat_mm':1,'front_bezel_mm':[170,130],'frontmost_text_z_mm':-1,'overall_depth_to_cover_including_text_mm':50,'flange_thickness_mm':7,'outer_chamfer_mm':3,'outer_chamfer_degrees':45,'outer_edge_remaining_thickness_mm':4,'housing_depth_mm':46,'cover_rear_z_mm':49,'panel_rear_z_mm':52,'glass_seat_z_mm':4,'cavity_start_z_mm':7,'retainer_post_top_z_mm':12,'retainer_finger_z_mm':[12,15],'glass_stack_shift_mm':2,'window_y_mm':1.475,'dash_mounting_hardware':'not designed; requires separate validation'}
report['design_release']={'name':'In-dash mount design','version':1,'drawing_revision':'J','font_status':'provisional; user requested later replacement'}
(OUT/'validation.json').write_text(json.dumps(report,indent=2))
(SRC/'assembly.scad').write_text('// Nominal assembly; not a print file\n'+''.join(q.code for q in items))
(SRC/'assembly-one-piece-cover.scad').write_text('// Nominal one-piece-cover alternative; not a print file\n'+''.join(q.code for q in one_piece_items))
print(json.dumps(report,indent=2))
