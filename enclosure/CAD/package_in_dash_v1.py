"""Package the design v1 checkpoint, excluding superseded working revisions."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import json

ROOT=Path(__file__).resolve().parents[2]
ENC=ROOT/'enclosure'
OUT=ENC/'in-dash-v1'/'vmax-in-dash-mount-v1.zip'
files=[]
for folder,extensions in [
    (ENC/'CAD'/'screwless-v2',{'.scad'}),
    (ENC/'STL'/'screwless-v2',{'.stl','.json','.md'}),
    (ENC/'drawings'/'screwless-concept-v2',{'.svg','.md'}),
]:
    files.extend(p for p in folder.iterdir() if p.suffix in extensions)
files.extend(ENC/'CAD'/name for name in [
    'generate_screwless_v2.py','draw_screwless_v2.py','draw_screwless_concept.py',
    'bezel_lettering.py','external-enclosure-v1.py','package_in_dash_v1.py',
])
files.extend([
    ENC/'drawings'/'screwless-concept-v2'/'vmax-screwless-enclosure-drawings-rev-j.pdf',
    ENC/'STL'/'external-v1'/'housing.stl',
    ENC/'in-dash-v1'/'README.md',
    ROOT/'media'/'graphics'/'vmax-dash-mounted-concept-v2.png',
])
files=sorted(set(files))
manifest={'design':'In-dash mount','version':1,'drawing_revision':'J',
          'font_status':'provisional; replacement remains open',
          'files':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}}
OUT.parent.mkdir(parents=True,exist_ok=True)
with ZipFile(OUT,'w',ZIP_DEFLATED) as archive:
    for p in files:archive.write(p,p.relative_to(ROOT).as_posix())
    archive.writestr('manifest.json',json.dumps(manifest,indent=2))
with ZipFile(OUT) as archive:
    assert archive.testzip() is None
    for name,digest in manifest['files'].items():
        assert hashlib.sha256(archive.read(name)).hexdigest()==digest,name
print(f'{OUT}: {len(files)} files plus verified SHA-256 manifest')
