"""Dimensioned vector design drawings; proposed styling on existing dash architecture.
No production CAD/STL geometry is changed. Units: mm. Run with bundled Python.
"""
from pathlib import Path
from reportlab.graphics.shapes import Drawing, Line, Rect, Circle, Polygon, String, PolyLine
from reportlab.graphics import renderPDF, renderSVG
from reportlab.pdfgen.canvas import Canvas
from reportlab.lib.colors import HexColor, Color, white
from reportlab.lib.units import mm
import json
import math

OUT=Path(__file__).resolve().parents[1]/'drawings'/'screwless-concept-v1'
OUT.mkdir(parents=True,exist_ok=True)
INK=HexColor('#172630'); BLUE=HexColor('#176f9a'); GRAY=HexColor('#687984')
pages=[]
def line(x1,y1,x2,y2,dash=None,color=INK,w=.23):
    d.add(Line(x1,y1,x2,y2,strokeColor=color,strokeWidth=w,strokeDashArray=dash))
def text(x,y,s,size=2.7,color=INK,anchor='start'):
    d.add(String(x,y,s,fontName='Helvetica',fontSize=size,fillColor=color,textAnchor=anchor))
def rect(x,y,w,h,r=0,dash=None,fill=None,color=INK,sw=.3):
    d.add(Rect(x,y,w,h,rx=r,ry=r,strokeColor=color,strokeWidth=sw,strokeDashArray=dash,fillColor=fill))
def circle(x,y,r,color=INK,dash=None):
    d.add(Circle(x,y,r,fillColor=None,strokeColor=color,strokeWidth=.23,strokeDashArray=dash))
def poly(points,color=INK,fill=None,w=.3):
    d.add(Polygon([v for p in points for v in p],fillColor=fill,strokeColor=color,strokeWidth=w))
def octagon(x,y,w,h,c=10,color=INK):
    poly([(x+c,y),(x+w-c,y),(x+w,y+c),(x+w,y+h-c),(x+w-c,y+h),(x+c,y+h),(x,y+h-c),(x,y+c)],color)
def cross(x,y,n=3):
    line(x-n,y,x+n,y,[2,1],GRAY,.15); line(x,y-n,x,y+n,[2,1],GRAY,.15)
def arrow(x,y,axis,sign):
    if axis=='h': poly([(x,y),(x+sign*1.9,y+.65),(x+sign*1.9,y-.65)],fill=INK,w=.1)
    else: poly([(x,y),(x+.65,y+sign*1.9),(x-.65,y+sign*1.9)],fill=INK,w=.1)
def dh(x1,x2,y,ext,label):
    line(x1,ext,x1,y+2,color=GRAY,w=.15);line(x2,ext,x2,y+2,color=GRAY,w=.15)
    line(x1,y,x2,y,w=.15);arrow(x1,y,'h',1);arrow(x2,y,'h',-1)
    text((x1+x2)/2,y+1.4,label,2.6,anchor='middle')
def dv(y1,y2,x,ext,label):
    line(ext,y1,x+2,y1,color=GRAY,w=.15);line(ext,y2,x+2,y2,color=GRAY,w=.15)
    line(x,y1,x,y2,w=.15);arrow(x,y1,'v',1);arrow(x,y2,'v',-1)
    text(x+2,(y1+y2)/2,label,2.6)
def leader(x,y,tx,ty,label):
    line(x,y,tx-2,ty+1,w=.18);circle(x,y,.55);text(tx,ty,label,2.5)
def note_block(x,y,title,items,width=0):
    text(x,y,title,3.3,BLUE)
    for i,s in enumerate(items): text(x,y-6-i*4.7,s,2.65)
