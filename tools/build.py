#!/usr/bin/env python3
from pathlib import Path
import subprocess,shutil,tempfile,xml.etree.ElementTree as ET,hashlib,json,sys,plistlib
R=Path(__file__).resolve().parents[1]
def run(*a):subprocess.run(list(map(str,a)),check=True)
if '--rebuild' in sys.argv:
 subprocess.run([sys.executable,'tools/build.py','--release'],cwd=R/'source/WhateverXe',check=True)
 subprocess.run([sys.executable,'tools/test.py'],cwd=R/'source/WhateverXe',check=True)
 shutil.rmtree(R/'artifacts/WhateverXe.kext',ignore_errors=True);(R/'artifacts').mkdir(exist_ok=True);shutil.copytree(R/'source/WhateverXe/build/WhateverXe.kext',R/'artifacts/WhateverXe.kext')
(R/'target').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory() as t:
 t=Path(t);expanded=t/'expanded';run('pkgutil','--expand',R/'vendor/WhateverXe-Graphics-0.4.0.pkg',expanded)
 root=t/'root';payload=root/'Library/Application Support/WhateverXe-Port';payload.mkdir(parents=True)
 shutil.copytree(R/'artifacts/WhateverXe.kext',payload/'WhateverXe.kext')
 shutil.copy(R/'README.md',payload/'README.md');shutil.copy(R/'tools/install-efi.sh',payload/'install-efi.sh');shutil.copy(R/'tools/controlcenter-workaround.sh',payload/'controlcenter-workaround.sh')
 component=t/'Port.pkg';cp=t/'components.plist';run('pkgbuild','--analyze','--root',root,cp)
 entries=plistlib.loads(cp.read_bytes())
 for entry in entries:entry['BundleIsRelocatable']=False
 cp.write_bytes(plistlib.dumps(entries))
 run('pkgbuild','--root',root,'--component-plist',cp,'--identifier','local.whateverxe.port.24H232','--version','0.4.0.1','--install-location','/',component)
 run('pkgutil','--expand',component,expanded/'Port.pkg')
 tree=ET.parse(expanded/'Distribution');d=tree.getroot();d.find('title').text='WhateverXe Graphics 0.4.0 — Local 24H232 Port'
 allowed=d.find('./volume-check/allowed-os-versions');allowed.clear();ET.SubElement(allowed,'os-version',min='15.8.2',before='15.8.3')
 script=ET.SubElement(d,'script');script.text="function localBuildCheck() { if (system.version.ProductBuildVersion != '24H232') { my.result.type='Fatal'; my.result.message='This local port is qualified only for macOS build 24H232.'; return false; } return true; }"
 ET.SubElement(d,'installation-check',script='localBuildCheck()')
 ET.SubElement(d.find('choices-outline'),'line',choice='LocalPort')
 ch=ET.SubElement(d,'choice',id='LocalPort',title='Local EFI plugin and instructions',visible='false',selected='true');ET.SubElement(ch,'pkg-ref',id='local.whateverxe.port.24H232')
 ref=ET.SubElement(d,'pkg-ref',id='local.whateverxe.port.24H232',version='0.4.0.1');ref.text='#Port.pkg'
 tree.write(expanded/'Distribution',encoding='utf-8',xml_declaration=True)
 out=R/'target/WhateverXe-Graphics-0.4.0-Port-24H232.pkg';run('pkgutil','--flatten',expanded,out)
 (R/'target/SHA256SUMS').write_text(hashlib.sha256(out.read_bytes()).hexdigest()+'  '+out.name+'\n')
 print(out)
