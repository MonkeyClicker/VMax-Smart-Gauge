"""Rev J drawings: integral raised VMAX lettering, 15 mm high and 1 mm proud."""
from pathlib import Path
from bezel_lettering import lettering, HEIGHT as TEXT_HEIGHT, WIDTH as TEXT_WIDTH, CENTER_Y as TEXT_CENTER_Y
base=Path(__file__).with_name('draw_screwless_concept.py').read_text()
helpers=base.split('# Sheet 1:')[0].replace("'screwless-concept-v1'","'screwless-concept-v2'").replace('REV A','REV J').replace('OF 06','OF 07').replace('Styling P; baseline B; hardware-dependent H','New details P; enclosure-v1 B; hardware-dependent H')
helpers=helpers.replace('V MAX  /  SCREWLESS FLUSH-DASH ENCLOSURE','V MAX  /  IN-DASH MOUNT DESIGN V1')
exec(compile(helpers,__file__,'exec'))
SCREWS=[(x,y) for x in [-68,68] for y in [-48,48]]+[(0,-48),(0,48)]
POSTS=[(x,y) for x in [-40,40] for y in [-48,48]]
def port_holes(cx,cy,labels=False):
    angle=math.acos(-.9)
    for dx in [-30,30]:
        poly([(cx+dx+10*math.cos(-angle+2*angle*i/160),cy+10*math.sin(-angle+2*angle*i/160)) for i in range(161)])
        cross(cx+dx,cy,2)
    circle(cx,cy,7.25);cross(cx,cy,2)
    if labels:
        for dx,name in [(-30,'USB'),(0,'NMEA'),(30,'SD')]:text(cx+dx,cy-15,name,2.6,anchor='middle')
def plan(cx,cy,rear=False):
    octagon(cx-85,cy-65,170,130,10)
    if rear:
        rect(cx-78,cy-58,156,116,8)
        for x,y in SCREWS:circle(cx+x,cy+y,1.7);cross(cx+x,cy+y,2.5)
        # Outside rear view: installed cable plate at Y=-22; X mirrored.
        rect(cx-50,cy-43,100,42,3)
        rect(cx-42,cy-35,84,26,0,[2,1],color=GRAY)
        port_holes(cx,cy-22,True)
        for x in [-45,45]:
            for y in [-38,-6]:circle(cx+x,cy+y,1.7)
    else:
        octagon(cx-82,cy-62,164,124,10-3*(2-math.sqrt(2)))
        octagon(cx-80,cy-60,160,120,10-5*(2-math.sqrt(2)),BLUE)
        octagon(cx-79,cy-59,158,118,10-6*(2-math.sqrt(2)),BLUE)
        rect(cx-61,cy+1.475-39.4,122,78.8,0)
        rect(cx-58,cy+1.475-36.4,116,72.8,0)
        dh(cx-58,cx+58,cy+48,cy+37.875,'116 P')
        dv(cy-34.925,cy+37.875,cx+66,cx+58,'72.8 P')
        for letter in lettering():
            poly([(cx+x,cy+y) for x,y in letter['outer']],fill=HexColor('#e5ebee'))
            for hole in letter['holes']:poly([(cx+x,cy+y) for x,y in hole],fill=white)

start(1,'01  Revised front and rear - 170 x 130 bezel','Rev J: VMAX lettering 15 high x 56 wide, raised 1, centered in bottom bezel land. Rear ports remain on 30 centers.')
for cx,rear in [(111,False),(310,True)]:
    cy=163;plan(cx,cy,rear)
    text(cx-85,242,('REAR / OUTSIDE' if rear else 'FRONT / OUTSIDE')+' / 1:1',3.3,BLUE)
    dh(cx-85,cx+85,86,98,'170 P');dv(98,228,cx-93,cx-85,'130 P')
note_block(26,69,'SCREWLESS FACE / P', ['170 x 130 flange, 10 mm corner clips; 7 total thickness.',
    '3 x 45 deg outer chamfer; 4 remaining edge thickness.',
    'Accent track 1 wide x 0.6 deep, shifted clear of chamfer.',
    'VMAX 15 high x 56 wide, integral raised lettering 1 proud.',
    'Letter center X0, Y-48.4625; equal vertical margins 3.0375.',
    'FONT PROVISIONAL - retained for version 1 checkpoint.'])
note_block(225,69,'THREE-PORT REAR PANEL / P', ['156 x 116 shell; 6 rear-cover screws, 4 cable-panel screws.',
    '100 x 42 panel; rear-cover opening widened to 84 x 26.',
    'USB 20 D-flat / NMEA 14.5 round / SD 20 D-flat.',
    'Optional one-piece rear cover: see sheet 07.'])
