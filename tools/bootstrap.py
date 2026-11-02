#!/usr/bin/env python3
from pathlib import Path
import hashlib,subprocess,tempfile,shutil
R=Path(__file__).resolve().parents[1]
assets=[('graphics','https://github.com/b00t0x/WhateverXe-Graphics/releases/download/v0.4.0/WhateverXe-Graphics-0.4.0.pkg','7a1b5015b3e6e8b522f26c25df85c313471c9142564a0b57356cddd2c88b9510'),('sdk','https://codeload.github.com/acidanthera/MacKernelSDK/tar.gz/3f750085caa17ec3a7880f11c11bf4f48cd6a164','8931e622b6d16a7052cb2c2dabe16be06963a34b747f19e6cda2bbe8bc7e2557'),('lilu','https://github.com/acidanthera/Lilu/releases/download/1.7.2/Lilu-1.7.2-DEBUG.zip','e95df95d82e8b151047abf359ecd9c853755c59293e19dfcc3907da8fbd8cf89')]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp)
 for name,url,sha in assets:
  archive=tmp/name
  subprocess.run(['curl','--fail','--location','--retry','3',url,'-o',str(archive)],check=True)
  assert hashlib.sha256(archive.read_bytes()).hexdigest()==sha, name+' checksum mismatch'
  if name=='graphics':
   (R/'vendor').mkdir(exist_ok=True);shutil.copy2(archive,R/'vendor/WhateverXe-Graphics-0.4.0.pkg');continue
  dest=R/'source/WhateverXe'/('MacKernelSDK' if name=='sdk' else 'Lilu.kext')
  if dest.exists():continue
  extracted=tmp/(name+'-unpacked');extracted.mkdir()
  if name=='sdk':
   subprocess.run(['tar','-xzf',str(archive),'-C',str(extracted)],check=True);shutil.copytree(next(extracted.iterdir()),dest)
  else:
   subprocess.run(['unzip','-q',str(archive),'-d',str(extracted)],check=True);shutil.copytree(extracted/'Lilu.kext',dest)
print('Dependencies verified')
