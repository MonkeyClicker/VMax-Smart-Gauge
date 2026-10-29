"""VMax external enclosure draft 1. Units mm; front at Z=0, top at +Y.

Requires numpy, manifold3d==3.5.4 and Pillow (preview only).
Writes matching editable OpenSCAD sources, binary STLs, validation JSON,
and a preview. Original fit-test files are never modified.
"""
from pathlib import Path
import json, math, struct
from collections import Counter
import numpy as np
import manifold3d as m

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'STL'/'external-v1'
SRC = ROOT/'CAD'/'external-v1'
OUT.mkdir(exist_ok=True)
SRC.mkdir(exist_ok=True)

# User-proven pocket and window; all other dimensions are draft choices.
GLASS = (128.30, 88.05, 4.30)
WINDOW = (110., 66.80)
WINDOW_Y = 1.475
WIDTH, HEIGHT, DEPTH = 156., 116., 46.
CHAMFER = .8
SCREWS = [(x,y) for x in (-68.,68.) for y in (-48.,48.)]+[(0.,-48.),(0.,48.)]
RETAINERS = [(x,y) for x in (-40.,40.) for y in (-48.,48.)]

class S:
    def __init__(self, solid, code): self.solid,self.code=solid,code
    def __add__(self, b): return S(self.solid+b.solid,'union(){'+self.code+b.code+'}')
    def __sub__(self, b): return S(self.solid-b.solid,'difference(){'+self.code+b.code+'}')
    def move(self,x=0,y=0,z=0):
        return S(self.solid.translate((x,y,z)),f'translate([{x},{y},{z}]){{{self.code}}}')
    def rotate(self,x=0,y=0,z=0):
        return S(self.solid.rotate((x,y,z)),f'rotate([{x},{y},{z}]){{{self.code}}}')

def box(w,h,d):
    return S(m.Manifold.cube((w,h,d)).translate((-w/2,-h/2,0)),
             f'translate([{-w/2},{-h/2},0])cube([{w},{h},{d}]);')

def cylinder(r,d):
    return S(m.Manifold.cylinder(d,r,circular_segments=64),f'cylinder(h={d},r={r},$fn=64);')

def outline(w,h,r):
    pts=[]
    for cx,cy,a in [(w/2-r,h/2-r,0),(-w/2+r,h/2-r,90),
                    (-w/2+r,-h/2+r,180),(w/2-r,-h/2+r,270)]:
        for i in range(17):
            t=math.radians(a+90*i/16)
            pts.append((cx+r*math.cos(t),cy+r*math.sin(t)))
    return pts

def rounded(w,h,r,d):
    pts=outline(w,h,r)
    return S(m.CrossSection([pts]).extrude(d),
             'linear_extrude(height='+str(d)+')polygon(points='+json.dumps(pts)+');')

def window_chamfer():
    w,h=WINDOW
    pts=[(-w/2-CHAMFER,-h/2-CHAMFER),(w/2+CHAMFER,-h/2-CHAMFER),
         (w/2+CHAMFER,h/2+CHAMFER),(-w/2-CHAMFER,h/2+CHAMFER)]
    # Extend the cutter 0.01 mm into the straight opening, avoiding coincident
    # Boolean faces while retaining a 45-degree bevel to Z=0.8 exactly.
    scale=((w-.02)/(w+2*CHAMFER),(h-.02)/(h+2*CHAMFER))
    return S(m.CrossSection([pts]).extrude(CHAMFER+.01,scale_top=scale),
             f'linear_extrude(height={CHAMFER+.01},scale=[{scale[0]},{scale[1]}])polygon(points={json.dumps(pts)});').move(y=WINDOW_Y)

# Integral chamfered front and open-backed shell. Front glass seat Z=2.
body=rounded(WIDTH,HEIGHT,8,DEPTH)
body=body-box(*WINDOW,7).move(y=WINDOW_Y,z=-1)-window_chamfer()
body=body-rounded(*GLASS,DEPTH).move(z=2)
body=body-rounded(144,104,5,DEPTH).move(z=5)
for x,y in SCREWS:
    body=body+cylinder(4.5,41.2).move(x,y,4.8)
for x,y in RETAINERS:
    body=body+cylinder(4.5,5.2).move(x,y,4.8)