def start(num,title,subtitle):
    global d
    d=Drawing(420,297)
    rect(8,8,404,281,sw=.35)
    text(14,279,'V MAX  /  SCREWLESS FLUSH-DASH ENCLOSURE',5)
    text(14,271,title,4,BLUE); text(14,264,subtitle,2.7,GRAY)
    line(8,258,412,258,color=GRAY,w=.2)
    line(8,27,412,27,color=GRAY,w=.2)
    text(14,20,'DESIGN PROPOSAL - NOT RELEASED FOR FABRICATION',3,BLUE)
    text(14,13,'Units mm | Dimensions govern | Print A3 at 100% | Styling P; baseline B; hardware-dependent H',2.6)
    text(405,20,f'VM-SD / REV A / {num:02d} OF 06',3,anchor='end')
    text(405,13,'2026-10-30 | View directions labeled individually',2.6,anchor='end')
def finish(name):
    pages.append((name,d)); renderSVG.drawToFile(d,str(OUT/(name+'.svg')))

# Sheet 1: two principal orthographic views. Both drawn at 0.80 scale.
start(1,'01  Exterior front and rear','Front has no visible screws or holes. Rear cover screws and dash clamps remain serviceable behind the dash.')
s=.8
for x,title in [(29,'FRONT - LOOKING INTO SCREEN'),(232,'REAR - LOOKING TOWARD SCREEN')]:
    y=92; w=197*s;h=162*s
    text(x,y+h+15,title,3.2,BLUE)
    if x==29:
        octagon(x,y,w,h,10*s);octagon(x+1*s,y+1*s,w-2*s,h-2*s,9*s)
        octagon(x+3*s,y+3*s,w-6*s,h-6*s,8*s,BLUE)
        octagon(x+4*s,y+4*s,w-8*s,h-8*s,7*s)
        wx=x+(197-110)/2*s;wy=y+(162-66.8)/2*s
        rect(wx,wy,110*s,66.8*s,1*s);rect(wx-.8*s,wy-.8*s,111.6*s,68.4*s,1.8*s)
        rect(x+(197-127.7)/2*s,y+(162-87.45)/2*s,127.7*s,87.45*s,.8*s,[2,1],color=GRAY)
        text(x+w/2,y+14,'V MAX',5,anchor='middle')
        dh(x,x+w,y-10,y,'197 B');dv(y,y+h,x-12,x,'162 B')
        dh(wx,wx+110*s,wy-8,wy,'110 B');dv(wy,wy+66.8*s,wx+110*s+4,wx+110*s,'66.8 B')
        leader(x+3*s,y+h-23, x+12,y+h-18,'1 wide accent inlay P')
        text(x+8,y+24,'Screen opening: R1 B',2.7)
        text(x+8,y+19,'Dashed line: glass outline B',2.7,GRAY)
    else:
        octagon(x,y,w,h,10*s,color=GRAY)
        bx=x+12*s;by=y+12*s
        rect(bx,by,173*s,138*s,6*s)
        for px,py in [(-82,-64),(82,-64),(-82,64),(82,64),(-82,0),(82,0),(0,-64),(0,64)]:
            circle(x+w/2+px*s,y+h/2+py*s,2.6*s)
            cross(x+w/2+px*s,y+h/2+py*s,2)
        for px in [-81.5,81.5]:
            for py in [-32,32]:
                circle(x+w/2+px*s,y+h/2+py*s,2.25*s)
        for px,py in [(-25,-20),(25,-20),(45,25)]:
            cross(x+w/2+px*s,y+h/2+py*s,2.5)
            circle(x+w/2+px*s,y+h/2+py*s,.8*s,dash=[1,.7])
        dh(bx,bx+173*s,y-10,by,'173 B');dv(by,by+138*s,x+w+4,bx+173*s,'138 B')
        text(x+12,y+4,'Pilot marks are on INSIDE face only',2.5,GRAY)
note_block(28,64,'FACE DESIGN', ['10 x 10 corner clips P; 1 x 45 deg perimeter chamfer P.',
    'Accent track: 1 wide x 0.6 deep, outer edge 3 from perimeter P.',
    'Centered V MAX mark: 22 wide x 0.3 engraved P. No front fasteners.'])
note_block(232,64,'REAR ARRANGEMENT', ['8 x M3 cover screws B; 4 x M4 clamp anchors B.',
    'Connector and vent bores remain closed until hardware selection H.',
    'Coordinates on sheet 04 use front-view X; rear view is mirrored.'])