finish('01-front-rear')

start(2,'02  All four side elevations','Depth dimensions below start at bezel face Z0. Raised lettering reaches Z-1; cover Z49, port panel Z52. Views at 1:1.')
for x,y,name in [(27,167,'TOP'),(27,65,'BOTTOM')]:
    text(x,y+64,name+' - FRONT EDGE AT TOP',3.3,BLUE)
    rect(x,y+42,170,7);rect(x+7,y+3,156,39);rect(x+7,y,156,3)
    line(x+3,y+46,x+167,y+46)
    rect(x+57,y+49,56,1)
    rect(x+35,y-3,100,3)
    dh(x,x+170,y-13,y+47,'170 P');dv(y,y+49,x+177,x+170,'49 B')
for x,name in [(257,'LEFT'),(339,'RIGHT')]:
    y=100;text(x-3,244,name+' SIDE / FRONT LEFT',3,BLUE)
    poly([(x,y+3),(x+3,y),(x+7,y),(x+7,y+130),(x+3,y+130),(x,y+127)])
    rect(x-1,y+65+TEXT_CENTER_Y-TEXT_HEIGHT/2,1,TEXT_HEIGHT)
    rect(x+7,y+7,39,116);rect(x+46,y+7,3,116)
    rect(x+49,y+22,3,42)
    line(x+7,y+13,x+46,y+13,[2,1],GRAY,.15);line(x+7,y+117,x+46,y+117,[2,1],GRAY,.15)
    dh(x,x+7,y-20,y,'7 P')
    dh(x,x+49,y-10,y,'49 B');dv(y,y+130,x+56,x+52,'130 P')
note_block(249,67,'DEPTH DATUM', ['Front face Z0; glass seat Z4; cavity opens at Z7.',
    'Housing rear Z46; cover Z46 to 49; panel Z49 to 52.',
    'Flange rear Z7: 39 body, 42 cover, 45 panel behind it.',
    'Connectors and panel gasket add further depth H.'])
finish('02-four-side-elevations')

start(3,'03  Housing interior - revised glass support depth','Open rear view; symmetric internal pattern. Glass seat, cavity and retainer stack move another 1 rearward: 2 total relative to v1.')
cx,cy=111,169
octagon(cx-85,cy-65,170,130,10);rect(cx-78,cy-58,156,116,8)
rect(cx-72,cy-52,144,104,5)
rect(cx-64.15,cy-44.025,128.3,88.05,4.3)
rect(cx-58,cy+1.475-36.4,116,72.8,0,[2,1],color=GRAY)
rect(cx-75.8,cy-55.8,151.6,111.6,6.3,color=BLUE)
rect(cx-74.2,cy-54.2,148.4,108.4,4.7,color=BLUE)
for x,y in SCREWS:circle(cx+x,cy+y,4.5);circle(cx+x,cy+y,1.3);cross(cx+x,cy+y,2)
for x,y in POSTS:circle(cx+x,cy+y,4.5);circle(cx+x,cy+y,1.3);cross(cx+x,cy+y,2)
dh(cx-72,cx+72,90,cy-52,'CAVITY 144 B');dv(cy-52,cy+52,18,cx-72,'104 B')
dh(cx-64.15,cx+64.15,cy+34,cy+44.025,'POCKET 128.30 B')
note_block(225,242,'GLASS POCKET B / REVISED DEPTH P', ['Pocket 128.30 x 88.05, R4.30; seat Z4 P.',
    'Window 116 x 72.80, centered at Y+1.475 P.',
    'Screen chamfer 3 x 45 deg; 1 mm straight throat.',
    'Front mouth 122 x 78.8; opening at glass 116 x 72.8.',
    'Nominal glass 127.7 x 87.45; bottom border is wider.',
    'No PCB carrier or PCB mounting-hole attachment.'])
note_block(225,200,'POSTS / B + REVISED FRONT DEPTH P', ['6 rear-cover posts: diameter 9, Z6.8 to 46.',
    'Centers (+/-68, +/-48) and (0, +/-48).',
    'Cover pilot bores diameter 2.6, Z34 to 46.',
    '4 retainer posts: diameter 9, Z6.8 to 12.',
    'Centers X+/-40, Y+/-48.',
    'Retainer pilots diameter 2.6, Z7.4 to 12.'])
