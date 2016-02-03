# scaleGenerator.py
# Generates accumulator values for a DDS routine, more or less in tune.
# 

import math

SCALE = ['C', 'Cx', 'D', 'Dx', 'E', 'F', 'Fx', 'G', 'Gx', 'A', 'Ax', 'B']

def calculateOctave(baseLength):
    wavelengths = [baseLength * math.exp(x*math.log(2)/12) for x in range(0, 12)]
    wavelengths = [int(round(x)) for x in wavelengths]
    return( zip(SCALE, wavelengths) )

def makePitches(basePitch, numOctaves):
    pitchList = []
    for octave in range(0, numOctaves):
        for note, wavelength in calculateOctave(basePitch * 2**octave):
            if wavelength < 65500:
                noteString = note + str(octave)
                pitchList.append((noteString,wavelength))
    return(pitchList)            
    
def makeDefines(basePitch, numOctaves):
    pitchList = makePitches(basePitch, numOctaves)
    defineString = "// Scale in the key of {} \n".format(basePitch)
    defineString += "// Automatically generated by scaleGenerator.py \n\n"
    for (note, length) in pitchList:
        defineString += "#define  {:<5}{:>6}\n".format(note, length)
    return(defineString)

if __name__ == "__main__":
    
    ## Change these if you like
    BASEPITCH = 512 
    OCTAVES   = 4
    OUTFILE   = "scale16.h"
    
    ## Write it out to a file
    out = open(OUTFILE, "w")
    out.write(makeDefines(BASEPITCH, OCTAVES))
    out.close()

    