finish('01-front-rear')

# Sheet 2: all four lateral views, 1:1, body bare (clamps shown separately).
start(2,'02  Top, bottom, left and right elevations','Bare housing and cover shown; removable clamps, connector projections and dash gasket excluded from outline.')
def horizontal_view(x,y,label):
    text(x,y+69,label+'  /  1:1',3.3,BLUE)
    rect(x,y+53,197,5);rect(x+12,y+3,173,50);rect(x+12,y,173,3)
    line(x+10,y+53,x+10,y+58);line(x+187,y+53,x+187,y+58)
    line(x+12,y+3,x+185,y+3)
    line(x+26,y+8,x+171,y+8,[3,1],GRAY,.17)
    dh(x,x+197,y-9,y+53,'197 B');dv(y,y+58,x+203,x+197,'58 B')
horizontal_view(27,163,'TOP - LOOKING DOWN')
horizontal_view(27,60,'BOTTOM - LOOKING UP')
def side_view(x,y,label):
    text(x-3,y+151,label+'  /  0.8:1',3.2,BLUE)
    k=.8
    rect(x,y,5*k,162*k);rect(x+5*k,y+12*k,50*k,138*k);rect(x+55*k,y+12*k,3*k,138*k)
    # Dashed inner cavity and glass-seat plane.
    line(x+8*k,y+26*k,x+55*k,y+26*k,[2,1],GRAY,.15)
    line(x+8*k,y+136*k,x+55*k,y+136*k,[2,1],GRAY,.15)
    line(x+5*k,y+37*k,x+5*k,y+125*k,[2,1],GRAY,.15)
    dh(x,x+58*k,y-10,y,'58 B');dv(y,y+162*k,x+58*k+4,x+58*k,'162 B')
    text(x-1,y+140,'FRONT',2.5);text(x+34,y+140,'REAR',2.5)
side_view(258,96,'LEFT SIDE')
side_view(338,96,'RIGHT SIDE')
note_block(254,66,'DEPTH CHAIN B', ['Front flange 5 + body 50 + rear cover 3 = 58 total.',
    'Depth behind flange rear: 53, before fittings.',
    'Top/bottom and left/right external outlines match.',
    'No side holes or external pivot bosses.'])
finish('02-four-side-elevations')

# Sheet 3: open back with features and coordinate pattern.
start(3,'03  Housing internal plan and sealing details','View from open rear, cover removed. X is mirrored relative to front view; dimensions are baseline CAD values unless marked P.')
x,y=28,78;cx=x+98.5;cy=y+81
octagon(x,y,197,162,10);rect(cx-86.5,cy-69,173,138,6)
rect(cx-72.5,cy-55,145,110,3)
rect(cx-64.25,cy-44.125,128.5,88.25,.8)
rect(cx-55,cy-33.4,110,66.8,1,[2,1],color=GRAY)
rect(cx-76.2,cy-58.7,152.4,117.4,6.2,color=BLUE)
rect(cx-73.8,cy-56.3,147.6,112.6,3.8,color=BLUE)
for px,py in [(-82,-64),(82,-64),(-82,64),(82,64),(-82,0),(82,0),(0,-64),(0,64)]:
    circle(cx+px,cy+py,2.1);cross(cx+px,cy+py,3)
for px in [-81.5,81.5]:
    for py in [-32,32]:circle(cx+px,cy+py,2.8);cross(cx+px,cy+py)
for px in [-68,68]:
    for py in [-49,49]:
        circle(cx+px,cy+py,5);circle(cx+px,cy+py,2.1)
line(cx-100,cy,cx+100,cy,[5,1,1,1],GRAY,.15)
line(cx,cy-84,cx,cy+84,[5,1,1,1],GRAY,.15)
dh(cx-72.5,cx+72.5,y-10,cy-55,'CAVITY 145 B')
dv(cy-55,cy+55,x-12,cx-72.5,'110 B')
dh(cx-64.25,cx+64.25,cy+38,cy+44.125,'GLASS POCKET 128.5 B')
note_block(247,240,'BODY FEATURES / B', ['Cavity: 145 x 110, R3; starts at Z = 8.',
    'Glass pocket: 128.5 x 88.25, R0.8; starts Z = 5.',
    'Window: 110 x 66.8, R1; through front flange.',
    'Glass nominal: 127.7 x 87.45; 0.4 clearance/side.',
    '4 carrier posts: diameter 10, centers X +/-68, Y +/-49.',
    'Carrier contact plane: Z = 17.1 H (provisional).'])
