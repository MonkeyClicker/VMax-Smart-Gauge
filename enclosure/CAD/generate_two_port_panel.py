"""Generate external-v1 replacement cable panel. Requires numpy/manifold3d.
Units mm. USB cutout provisional; connector nut/flange envelopes unverified.
"""
from pathlib import Path
from collections import Counter
import json, math, struct
import numpy as np
import manifold3d as m

ROOT=Path(__file__).resolve().parents[1]
W,H,T,R=80.,42.,3.,3.
PORTS=[('NMEA',-17.,16.),('USB-C',17.,16.)]
pts=[]
for cx,cy,a in [(W/2-R,H/2-R,0),(-W/2+R,H/2-R,90),
                (-W/2+R,-H/2+R,180),(W/2-R,-H/2+R,270)]:
    for i in range(17):
        t=math.radians(a+i*90/16)
        pts.append((cx+R*math.cos(t),cy+R*math.sin(t)))
panel=m.CrossSection([pts]).extrude(T)
holes=[(x,y,3.4) for x in (-35,35) for y in (-16,16)]
holes += [(x,0,d) for _,x,d in PORTS]
for x,y,d in holes:
    panel=panel-m.Manifold.cylinder(5,d/2,circular_segments=96).translate((x,y,-1))
assert panel.status()==m.Error.NoError and len(panel.decompose())==1
mesh=panel.to_mesh(); v=np.array(mesh.vert_properties)[:,:3]; f=np.array(mesh.tri_verts)
edges=Counter((int(a),int(b)) for tri in f for a,b in zip(tri,np.roll(tri,-1)))
assert all(edges.get((b,a),0)==n==1 for (a,b),n in edges.items())
tri=v[f]; norm=np.cross(tri[:,1]-tri[:,0],tri[:,2]-tri[:,0]); size=np.linalg.norm(norm,axis=1)
assert size.min()>1e-8 and panel.volume()>0
# Holes must remain inside the 64 x 26 rear-cover opening and separated.
assert all(abs(x)+d/2<32 and d/2<13 for _,x,d in PORTS)
assert PORTS[1][1]-PORTS[0][1]>(PORTS[0][2]+PORTS[1][2])/2
out=ROOT/'STL'/'external-v1'/'cable-panel-two-port.stl'
with out.open('wb') as stream:
    stream.write(b'VMax two-port draft; 16mm NMEA and provisional USB-C; units mm'.ljust(80,b' '))
    stream.write(struct.pack('<I',len(f)))
    for n,t in zip(norm/size[:,None],tri):stream.write(struct.pack('<12fH',*n,*t.flatten(),0))
scad='// mm. NMEA left, provisional USB-C right, viewed from outside rear.\n'
scad+='// Both cutouts 16 mm; verify USB fit and nut/flange clearance.\n'
scad+='difference(){linear_extrude(height=3)polygon(points='+json.dumps(pts)+');\n'
for x,y,d in holes: scad+=f'translate([{x},{y},-1])cylinder(h=5,d={d},$fn=96);\n'
scad+='}\n'
(ROOT/'CAD'/'external-v1'/'cable-panel-two-port.scad').write_text(scad)
report={'closed_manifold':True,'single_connected_part':True,'triangles':len(f),
        'bounds_mm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()],
        'port_holes_inside_rear_cover_opening':True,
        'ports':[{'name':name,'center_mm':[x,0],'diameter_mm':d} for name,x,d in PORTS],
        'usb_diameter_provisional':True,'flange_nut_envelopes_verified':False}
(out.parent/'cable-panel-two-port-validation.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
