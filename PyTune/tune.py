import winsound
import wave
import struct
import sys
import re

NNAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
SRATE=48000
SLEW=0.005
SEMITONE=1.05946309436
AMIDI=69
AHERTZ=440

def bytetonote(byte):
	return NNAMES[byte%12] + str(byte/12)

# simple triangle wave synth
class synth:
	def __init__(self):
		self.acc=0
		self.inc=0
		self.gain=0
		self.on=0
		
	def getframe(self):
		self.acc += self.inc
		if(self.acc>=1 or self.acc<=-1):
			self.inc *= -1
		if(self.on and self.gain < 1):
			self.gain += SLEW
		elif(not self.on and self.gain > 0):
			self.gain -= SLEW
		return int(round(self.acc * self.gain * 4096))
		
	def setnote(self, note):
		hertz = AHERTZ * (SEMITONE ** (note-AMIDI))
		print bytetonote(note) + " => " + str(hertz) + "Hz"
		if(self.inc>=0):
			self.inc=(hertz*2)/SRATE
		else:
			self.inc=(hertz*-2)/SRATE
	
	def setstate(self, state):
		self.on=state
		
# arduboyplaytune pseudo-emulator https://github.com/Arduboy/ArduboyPlaytune
def playtune(data, outfile):
	idx=0
	voices=[None] * 16
	exit=0;
	while not exit and idx<len(data):
		if (data[idx] & 0xf0) == 0x90:
			print(str(idx) + 'on')
			vidx = data[idx] & 0x0f
			idx += 1
			if not voices[vidx]:
				voices[vidx] = synth()
			voices[vidx].setnote(data[idx])
			voices[vidx].setstate(1)
			idx += 1
		elif (data[idx] & 0xf0) == 0x80:
			print(str(idx) + 'off')
			vidx = data[idx] & 0x0f
			idx += 1
			if not voices[vidx]:
				voices[vidx] = synth()
			voices[vidx].setstate(0)
		elif (data[idx] & 0x80) == 0x00:
			print(str(idx) + 'wait')
			ms = (data[idx] & 0x7f) * 256
			idx += 1
			ms += data[idx]
			idx += 1
			frames = int(round(ms * SRATE / 1000))
			for i in range(0, frames):
				samp=0
				for j in range(0, 16):
					if voices[j]:
						samp += voices[j].getframe()
				frame = struct.pack('h', samp)
				outfile.writeframes(frame)
		elif data[idx] == 0xe0 or data[idx] == 0xf0:
			print(str(idx) + 'end')
			exit=1
			idx += 1
			

infile = open("tune.txt", 'r')
hexcmds = re.split('\s+', infile.read())
infile.close()
tune = []
for i in range(0, len(hexcmds)):
	tune.append(int(hexcmds[i], 16))
			
outfile = wave.open("tune.wav", 'w')
outfile.setnchannels(1)
outfile.setframerate(SRATE)
outfile.setsampwidth(2)

playtune(tune, outfile)

outfile.close()
winsound.PlaySound("tune.wav", winsound.SND_FILENAME)