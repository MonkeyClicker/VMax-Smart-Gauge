"""Shared geometric VMAX outlines in outside-front-view coordinates, mm."""
HEIGHT=15.0
RAISE=1.0
WIDTH=56.0
# Center usable bottom land between the screen chamfer and accent groove.
CENTER_Y=(-37.925-59.0)/2

def lettering():
    shapes=[
        (0,[(0,15),(3.6,15),(6,4),(8.4,15),(12,15),(8.1,0),(3.9,0)],[]),
        (14,[(0,0),(0,15),(3.4,15),(7,7),(10.6,15),(14,15),(14,0),(10.6,0),(10.6,8),(8.4,2.5),(5.6,2.5),(3.4,8),(3.4,0)],[]),
        (30,[(0,0),(4.5,15),(7.5,15),(12,0),(8.6,0),(7.8,3),(4.2,3),(3.4,0)], [[(4.95,6),(7.05,6),(6,10.2)]]),
        (44,[(0,0),(4,7.6),(0,15),(3.8,15),(6,10.2),(8.2,15),(12,15),(8,7.6),(12,0),(8.2,0),(6,4.8),(3.8,0)],[]),
    ]
    def move(points,offset):return [(x+offset-WIDTH/2,y+CENTER_Y-HEIGHT/2) for x,y in points]
    return [{'outer':move(outer,x),'holes':[move(h,x) for h in holes]} for x,outer,holes in shapes]
