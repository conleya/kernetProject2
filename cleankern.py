import sys
inf = open(sys.argv[1], "r")
outf = open(sys.argv[2], "w+")


line = inf.readline()
while line:
    if(line.find('KERNET src:') != -1) 
        outf.write(line)
    line = inf.readline()

inf.close()
outf.close()