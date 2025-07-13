import sys
from tm1637 import *

# GPIO pin numbers
CLK = 24
DIO = 23

display = TM1637(CLK, DIO)

if sys.argv[1] == "clean":
    display.cleanup()
else:
    display.show(sys.argv[1])
