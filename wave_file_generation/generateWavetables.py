## This file generates headers with lookup tables for various waveforms
## Add your own.

import math

def phaseSteps(maxPhase, length=256):
    steps = range(0, length) 
    steps = [1.0*x/length * 2.0*math.pi * (maxPhase/360.0) for x in steps]
    return(steps)

def scaleAndRound(data, scale=255, signedInt=True):
    data = [0.0+x-min(data) for x in data]
    data = [1.0*x/max(data)*scale for x in data]
    data = [int(round(x)) for x in data]
    if signedInt:
        data = [x-(scale+1)/2 for x in data]
    return(data)

def makeSin(maxPhase, length=256):
    sinus = [math.sin(x) for x in phaseSteps(maxPhase, length)]
    return(sinus)

def prettyPrint(data, perLine = 8):
    outString = ""
    for i in range(len(data) / perLine):
        strings = [str(x) for x in data[perLine*i:(perLine*i+perLine)]]
        outString += "\t" + ", ".join(strings) + ",\n"
    outString = outString[:-2] + "\n" # drop the final comma
    return(outString)

def writeHeader(fileName, dataName, data, signedInt=True):
    outfile = open(fileName, "a")
    if signedInt:
        outfile.write("const int8_t {}[{:d}] PROGMEM = {{ \n".format(dataName, len(data)))
    else:
        outfile.write("const uint8_t {}[{:d}] PROGMEM = {{ \n".format(dataName, len(data)))
    outfile.write(prettyPrint(data))
    outfile.write("};\n")
    outfile.close()

def bandlimitedSawtooth(maxPhase, numberPartials, length=256):
    wave = [0]*length
    sign = 1.0
    for k in range(1, numberPartials+1):
        phases = phaseSteps(maxPhase*k, length)
        for i in range(length):
            wave[i] += sign * math.sin(phases[i]) / k
        sign = sign * -1
    return(wave)

def bandlimitedSquare(maxPhase, numberPartials, length=256):
    wave = [0]*length
    for k in range(1, numberPartials*2, 2):
        phases = phaseSteps(maxPhase*k, length)
        for i in range(length):
            wave[i] +=  math.sin(phases[i]) / k
    return(wave)

def bandlimitedTriangle(maxPhase, numberPartials, length=256):
    wave = [0]*length
    sign = 1.0
    for k in range(1, numberPartials*2, 2):
        phases = phaseSteps(maxPhase*k, length)
        for i in range(length):
            wave[i] += sign * math.sin(phases[i]) / k**2
        sign = sign * -1
    return(wave)



if __name__ == "__main__":
    
    ## Full-waves, full 256 bytes, 0-255 range
    writeHeader("dds_waveforms.h", 'Sine', scaleAndRound(makeSin(360)))

    triangleWave = range(0,64)
    triangleWave.extend(range(64, -64, -1))
    triangleWave.extend(range(-64, 0, 1))
    triangleWave = scaleAndRound(triangleWave)
    writeHeader("dds_waveforms.h", 'Triangle', triangleWave)

    for numberFrequencies in [3,7,15]:
        saw = scaleAndRound(bandlimitedSawtooth(360, numberFrequencies))
        writeHeader("dds_waveforms.h".format(numberFrequencies), 
                    'Saw{}'.format(numberFrequencies), saw)
        tri = scaleAndRound(bandlimitedTriangle(360, numberFrequencies))
        writeHeader("dds_waveforms.h".format(numberFrequencies), 
                    'Tri{}'.format(numberFrequencies), tri)
        square = scaleAndRound(bandlimitedSquare(360, numberFrequencies))
        writeHeader("dds_waveforms.h".format(numberFrequencies), 
                    'Square{}'.format(numberFrequencies), square)

    