# External pivot bosses: blind tapped holes never penetrate the enclosure wall.
for side in (-1,1):
    boss=cylinder(10,13).rotate(y=90*side).move(side*77.4,0,27)
    body=body+boss
    # 45-degree ribs grow outward from the wall under the pivot bosses.
    rib_pts=[(77.4,4),(90.4,17),(90.4,22),(77.4,22)]
    rib=S(m.CrossSection([rib_pts]).extrude(14),
          f'linear_extrude(height=14)polygon(points={json.dumps(rib_pts)});')
    rib=rib.rotate(x=90).move(y=7)
    if side<0: rib=rib.rotate(z=180)
    body=body+rib
    bore=cylinder(1.65,9).rotate(y=-90*side).move(side*90.5,0,27)
    body=body-bore
for x,y in SCREWS:
    body=body-cylinder(1.3,12.1).move(x,y,34)
for x,y in RETAINERS:
    body=body-cylinder(1.3,4.7).move(x,y,5.4)
# Continuous rear gasket track, for 1.5 mm silicone cord; seat Z=44.8.
groove=rounded(151.6,111.6,6.3,1.3)-rounded(148.4,108.4,4.7,1.5).move(z=-.1)
body=body-groove.move(z=44.8)

# Cover is exported in print orientation, outer rear face on bed.
# Assembly transform: rotate X=180 then translate Z=49.
cover=rounded(WIDTH,HEIGHT,8,3)
cover=cover-box(64,26,5).move(y=22,z=-1)
for x,y in SCREWS:
    cover=cover-cylinder(1.7,5).move(x,-y,-1)
for x in (-35,35):
    for y in (6,38):
        cover=cover-cylinder(1.7,5).move(x,y,-1)

# Replaceable blank rear cable panel; drill only after connector selection.
panel=rounded(80,42,3,3)
for x in (-35,35):
    for y in (-16,16):
        panel=panel-cylinder(1.7,5).move(x,y,-1)

# Four separate glass-retaining fingers. Hole has 2 mm of adjustment along Y.
tab=rounded(8,11,1,3).move(y=-2)
slot=cylinder(1.7,5).move(y=-1,z=-1)+cylinder(1.7,5).move(y=1,z=-1)+box(3.4,2,5).move(z=-1)
tab=tab-slot

# U bracket in assembly coordinates; M4 pivot clearance, M5 base mounting.
# Circular lower bore with a tangent 45-degree roof, pointing toward +Z.
# Keeps the nominal screw clearance while avoiding unsupported hole ceilings
# when the bracket is printed on its newly coplanar Z=12 side.
def roofed_bore(r, length):
    pts=[(r*math.cos(math.radians(a)),r*math.sin(math.radians(a)))
         for a in np.linspace(135,405,49)]
    pts.append((0,r*math.sqrt(2)))
    bore=S(m.CrossSection([pts]).extrude(length),
           f'linear_extrude(height={length})polygon(points={json.dumps(pts)});')
    return bore.rotate(x=90)

bracket=box(192,8,30).move(y=-66,z=12)
for side in (-1,1):
    # Extend the front edge by 5 mm; rear edge and pivot centers stay fixed.
    bracket=bracket+box(5.5,75,25).move(side*93.25,-25.5,12)
    bracket=bracket-roofed_bore(2.2,10).rotate(z=90).move(side*93.25-5,0,27)
for x in (-60,60):
    bracket=bracket-roofed_bore(2.7,12).move(x,-60,27)
# Print on front side: complete U footprint on bed, only 30 mm high.
bracket_print=bracket.move(y=70,z=-12)
assert np.allclose(bracket_print.solid.bounding_box(),(-96,0,0,96,82,30))
# All three members start on the bed; no suspended arm foundations.
assert abs(bracket_print.solid.slice(0.1).area()-2350)<0.01

parts={'housing':body,'rear-cover':cover,'cable-panel-blank':panel,
       'display-retainer-print-4':tab,'tilt-bracket':bracket_print}