note_block(247,199,'REAR FASTENER PATTERNS / B', ['8 M3 insert bores: diameter 4.2 x 6 deep H.',
    'Centers: (+/-82, +/-64), (+/-82, 0), (0, +/-64).',
    '4 M4 insert bores: diameter 5.6 x 6 deep H.',
    'Centers: X +/-81.5, Y +/-32.',
    'All rear inserts enter from Z = 55 toward front.',
    'Insert bore sizes depend on selected hardware H.'])
note_block(247,157,'REAR GASKET / B', ['Track centerline: 150 x 115, R5.',
    'Groove width 2.4; depth 1.4 from Z = 55.',
    'Nominal cord diameter 2.0, continuous sealed joint.',
    'Proposed 30% squeeze; validate selected cord H.',
    'Blue double line identifies the sealing groove.'])
note_block(247,118,'FRONT STYLING / P', ['10 mm clipped corners replace baseline R10 outline.',
    'Outer 1 x 45 deg chamfer; screen lip 0.8 x 45 deg.',
    'Accent track is cosmetic and must not break seal lands.',
    'No front holes; rear insert bores remain blind.'])
note_block(28,52,'Z DATUM', ['Z = 0 at front bezel face; +Z runs into dash. X/Y origin at display center.',
    'Internal plan is dimensioned schematically; ribs at post/wall junctions are not shown.'])
finish('03-body-internals')

# Sheet 4 rear cover detailed, front-view coordinates explicit.
start(4,'04  Rear cover, hole pattern and cable-entry provisions','Cover shown from INSIDE looking rearward, so coordinates match front-view X/Y. Rear external view on sheet 01 is mirrored.')
cx,cy=118,157
rect(cx-86.5,cy-69,173,138,6)
for px,py in [(-82,-64),(82,-64),(-82,64),(82,64),(-82,0),(82,0),(0,-64),(0,64)]:
    circle(cx+px,cy+py,1.7);cross(cx+px,cy+py)
for px in [-81.5,81.5]:
    for py in [-32,32]:circle(cx+px,cy+py,2.25);cross(cx+px,cy+py)
for px,py in [(-25,-20),(25,-20),(45,25)]:
    circle(cx+px,cy+py,1);cross(cx+px,cy+py,4)
line(cx-93,cy,cx+93,cy,[5,1,1,1],GRAY,.15)
line(cx,cy-75,cx,cy+75,[5,1,1,1],GRAY,.15)
text(cx+7,cy+3,'(0,0)',2.5)
dh(cx-86.5,cx+86.5,77,88,'173 B');dv(88,226,22,31.5,'138 B')
dh(cx-82,cx+82,237,221,'164 B')
leader(cx-25,cy-20,74,116,'Cable pilot (-25, -20)')
leader(cx+25,cy-20,147,108,'Cable pilot (+25, -20)')
leader(cx+45,cy+25,153,201,'Vent pilot (+45, +25)')
text(42,247,'INSIDE FACE / 1:1',3.3,BLUE)
note_block(242,240,'COVER / B', ['173 x 138 x 3; corner R6.',
    '8 x diameter 3.4 THRU for M3 cover screws.',
    '4 x diameter 4.5 THRU for M4 clamp anchors.',
    'Hole centers match body pattern on sheet 03.',
    'Use washers; fastener length depends on insert H.'])
note_block(242,200,'CLOSED PILOT DETAILS / B', ['3 x diameter 2 x 0.6 deep, from inside face.',
    'These are blind locating dimples, not open ports.',
    'Remaining cover thickness at each dimple: 2.4.',
    'Cable gland bores and vent bore: diameter TBD H.',
    'Select hardware before drilling or releasing CAD.'])
