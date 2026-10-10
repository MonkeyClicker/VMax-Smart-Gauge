import importlib.util
from pathlib import Path
spec = importlib.util.spec_from_file_location('validator', Path(__file__).resolve().parents[3] / 'tools/validate_scanner_log.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
def record(seq, payload):
    return f'@{seq} '.encode() + payload + f' *{module.checksum(payload):08X}\n'.encode()
first = record(1, b'CAN busDelta=10')
assert module.validate(first + record(2, b'SNAP end=1')) == (2, 0, 0)
assert module.validate(first + record(3, b'SNAP end=1')) == (2, 0, 1)
assert module.validate(first.replace(b'10', b'11')) == (0, 1, 0)
assert module.validate(first[:-5]) == (0, 1, 0)
assert module.validate(first + first) == (2, 1, 0)
print('Scanner log validator tests passed')