report={}
for name,part in parts.items():
    part.solid=part.solid.set_tolerance(0.005).simplify(0.005)
    mesh=part.solid.to_mesh()
    v=np.array(mesh.vert_properties)[:,:3]
    f=np.array(mesh.tri_verts)
    assert part.solid.status()==m.Error.NoError, (name,part.solid.status())
    assert len(part.solid.decompose())==1, (name,'disconnected bodies')
    edges=Counter()
    for a,b,c in f:
        for u,w in ((a,b),(b,c),(c,a)): edges[(int(u),int(w))]+=1
    assert all(edges.get((w,u),0)==n==1 for (u,w),n in edges.items()), (name,'edge winding/manifold')
    triangles=v[f]
    normal=np.cross(triangles[:,1]-triangles[:,0],triangles[:,2]-triangles[:,0])
    areas=np.linalg.norm(normal,axis=1)
    assert areas.min()>1e-8,(name,'degenerate triangle',areas.min(),triangles[areas<=1e-8].tolist())
    assert part.solid.volume()>0
    with (OUT/(name+'.stl')).open('wb') as stream:
        stream.write((f'VMax external draft 1: {name}; units mm').encode().ljust(80,b' '))
        stream.write(struct.pack('<I',len(f)))
        for n,tri in zip(normal/areas[:,None],triangles):
            stream.write(struct.pack('<12fH',*n,*tri.flatten(),0))
    (SRC/(name+'.scad')).write_text('// VMax external draft 1; mm; generated by external-enclosure-v1.py\n'+part.code+'\n')
    report[name]={'triangles':len(f),'volume_mm3':round(part.solid.volume(),2),
                  'bounds_mm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()],
                  'closed_manifold':True,'single_connected_part':True}

cover_asm=cover.rotate(x=180).move(z=49)
panel_asm=panel.move(y=-22,z=49)
tabs=[]
for x,y in RETAINERS:
    tabs.append(tab.rotate(z=180 if y<0 else 0).move(x,y,10))
# No component collisions in nominal dry assembly (gasket excluded).
assembly=[body,cover_asm,panel_asm,bracket]+tabs
collisions=[]
for i,a in enumerate(assembly):
    for j,b in enumerate(assembly[i+1:],i+1):
        overlap=(a.solid ^ b.solid).volume()
        if overlap>0.01: collisions.append([i,j,overlap])
assert not collisions,collisions
report['assembly']={'interference_check':'passed','hardware_and_display_not_modeled':True,
                    'glass_seat_z_mm':2,'rear_cover_z_mm':[46,49],
                    'nominal_pocket_mm':list(GLASS),'window_mm':list(WINDOW),
                    'body_excluding_pivots_mm':[156,116,46]}
(OUT/'validation.json').write_text(json.dumps(report,indent=2))
assembly_code=body.code+cover_asm.code+panel_asm.code+bracket.code+''.join(t.code for t in tabs)
(SRC/'assembly.scad').write_text('// Nominal assembly, mm. For review; do not print assembled.\n'+assembly_code)
print(json.dumps(report,indent=2))

# Render two views using actual generated triangles (no external renderer).
from PIL import Image, ImageDraw, ImageFont
canvas=Image.new('RGB',(1600,1080),'#eef2f5')
draw=ImageDraw.Draw(canvas)
try:
    font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',21)
    title=ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf',30)
except OSError: font=title=ImageFont.load_default()
draw.text((40,22),'VMax — external enclosure / draft 1',font=title,fill='#162b3a')
draw.text((40,68),'Proven glass pocket • chamfered front • serviceable back • removable tilt bracket',font=font,fill='#344c5e')