note_block(225,157,'REAR SEAL / B', ['Groove centerline 150 x 110, R5.5.',
    'Width 1.6; depth 1.2 from rear rim at Z46.',
    'Nominal 1.5 silicone cord; 20% compression target H.',
    'Cover screw holes are INSIDE the gasket track.',
    'Seal those fasteners separately with sealing washers H.'])
note_block(26,65,'CHANGE FROM REV A', ['Earlier 145 x 110 cavity, PCB carrier, eight cover screws and M4 clamp-anchor pattern are removed.',
    'New interior uses the existing enclosure-v1 pocket, four glass-retaining fingers and six cover posts.',
    '7 mm flange overlap on each side of the 156 x 116 shell; dash attachment is not yet detailed.'])
finish('03-housing-interior')

start(4,'04  Rear cover and replaceable cable panel','Cover and panel viewed from outside rear in installed orientation. Positive Y is up. All plan views at 1:1.')
cx,cy=111,172
rect(cx-78,cy-58,156,116,8)
rect(cx-42,cy-35,84,26)
for x,y in SCREWS:circle(cx+x,cy+y,1.7);cross(cx+x,cy+y)
for x in [-45,45]:
    for y in [-38,-6]:circle(cx+x,cy+y,1.7);cross(cx+x,cy+y)
line(cx-82,cy,cx+82,cy,[4,1],GRAY,.15)
line(cx,cy-62,cx,cy+62,[4,1],GRAY,.15)
dh(cx-78,cx+78,101,114,'156 B');dv(114,230,22,33,'116 B')
text(34,243,'REAR COVER / 3 THICK B',3.3,BLUE)
pcx,pcy=295,198
rect(pcx-50,pcy-21,100,42,3)
for x in [-45,45]:
    for y in [-16,16]:circle(pcx+x,pcy+y,1.7);cross(pcx+x,pcy+y)
port_holes(pcx,pcy,True)
dh(pcx-30,pcx,231,pcy,'30 P');dh(pcx,pcx+30,231,pcy,'30 P')
dh(pcx-50,pcx+50,163,pcy-21,'100 P');dv(pcy-21,pcy+21,pcx+58,pcx+50,'42 B')
text(242,242,'USB / NMEA / SD PANEL / 3 THICK',3.3,BLUE)
note_block(225,141,'PATTERNS / B + P', ['Cover: 6 x diameter 3.4 THRU at (+/-68,+/-48),',
    'and (0,+/-48). Rear opening 84 x 26 at (0,-22) P.',
    'Panel attachment on cover: X+/-45, Y-38 and -6.',
    'Panel local holes: diameter 3.4 at X+/-45, Y+/-16.',
    'All 3 port centers Y0; X=-30, 0, +30 (outside rear).',
    'USB / SD: diameter 20, left flat inset 1 from tangent.',
    'Flat X = center minus 9; flat chord length 8.72.',
    'NMEA: diameter 14.5 (selected within 14-15 range).'])
note_block(26,78,'PORTS VIEWED FROM OUTSIDE REAR', ['LEFT USB power: 20 mm D-flat. CENTER NMEA 2000: 14.5 mm round. RIGHT SD card: 20 mm D-flat.',
    'Both flats are vertical on the LEFT of their own hole, 1 mm inward from the nominal circle tangent.',
    'Use matching revised cover and 100 mm panel: new 84 mm opening leaves 2 mm minimum cutout clearance.',
    'Seal panel/fasteners; verify actual connector nuts, flanges and cable bends. Blank panel retained as fallback.'])
finish('04-cover-cable-panel')

start(5,'05  Four retaining fingers and glass support','Display retention now matches enclosure-v1. The four fingers retain the glass border; there is no large internal carrier plate.')
cx,cy=83,184;k=4
text(28,240,'RETAINER / QTY 4 / 4:1',3.3,BLUE)
rect(cx-4*k,cy-7.5*k,8*k,11*k,1*k)
rect(cx-1.7*k,cy-2.7*k,3.4*k,5.4*k,1.7*k)
cross(cx,cy,4)
dh(cx-16,cx+16,142,154,'8 B');dv(cy-30,cy+14,109,cx+16,'11 B')
leader(cx+6.8,cy,127,205,'Slot 3.4 wide x 5.4 overall B')
leader(cx,cy-30,127,190,'Long end points inward when installed')
note_block(225,241,'RETAINER GEOMETRY / B', ['8 x 11, corner R1, thickness 3.',
    'Local slot center (0,0); part center offset Y-2.',
    'Slot end centers Y+/-1; end radius 1.7.',
    'Installed lower face Z12, upper face Z15 P.',
    'Top pair long ends down; bottom pair long ends up.',
    'Retainer post centers X+/-40, Y+/-48.'])