note_block(242,160,'CONNECTOR ENVELOPE / H', ['Reserve clearance for locknuts, bodies and cable bends.',
    'Check board, carrier, clamp and gasket interference.',
    'This concept retains the flush-dash blank cover.',
    'The external-test two-port plate is a separate design.'])
# Enlarged cover pilot section.
text(244,117,'PILOT SECTION / 5:1',3.3,BLUE)
poly([(248,82),(318,82),(318,97),(288,97),(288,94),(278,94),(278,97),(248,97)])
for k in range(11):
    xx=249+k*6; line(xx,83,xx+4,87,color=GRAY,w=.15)
text(249,103,'INSIDE',2.6);text(249,75,'OUTSIDE - UNBROKEN SURFACE',2.6)
dv(82,97,326,318,'3 B');dh(278,288,110,97,'diameter 2 B')
note_block(29,54,'SEALING LAND', ['Rear gasket seats against flat cover inside face; all fasteners lie outside the continuous gasket path.',
    'Do not countersink or add ports through the sealing land. Pilot detail is enlarged; outline is schematic.'])
finish('04-rear-cover')

# Sheet 5 separate carrier and clamp part details.
start(5,'05  Internal carrier and concealed dash clamps','Four clamps secure the assembly from behind the dash. Front bezel remains completely free of visible hardware.')
cx,cy=110,169
# Boolean boundary extraction preserves the true carrier union and relief edges.
# A rounded rectangle is (center X, center Y, width, height, radius).
outer=(0,0,144,109,3); inner=(0,0,117,69,2)
lands=[(a,b,12,12,6) for a in [-55.5,55.5] for b in [-31.5,31.5]]
cuts=[(-0.,-36.5,104,19,0),(68.5,0,20,44,0)]
cuts += [(a,b,2.8,4.8,1.4) for a in [-55.5,55.5] for b in [-31.5,31.5]]
cuts += [(a,b,3.4,3.4,1.7) for a in [-68,68] for b in [-49,49]]
def inside(p,q):
    x,y=p; a,b,w,h,r=q
    dx=abs(x-a)-(w/2-r);dy=abs(y-b)-(h/2-r)
    return math.hypot(max(dx,0),max(dy,0))+min(max(dx,dy),0)<=r
def carrier_material(p):
    base=(inside(p,outer) and not inside(p,inner)) or any(inside(p,q) for q in lands)
    return base and not any(inside(p,q) for q in cuts)
def boundary(q):
    a,b,w,h,r=q;pts=[]
    if r==0:
        corners=[(a+w/2,b+h/2),(a-w/2,b+h/2),(a-w/2,b-h/2),(a+w/2,b-h/2)]
        for i,p in enumerate(corners):
            e=corners[(i+1)%4];n=max(1,int(math.dist(p,e)/.08))
            pts.extend([(p[0]+(e[0]-p[0])*j/n,p[1]+(e[1]-p[1])*j/n) for j in range(n)])
    else:
        for u,v,t in [(w/2-r,h/2-r,0),(-w/2+r,h/2-r,90),(-w/2+r,-h/2+r,180),(w/2-r,-h/2+r,270)]:
            arc=[(a+u+r*math.cos(math.radians(t+j/2)),b+v+r*math.sin(math.radians(t+j/2))) for j in range(181)]
            if pts:
                p,e=pts[-1],arc[0];n=max(1,int(math.dist(p,e)/.08))
                pts.extend([(p[0]+(e[0]-p[0])*j/n,p[1]+(e[1]-p[1])*j/n) for j in range(1,n)])
            pts.extend(arc)
        p,e=pts[-1],pts[0];n=max(1,int(math.dist(p,e)/.08))
        pts.extend([(p[0]+(e[0]-p[0])*j/n,p[1]+(e[1]-p[1])*j/n) for j in range(1,n)])
    for i,p in enumerate(pts):
        e=pts[(i+1)%len(pts)];dx=e[0]-p[0];dy=e[1]-p[1];n=math.hypot(dx,dy)
        if n<1e-9:continue
        mid=((p[0]+e[0])/2,(p[1]+e[1])/2);nx=-dy/n*.008;ny=dx/n*.008
        if carrier_material((mid[0]+nx,mid[1]+ny)) != carrier_material((mid[0]-nx,mid[1]-ny)):
            line(cx+p[0],cy+p[1],cx+e[0],cy+e[1],w=.25)
