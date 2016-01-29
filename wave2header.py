## Start out with a .WAV file, 8-bit, mono, 8K samples/sec
## End up with a C header file for the samples

import wave
from struct import unpack

def unpackMono(waveFile):
    w = wave.Wave_read(waveFile)
    data = []
    for i in range(w.getnframes()):
        data.append(unpack("B", w.readframes(1))[0])
    return(data)

def createHeader(basename, data):
    outfile = open("WAV_" + basename + ".h", "w")
    outfile.write('const uint8_t WAV_%s[%d] PROGMEM = {\n' % (basename, len(data)))
    i = 0
    while (i < (len(data) - 10)):
        thisline = "\t" + ", ".join([str(x) for x in data[i:i+10]]) + ",\n"
        i = i + 10
        outfile.write(thisline)
    outfile.write("\t" + ", ".join([str(x) for x in data[i:]]))
    outfile.write('};\n')
    outfile.close()

if __name__ == "__main__":

   ## This is how I converted / edited the file
   ## You could do this with whatever audio tools you've got
   ## sox dog2.wav -c 1 -b 8 -r 8000 doggy_8000.wav trim 0.725 0.38
   data = unpackMono("doggy_8000.wav")
   createHeader("bark", data)   