note_block(225,194,'HARDWARE / H', ['4 M3 x 6 coarse plastic-thread screws, washers.',
    'Pad fingers only against the glass border.',
    'Choose soft silicone/foam pad thickness by measurement.',
    'Do not bear on PCB, LCD module, ribbons or components.',
    'Do not tighten screws to close a glass-to-finger gap.'])
note_block(28,116,'FRONT STACK / P + H', ['Front face Z0; bare glass seat Z4; glass inserts from rear.',
    'Main cavity opens at Z7; posts end at Z12; retaining fingers occupy Z12 to 15.',
    'Actual glass-back height and required pad thickness remain to be measured H.',
    'A thin compatible perimeter seal is planned; no front gasket groove is modeled.',
    'The rear cover can be removed without releasing the display.'])
note_block(28,74,'CAD STATUS', ['Housing STL includes the 170 x 130 screwless flange, perimeter bevel and cosmetic accent groove.',
    'Rear-cover opening widened to 84; panel widened to 100. Retaining fingers retain v1 geometry.',
    'Raised VMAX is included in housing STL. Accent insert and dash mounting hardware remain unresolved.'])
finish('05-display-retainers')

start(6,'06  Installation section and revision dimensions','Depth shown schematically at 1:1. Housing and retained parts are modeled; real display, connectors, seals and dash mounting hardware are not.')
ox,oy=37,169
poly([(ox,oy-62),(ox+3,oy-65),(ox+7,oy-65),(ox+7,oy-44.025),(ox+4,oy-44.025),(ox+4,oy-34.925),(ox+3,oy-34.925),(ox,oy-37.925)],fill=Color(.94,.95,.96))
poly([(ox,oy+62),(ox+3,oy+65),(ox+7,oy+65),(ox+7,oy+44.025),(ox+4,oy+44.025),(ox+4,oy+37.875),(ox+3,oy+37.875),(ox,oy+40.875)],fill=Color(.94,.95,.96))
rect(ox+7,oy-58,39,6,fill=Color(.94,.95,.96));rect(ox+7,oy+52,39,6,fill=Color(.94,.95,.96))
rect(ox+46,oy-58,3,116);rect(ox+49,oy-43,3,42)
line(ox+4,oy-43.725,ox+4,oy+43.725,w=.5)
rect(ox-1,oy+TEXT_CENTER_Y-TEXT_HEIGHT/2,1,TEXT_HEIGHT,fill=HexColor('#e5ebee'))
rect(ox+8,oy+58,5,15,fill=Color(.9,.9,.9));rect(ox+8,oy-73,5,15,fill=Color(.9,.9,.9))
rect(ox+7,oy+58,1,7,color=BLUE);rect(ox+7,oy-65,1,7,color=BLUE)
text(26,245,'SCHEMATIC CENTER SECTION',3.3,BLUE)
leader(ox+4,oy+15,117,209,'Glass seat Z4 P')
leader(ox+1.5,oy+39.375,117,197,'Screen chamfer 3 x 45 deg P')
leader(ox+47,oy+30,117,191,'Cover Z46 to 49 B')
leader(ox+50,oy-18,117,173,'Cable panel Z49 to 52 B')
leader(ox+10,oy+68,117,226,'Dash + 1 compressed gasket H')
leader(ox+1.5,oy-63.5,117,144,'3 x 45 deg outer chamfer P')
leader(ox-.5,oy+TEXT_CENTER_Y,117,127,'Integral VMAX raised 1, height 15 P')
dh(ox,ox+7,99,oy-65,'7 P')
dh(ox,ox+49,88,oy-58,'49 to cover B');dh(ox,ox+52,78,oy-65,'52 to panel B')
note_block(237,241,'ENVELOPE', ['Bezel: 170 x 130 P; clipped corners 10.',
    'Main shell: 156 x 116, R8; rear rim Z46 B.',
    'Outer flange thickness 7; overlap 7 per side P.',
    'Outer chamfer 3 x 45 deg; remaining edge thickness 4.',
    'Rear cover 3 thick; cable panel 3 thick B.',
    'Connectors/gaskets increase the rear envelope H.'])
