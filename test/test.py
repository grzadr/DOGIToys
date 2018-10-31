#!/usr/bin/python3

from sys import path

path.append(".")

from pyDOGIToys import DOGI

print(path)

db = DOGI();
db.open("temp.db", True)
db.close(True);