for shape in [outer,inner]+lands+cuts:boundary(shape)
text(40,239,'CARRIER - FRONT-VIEW COORDINATES / 1:1',3.2,BLUE)
dh(cx-72,cx+72,103,114.5,'144 B');dv(cy-54.5,cy+54.5,24,cx-72,'109 B')
dh(cx-55.5,cx+55.5,cy+12,cy+31.5,'BOARD GRID 111 B')
note_block(222,241,'CARRIER / B', ['Thickness 3; outer R3; opening 117 x 69, R2.',
    '4 board lands: diameter 12 at X +/-55.5, Y +/-31.5.',
    '4 board slots: 2.8 wide x 4.8 overall (Y +/-1 travel).',
    '4 post holes: diameter 3.4, X +/-68, Y +/-49.',
    'USB relief: X = 58.5 to 78.5, Y = -22 to +22.',
    'Terminal relief: X = -52 to +52, Y = -46 to -27.',
    'Cutters overlap ring; reliefs are open at inner edge.',
    'Verify slot-to-PCB alignment and component clearance H.'])
text(239,181,'CLAMP - QTY 4 / 2:1',3.3,BLUE)
qx,qy=259,142;k=2
rect(qx-9*k,qy-8*k,34*k,16*k,3*k)
circle(qx,qy,2.25*k);circle(qx+16*k,qy,2.25*k)
poly([(qx+32+4.15*k*math.cos(a*math.pi/3),qy+4.15*k*math.sin(a*math.pi/3)) for a in range(6)])
cross(qx,qy,6);cross(qx+32,qy,6)
dh(qx-18,qx+50,116,qy-16,'34 B');dh(qx,qx+32,167,qy,'16 B')
dv(qy-16,qy+16,qx+57,qx+50,'16 B')
note_block(222,103,'CLAMP / B', ['34 x 16 x 7, R3; local X = -9 to +25.',
    '2 x diameter 4.5 THRU at X = 0 and +16, Y = 0.',
    'Nut recess at X = +16: hex 8.3 across corners,',
    '7.19 across flats; 3.5 deep from top face.',
    'Mirror clamp orientation left/right; jack screws outboard.',
    'M4 x 60 jack screws are starting candidates H.'])
note_block(29,74,'CARRIER FIT / H', ['Carrier front contact plane Z = 17.1; mounting holes must land only on board mounting pads.',
    'Board mounting hardware, glass-to-PCB depth and carrier spacers require actual measurements.',
    'Do not use the carrier to pull the glass or PCB into alignment.'])
finish('05-carrier-clamps')

