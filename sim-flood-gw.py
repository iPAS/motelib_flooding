from sys import argv
from motesim.SimGateway import Gateway

###################################
class MyGateway(Gateway):
    pass

###################################

if __name__ == '__main__':
    sequence = 1

    if len(argv) > 1:
        gw = MyGateway(argv[1])
        if len(argv) > 2:
            sequence = int(argv[2])            
    else:    
        gw = MyGateway('localhost:30000')
        
    gw.send(0xFFFF, 1, [sequence & 0xFF, (sequence >> 8) & 0xFF, 0xFF])

