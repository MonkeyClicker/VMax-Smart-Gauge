"""Generate and validate the fit-test STL without external CAD dependencies.

Matches front-bezel-fit-test-chamfered.scad defaults. Run from any working directory.
"""
from pathlib import Path
import math
from collections import Counter

ROOT = Path(__file__).resolve().parents[1]
SEGMENTS = 24

def rounded(w, h, r, z, dy=0):
    points = []
    for cx, cy, start in [(w/2-r,h/2-r,0),(-w/2+r,h/2-r,90),
                           (-w/2+r,-h/2+r,180),(w/2-r,-h/2+r,270)]:
        for i in range(SEGMENTS+1):
            a = math.radians(start + i*90/SEGMENTS)
            points.append((cx+r*math.cos(a),cy+r*math.sin(a)+dy,z))
    return points

def window(z, expansion=0):
    # Subdivide square edges to correspond to the rounded loops, avoiding
    # coincident vertices at zero-radius corners.
    corners = [(55,33.4+1.475),(-55,33.4+1.475),
               (-55,-33.4+1.475),(55,-33.4+1.475)]
    # Each rounded quadrant maps to a quarter of the rectangle perimeter:
    # begin at right/top edge midpoint, proceed counterclockwise.
    mids = [(55,1.475),(0,34.875),(-55,1.475),(0,-31.925)]
    pts=[]
    for k in range(4):
        for i in range(SEGMENTS+1):
            t=i/(SEGMENTS+1)
            if t <= 0.5:
                a,b,u=mids[k],corners[k],t*2
            else:
                a,b,u=corners[k],mids[(k+1)%4],(t-.5)*2
            pts.append((a[0]+(b[0]-a[0])*u,a[1]+(b[1]-a[1])*u,z))
    return [(x + (expansion*x/55), 1.475 + (y-1.475)*(33.4+expansion)/33.4, z) for x,y,z in pts]

verts=[]
loops=[]
for pts in [rounded(140.3,100.05,7.3,0),rounded(140.3,100.05,7.3,4),
            window(0,0.8),window(2),rounded(128.3,88.05,4.3,2),
            rounded(128.3,88.05,4.3,4),window(0.8)]:
    loops.append(list(range(len(verts),len(verts)+len(pts))))
    verts.extend(pts)
faces=[]
def strip(a,b):
    for i in range(len(a)):
        j=(i+1)%len(a)
        faces.extend([(a[i],a[j],b[j]),(a[i],b[j],b[i])])

# Exterior, front face, window wall, glass seat, pocket wall, rim top.
strip(loops[0],loops[1])
strip(loops[2],loops[0])
strip(loops[6],loops[2])
strip(loops[3],loops[6])
strip(loops[4],loops[3])
strip(loops[5],loops[4])
strip(loops[1],loops[5])

def cross(a,b):
    return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def sub(a,b): return tuple(x-y for x,y in zip(a,b))
def dot(a,b): return sum(x*y for x,y in zip(a,b))
edges=Counter()
directed=Counter()
volume=0
for face in faces:
    a,b,c=[verts[i] for i in face]
    n=cross(sub(b,a),sub(c,a))
    assert dot(n,n)>1e-15, 'Degenerate triangle'
    volume+=dot(a,cross(b,c))/6
    for u,v in zip(face,face[1:]+face[:1]):
        edges[tuple(sorted((u,v)))]+=1
        directed[(u,v)]+=1
assert all(n==2 for n in edges.values()), 'Non-manifold edge'
assert all(directed[(v,u)]==n for (u,v),n in directed.items()), 'Inconsistent winding'
assert volume>0, 'Inverted mesh'
for start, sign in [(200,-1),(800,1),(1200,1)]:
    for a,b,c in faces[start:start+200]:
        assert cross(sub(verts[b],verts[a]),sub(verts[c],verts[a]))[2]*sign>0
out=ROOT/'STL'/'front-bezel-fit-test-chamfered.stl'
with out.open('w',encoding='ascii') as f:
    f.write('solid vmax_front_bezel_fit_test\n')
    for face in faces:
        a,b,c=[verts[i] for i in face]
        n=cross(sub(b,a),sub(c,a))
        length=math.sqrt(dot(n,n))
        f.write('  facet normal '+' '.join(f'{v/length:.9g}' for v in n)+'\n    outer loop\n')
        for p in [a,b,c]:
            f.write('      vertex '+' '.join(f'{v:.9f}' for v in p)+'\n')
        f.write('    endloop\n  endfacet\n')
    f.write('endsolid vmax_front_bezel_fit_test\n')
print(f'{out}\n{len(faces)} triangles; closed manifold; consistent winding; volume {volume:.2f} mm^3')
print('Bounds: 140.30 x 100.05 x 4.00 mm')