def render(items,eye,rect,up_axis=(0,1,0)):
    eye=np.array(eye,dtype=float); eye/=np.linalg.norm(eye)
    right=np.cross(up_axis,eye);right/=np.linalg.norm(right)
    up=np.cross(eye,right)
    alltris=[]
    for part,color in items:
        mesh=part.solid.to_mesh(); vv=np.array(mesh.vert_properties)[:,:3]
        for tri in vv[np.array(mesh.tri_verts)]:
            norm=np.cross(tri[1]-tri[0],tri[2]-tri[0]); norm/=np.linalg.norm(norm)
            if np.dot(norm,eye)<=0: continue
            uv=np.column_stack((tri@right,-tri@up))
            light=.5+.5*max(0,np.dot(norm,np.array([.3,.7,.64])))
            rgb=tuple(int(c*light) for c in color)
            alltris.append((float(tri.mean(axis=0)@eye),uv,rgb,tri@eye))
    points=np.concatenate([t[1] for t in alltris]);lo=points.min(axis=0);hi=points.max(axis=0)
    x,y,w,h=rect; scale=min(w/(hi[0]-lo[0]),h/(hi[1]-lo[1]))
    offset=np.array([x+w/2,y+h/2])-(lo+hi)*scale/2
    pixels=np.array(canvas)
    depth=np.full(pixels.shape[:2],-np.inf)
    for _,uv,rgb,zz in alltris:
        q=uv*scale+offset
        low=np.maximum(np.floor(q.min(axis=0)).astype(int),[0,0])
        high=np.minimum(np.ceil(q.max(axis=0)).astype(int),[pixels.shape[1]-1,pixels.shape[0]-1])
        if np.any(high<low): continue
        xx,yy=np.meshgrid(np.arange(low[0],high[0]+1)+.5,np.arange(low[1],high[1]+1)+.5)
        a,b,c=q
        den=(b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1])
        if abs(den)<1e-10: continue
        u=((b[1]-c[1])*(xx-c[0])+(c[0]-b[0])*(yy-c[1]))/den
        v=((c[1]-a[1])*(xx-c[0])+(a[0]-c[0])*(yy-c[1]))/den
        w=1-u-v
        dd=u*zz[0]+v*zz[1]+w*zz[2]
        region=depth[low[1]:high[1]+1,low[0]:high[0]+1]
        mask=(u>=-1e-8)&(v>=-1e-8)&(w>=-1e-8)&(dd>region)
        region[mask]=dd[mask]
        pixels[low[1]:high[1]+1,low[0]:high[0]+1][mask]=rgb
    canvas.paste(Image.fromarray(pixels))

blue=(39,116,150); gray=(128,148,162); orange=(227,146,57)
# Dark glass placeholder for visual context only; not a manufactured part.
glass=rounded(127.7,87.45,4,.5).move(z=2.1)
render([(body,blue),(glass,(24,33,42)),(cover_asm,gray),(panel_asm,orange),(bracket,gray)],
       [1.4,1.0,-2.5],(45,160,700,650))
draw.text((50,840),'Front / mounted assembly',font=title,fill='#162b3a')
draw.text((50,888),'156 × 116 mm body; 49 mm with rear cover',font=font,fill='#344c5e')
draw.text((50,923),'Side pivot bosses increase width to 180.8 mm',font=font,fill='#344c5e')
render([(body,blue),(cover_asm.move(z=30),gray),(panel_asm.move(z=60),orange)]
       +[(t,(207,175,91)) for t in tabs], [1.5,1.2,2.5],(825,155,720,655))
draw.text((830,840),'Rear / exploded service view',font=title,fill='#162b3a')
draw.text((830,888),'Blank cable panel: choose ports after cable trials',font=font,fill='#344c5e')
draw.text((830,923),'Six cover screws • four internal display retainers',font=font,fill='#344c5e')
draw.text((40,1015),'DRAFT: dry-fit hardware and verify retention, cable clearance, mounting and seals before boat use.',font=font,fill='#7e4b20')
canvas.save(ROOT/'drawings'/'external-enclosure-v1.png')

# Separate print-orientation preview for the revised bracket.
canvas=Image.new('RGB',(1600,1080),'#eef2f5')
draw=ImageDraw.Draw(canvas)
draw.text((40,25),'Tilt bracket — side-print revision',font=title,fill='#162b3a')
draw.text((40,78),'192 × 82 mm footprint · 30 mm print height · flat U-shaped face on bed',font=font,fill='#344c5e')
render([(bracket_print,blue)],[1,-1.6,2.2],(70,155,1460,720),up_axis=(0,0,1))
draw.text((40,930),'Mounting centers preserved: M5 base holes 120 mm apart; M4 pivots at original locations.',font=font,fill='#344c5e')
draw.text((40,975),'45° hole roofs; support-free design. Verify fit and sliced preview before printing.',font=font,fill='#344c5e')
canvas.save(ROOT/'drawings'/'tilt-bracket-side-print.png')
