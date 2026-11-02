import struct,re,pathlib
r=pathlib.Path(__file__).resolve().parents[1]/'source/WhateverXe'
d=pathlib.Path('/System/Library/Kernels/kernel').read_bytes();pos=32;segs=[]
for _ in range(struct.unpack_from('<I',d,16)[0]):
 c,n=struct.unpack_from('<II',d,pos)
 if c==0x19:segs.append(struct.unpack_from('<4Q',d,pos+24))
 if c==2:so,ns,st,ss=struct.unpack_from('<4I',d,pos+8)
 pos+=n
symbols={}
for i in range(ns):
 off=so+i*16;ix=struct.unpack_from('<I',d,off)[0];v=struct.unpack_from('<Q',d,off+8)[0];name=d[st+ix:d.index(b'\0',st+ix)].decode(errors='replace');symbols[name]=v

def read(v,n):
 for base,vs,off,fs in segs:
  if base<=v and v+n<=base+fs:return d[off+v-base:off+v-base+n]
 raise ValueError(hex(v))
s=(r/'WhateverXe/kern_xe.cpp').read_text();b=s[s.index('static void tglPrepareSequoiaMMIOUC'):s.index('static void tglPrepareSonomaMMIOUC')]
a={'fn':symbols['_pmap_enter_options'],'protect':symbols['_IOProtectCacheMode']};count=0
for base,offset,name in re.findall(r'!memcmp\(reinterpret_cast<void \*>\((fn|protect)(\+0x[0-9A-Fa-f]+)?\),([A-Za-z]+),sizeof',b):
 vals=re.search(r'const uint8_t '+name+r'\[\] \{([^}]+)\}',b)[1];expected=bytes(int(x.strip(),0) for x in vals.split(','));assert read(a[base]+(int(offset[1:],16) if offset else 0),len(expected))==expected,name;count+=1
assert count==9
call=a['protect']+0xca;assert read(call,1)==b'\xe8';assert call+5+struct.unpack('<i',read(call+1,4))[0]==a['fn']
print('PASS: all 9 runtime byte guards and call destination match installed kernel')
