"""Synthesize shark-boss SFX (original; no third-party audio). Requires numpy/scipy.
Writes stereo 44.1kHz WAVs to Data/Sound/SE/."""
import numpy as np, scipy.signal as ss
from scipy.io import wavfile
from pathlib import Path
SR=44100; rng=np.random.default_rng(42)
OUT=Path(__file__).resolve().parents[2]/"Data"/"Sound"/"SE"; OUT.mkdir(parents=True, exist_ok=True)
t=lambda d: np.arange(int(SR*d))/SR
def lp(x,f,o=4): return ss.sosfilt(ss.butter(o,f,"low",fs=SR,output="sos"),x)
def hp(x,f,o=2): return ss.sosfilt(ss.butter(o,f,"high",fs=SR,output="sos"),x)
def bp(x,a,b,o=2): return ss.sosfilt(ss.butter(o,[a,b],"band",fs=SR,output="sos"),x)
def env(n,a,curve=4):
    e=np.ones(n); na=max(1,int(a*SR)); e[:na]=np.linspace(0,1,na); e[na:]=np.exp(-curve*np.linspace(0,1,n-na)); return e
def sweep(f0,f1,d):
    f=np.geomspace(f0,f1,int(SR*d)); return np.sin(2*np.pi*np.cumsum(f)/SR)
def noise(d): return rng.standard_normal(int(SR*d))
def bubble(f,d=.08):
    tt=t(d); return np.sin(2*np.pi*np.cumsum(f*(1+tt*30))/SR)*np.exp(-tt*45)
def place(buf,x,at):
    i=int(at*SR); n=min(len(x),len(buf)-i)
    if n>0: buf[i:i+n]+=x[:n]
    return buf
def verb(x,d=.5,mix=.25):
    ir=lp(noise(d)*np.exp(-np.linspace(0,7,int(SR*d))),3000)
    w=ss.fftconvolve(x,ir)[:len(x)+int(SR*d)]; w/=np.max(np.abs(w))+1e-9
    y=np.pad(x,(0,len(w)-len(x))); return y*(1-mix)+w*mix*np.max(np.abs(x))