# Sheet 6 axial installation section and design basis.
start(6,'06  Section, installation stack and design basis','Section is schematic at 1:1 in depth; glass/PCB and front seal are simplified. Clamps shown by mounting principle, not full hardware geometry.')
ox,oy=34,151
# Section axes horizontal Z, vertical Y. Front flange ribs top/bottom.
rect(ox,oy-62,5,28,fill=Color(.94,.95,.96));rect(ox,oy+34,5,28,fill=Color(.94,.95,.96))
rect(ox+5,oy-55,50,14,fill=Color(.94,.95,.96));rect(ox+5,oy+41,50,14,fill=Color(.94,.95,.96))
rect(ox+55,oy-55,3,110,fill=Color(.94,.95,.96))
# glass seat nominal and carrier plane
rect(ox+5.6,oy-43.725,1.6,87.45)
line(ox+17.1,oy-50,ox+17.1,oy+50,[3,1],GRAY,.2)
text(ox-10,oy+81,'OUTSIDE',3,BLUE);text(ox+62,oy+81,'INSIDE DASH',3,BLUE)
# dash panel plus gasket at flange back, not through body cavity.
rect(ox+6,oy+55,5,22,fill=Color(.9,.9,.9));rect(ox+6,oy-77,5,22,fill=Color(.9,.9,.9))
rect(ox+5,oy+55,1,7,color=BLUE);rect(ox+5,oy-62,1,7,color=BLUE)
# bracket at rear, jack running forward to dash, drawn at top near Y=+63.
rect(ox+58,oy+53,7,16)
line(ox+11,oy+65,ox+67,oy+65,w=.7)
rect(ox+11,oy+61,2,8)
leader(ox+6.5,oy+20,115,156,'Glass / LCD (schematic)')
leader(ox+17.1,oy+45,115,171,'Carrier contact plane Z17.1 H')
leader(ox+56,oy+15,115,139,'3 thick removable rear cover B')
leader(ox+8,oy+73,115,190,'Dash thickness 3.175 to 6.35 (owner)')
leader(ox+5.5,oy+58,115,181,'Dash gasket: 1 compressed H')
leader(ox+40,oy+65,115,202,'M4 jack screw + broad padded foot H')
dh(ox,ox+58,oy-94,oy-62,'58 total B')
dh(ox+5,ox+58,oy-85,oy-55,'53 behind flange B')
text(29,238,'A-A / CENTER SECTION + SCHEMATIC CLAMP',3.3,BLUE)
note_block(244,240,'DESIGN BASIS', ['B = taken from enclosure/CAD/vmax_dash.scad.',
    'P = proposed screwless styling; requires CAD update.',
    'H = depends on chosen hardware or physical measurement.',
    'The generated hero image is visual inspiration only.',
    'These drawings do not change any existing STL files.'])
note_block(244,200,'FRONT SEAL STACK / H', ['Z0 front face; flange rear / glass seat at Z5 B.',
    'Nominal compressed front seal 0.6.',
    'Glass face at Z5.6; glass-to-PCB mounting land 11.5.',
    'Carrier contact plane Z17.1; carrier thickness 3 B.',
    'Seal glass border without covering active touchscreen.'])
note_block(244,160,'REAR CLEARANCE', ['Reported available depth: 101.6 to 127 behind dash.',
    'Cover behind rear dash face: 45.65 to 48.83 nominal.',
    'Remaining nominal space: 52.78 to 81.35.',
    'Subtract connector projections, cable bends and hardware.',
    'Rear clamp screw adjustment must suit actual dash.'])
note_block(244,119,'BEFORE FABRICATION', ['Reconcile proven fit-test pocket with dash CAD pocket.',
    'Measure module, mount lands and connector envelopes.',
    'Select inserts, seals, glands, vent and mounting pads.',
    'Model new corner clips, chamfers and accent track.',
    'Check rear clamps and finish/seal prototype physically.'])
note_block(29,46,'PROPOSED DASH CUTOUT / B - REFERENCE ONLY', ['174 x 139, corner R6.5; nominal body clearance 0.5 per side.',
    'Use a verified full-size template only after physical assembly fit.',
    'PETG prototype; appearance finish and accent insert construction are proposed.'])
finish('06-section-design-basis')

pdf=Canvas(str(OUT/'vmax-screwless-enclosure-drawings-rev-a.pdf'),pagesize=(420*mm,297*mm))
pdf.setTitle('V MAX Screwless Flush-Dash Enclosure - Design Proposal Rev A')
pdf.setAuthor('VMax Smart Gauge')
for name,page in pages:
    pdf.saveState();pdf.scale(mm,mm);renderPDF.draw(page,pdf,0,0);pdf.restoreState();pdf.showPage()
pdf.save()
(OUT/'drawing-index.json').write_text(json.dumps({'units':'mm','status':'design proposal; not fabrication release','revision':'A','sheets':[p[0] for p in pages],'baseline':'enclosure/CAD/vmax_dash.scad','styling':'screwless dash concept v2','no_stl_changes':True},indent=2))
print(OUT/'vmax-screwless-enclosure-drawings-rev-a.pdf')
