import os
import sys
import shutil

filename = sys.argv[1]

basename, ext = os.path.splitext(filename)
newname = basename + '-out' + ext

with open(filename) as read:
    read.read(72)
    
    with open(filename, 'w') as write:
        shutil.copyfileobj(read, write)