def finish(name,x,fade_in=.003,fade_out=.03,uw=True,peak_db=-3):
    if uw: x=lp(x,6500,2)
    x=x-np.mean(x); a=np.abs(x); idx=np.where(a>a.max()*10**(-50/20))[0]; x=x[idx[0]:idx[-1]+1]
    fi=min(int(fade_in*SR),len(x)//4); fo=min(int(fade_out*SR),len(x)//4)
    if fi: x[:fi]*=np.linspace(0,1,fi)
    if fo: x[-fo:]*=np.linspace(1,0,fo)
    x=x/(np.max(np.abs(x))+1e-9)*10**(peak_db/20)
    wavfile.write(str(OUT/(name+".wav")),SR,(np.stack([x,x],1)*32767).astype(np.int16))
    print(f"{name:28s} {len(x)/SR:.2f}s")
d=.55; tt=t(d); x=np.zeros(int(SR*.62))
x=place(x,sweep(180,920,d)*env(len(tt),.02,1.8)*.55,0)
x=place(x,bp(noise(d),800,3200)*env(len(tt),.01,3)*.35,0)
x=place(x,np.sin(2*np.pi*55*tt)*env(len(tt),.05,1.2)*.4,0)
x=place(x,sweep(1400,2200,.08)*env(int(SR*.08),.001,10)*.45,.42)
finish("sfx_shark_telegraph",verb(x,.35,.18),fade_out=.06)
d=.28; x=bp(noise(d),200,4500)*np.sin(np.pi*np.linspace(0,1,int(SR*d)))**1.5
x+=sweep(900,220,d)*env(int(SR*d),.001,6)*.7
x=place(x,hp(noise(.06),3000)*env(int(SR*.06),.001,25)*.5,0)
finish("sfx_shark_dash",verb(x,.25,.15),fade_out=.04)
x=np.zeros(int(SR*.85))
x=place(x,sweep(120,35,.35)*env(int(SR*.35),.01,4)*.9,0)
x=place(x,lp(noise(.45),800)*env(int(SR*.45),.02,3)*.7,.08)
x=place(x,bp(noise(.12),400,2500)*env(int(SR*.12),.001,12)*.6,.05)
for _ in range(12): x=place(x,bubble(rng.uniform(200,700),.1)*.25,.1+rng.uniform(0,.45))
finish("sfx_shark_dive",verb(x,.4,.2),fade_out=.08)
x=np.zeros(int(SR*.7))
x=place(x,sweep(80,28,.22)*env(int(SR*.22),.001,5),0)
x=place(x,hp(noise(.08),1500)*env(int(SR*.08),.001,18)*.8,0)
x=place(x,bp(noise(.18),300,2800)*env(int(SR*.18),.002,8)*.7,.02)
x=place(x,sweep(700,140,.12)*env(int(SR*.12),.001,9)*.5,.04)
for _ in range(16): x=place(x,bubble(rng.uniform(350,1200),.07)*.3,.05+rng.uniform(0,.35))
finish("sfx_shark_erupt",verb(x,.45,.22),fade_out=.06)
d=.35; x=sweep(380,90,d)*env(int(SR*d),.001,7)*.7
x+=bp(noise(d),600,3500)*env(int(SR*d),.001,14)*.55
x=place(x,sweep(1600,400,.08)*env(int(SR*.08),.001,15)*.4,0)
finish("sfx_shark_grab",verb(x,.3,.18),fade_out=.04)
d=.4; x=bp(noise(d),150,3500)*np.sin(np.pi*np.linspace(0,1,int(SR*d)))**1.2*.7
x+=sweep(500,90,d)*env(int(SR*d),.005,4)*.65
x=place(x,hp(noise(.05),2500)*env(int(SR*.05),.001,20)*.45,0)
finish("sfx_shark_throw",verb(x,.35,.2),fade_out=.05)
d=.45; tt=t(d)
x=bp(noise(d),250,2200)*(0.5+0.5*np.sin(2*np.pi*3.5*tt))*np.sin(np.pi*np.linspace(0,1,len(tt)))
x+=sweep(420,180,d)*env(len(tt),.02,2.5)*.4
finish("sfx_shark_strafe",verb(x,.3,.15),fade_out=.08,peak_db=-5)
d=1.8; tt=t(d)
x=lp(noise(d),700)*(0.55+0.45*np.sin(2*np.pi*1.8*tt))
x+=0.35*np.sin(2*np.pi*(48*tt+2.5*np.sin(2*np.pi*.7*tt)))
x+=bp(noise(d),1200,3200)*.18*(0.5+0.5*np.sin(2*np.pi*4*tt))
for _ in range(40): x=place(x,bubble(rng.uniform(280,900),.09)*rng.uniform(.12,.28),rng.uniform(0,d-.1))
xf=int(SR*.12); x[-xf:]=x[-xf:]*np.linspace(1,0,xf)+x[:xf]*np.linspace(0,1,xf); x=x[xf:]
x=lp(x,6500,2); x=x/(np.max(np.abs(x))+1e-9)*10**(-4/20)
wavfile.write(str(OUT/"sfx_shark_vortex_loop.wav"),SR,(np.stack([x,x],1)*32767).astype(np.int16))
print(f"{"sfx_shark_vortex_loop":28s} {len(x)/SR:.2f}s (loop)")
d=1.15; tt=t(d); ph=np.cumsum(np.geomspace(90,55,len(tt)))/SR
x=lp(2*(ph%1)-1,400)*.45*env(len(tt),.04,1.6)
x+=lp(noise(d),500)*env(len(tt),.03,1.8)*.55
x+=bp(noise(d),800,2500)*env(len(tt),.02,2.5)*.35
x=place(x,sweep(1100,400,.2)*env(int(SR*.2),.005,5)*.45,.15)
x=place(x,sweep(900,280,.25)*env(int(SR*.25),.005,4)*.4,.45)
finish("sfx_shark_frenzy",verb(x,.55,.28),fade_out=.1)
d=.75; tt=t(d); x=np.zeros(int(SR*.9))
for k,f0 in enumerate([1900,2400,2900]):
    s=np.sin(2*np.pi*f0*tt+4*np.sin(2*np.pi*9*tt))*np.exp(-tt*2.8)*.28
    x=place(x,s,k*.05)
x=place(x,sweep(650,200,.12)*env(int(SR*.12),.001,7),0)
finish("sfx_shark_stun",verb(x,.45,.28),uw=False,fade_out=.08)
print("Wrote", len(list(OUT.glob("sfx_shark_*.wav"))), "files to", OUT)