note_block(237,199,'DASH FIT / H', ['Dash thickness supplied: 3.175 to 6.35.',
    'Existing rear space supplied: 101.6 to 127.',
    'Trial cutout: 157 x 117, R8.5 (0.5/side clearance).',
    'This leaves 6.5 nominal flange overlap at straight sides.',
    'Verify flange stiffness, gasket overlap and cutout first.',
    'Do not cut the dash from this drawing alone.'])
note_block(237,153,'MOUNTING REMAINS TO BE DESIGNED', ['No old M4 clamp anchors or old large carrier remain.',
    'V1 cover posts are preserved without additional holes.',
    'Rear mounting brackets must avoid the cable panel,',
    'cover screw access and the continuous seal track.',
    'No external tilt-bracket mounting is included.'])
note_block(237,110,'MODEL VERIFICATION', ['Housing is a single closed, consistently wound mesh.',
    'Retained nominal parts pass interference checks.',
    'Rear core beyond Z15 matches v1; glass seat and',
    'supports move rearward by 2 total; pivots omitted.',
    'No physical fit, load or water-resistance claim.'])
note_block(27,61,'CONFIGURATION OPTIONS / REV J', ['Raised text Z-1: overall depth to one-piece cover 50; to separate panel 53. Bezel datum Z0 and glass seat Z4 retained.',
    'B = enclosure-v1 baseline; P = new bezel detail; H = hardware-dependent. Dimensions govern; plans at labeled scales.'])
finish('06-section-installation')

start(7,'07  Optional one-piece rear cover - direct bulkhead mounting','Alternative to rear-cover plus cable-panel-three-port. Outside rear view; USB left, NMEA center, SD right. Plan shown at 1:1.')
cx,cy=111,167
rect(cx-78,cy-58,156,116,8)
for x,y in SCREWS:circle(cx+x,cy+y,1.7);cross(cx+x,cy+y,2.5)
port_holes(cx,cy-22,True)
line(cx-83,cy,cx+83,cy,[4,1],GRAY,.15)
line(cx,cy-62,cx,cy+62,[4,1],GRAY,.15)
dh(cx-78,cx+78,95,109,'156 B')
dv(109,225,21,33,'116 B')
dh(cx-30,cx,191,cy-22,'30 P');dh(cx,cx+30,191,cy-22,'30 P')
dv(cy-22,cy,204,189,'22 P')
text(30,240,'OUTSIDE REAR / 3 mm THICK',3.3,BLUE)
note_block(231,241,'ONE-PIECE COVER / P', ['Outline 156 x 116, R8; uniform thickness 3.',
    '6 x diameter 3.4 THRU; existing housing screw pattern.',
    'Centers (+/-68,+/-48) and (0,+/-48).',
    'No separate cable panel, rectangular opening or',
    'four panel attachment holes in this option.'])
note_block(231,201,'BULKHEAD CUTOUTS / OUTSIDE REAR', ['Centers (-30,-22), (0,-22), (+30,-22).',
    'LEFT USB power: nominal diameter 20, left flat 1.',
    'CENTER NMEA 2000: round diameter 14.5.',
    'RIGHT SD card: nominal diameter 20, left flat 1.',
    'Flats lie 9 left of hole center; vertical chord 8.72.',
    '30 mm equal center spacing; 3 mm mounting thickness.'])
note_block(231,156,'ASSEMBLY AND SEAL', ['Install instead of the removable-panel cover assembly.',
    'Cover occupies Z46 to Z49; rear surface at Z49.',
    'Housing rear gasket and six fasteners are retained.',
    'No cable-panel joint or cable-panel gasket is needed.',
    'Seal bulkheads and six cover fasteners as required H.',
    'Check real bulkhead nuts/flanges and cable bends H.'])
note_block(30,73,'PRINT AND CONFIGURATION', ['Print rear-cover-one-piece-three-port.stl flat at 100%, with outside rear surface on the print bed.',
    'Do not install the separate cable panel with this cover. Removable-panel files remain available as an alternative.',
    'Nominal part interference and port-clearance checks pass; connector hardware and physical sealing remain unverified.'])
finish('07-one-piece-rear-cover')

pdf=Canvas(str(OUT/'vmax-screwless-enclosure-drawings-rev-j.pdf'),pagesize=(420*mm,297*mm))
pdf.setTitle('V MAX In-Dash Mount Design V1 - Drawing Rev J')
for name,page in pages:
    pdf.saveState();pdf.scale(mm,mm);renderPDF.draw(page,pdf,0,0);pdf.restoreState();pdf.showPage()
pdf.save()
print(OUT/'vmax-screwless-enclosure-drawings-rev-j.pdf')
