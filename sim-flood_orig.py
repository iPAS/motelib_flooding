from motesim import Simulator, Mote, Gateway

import random
from time import sleep

from threading import Thread

###################################
class MyMote(Mote):
    def debug(self, msg):
        txt = msg.split(' ')
        old_parent = int(txt[3])
        new_parent = int(txt[5])
        
        #print old_parent, new_parent
        
        if old_parent != 0xFFFF:        
            sim.scene.dellink(self.id, old_parent, "my_style")
            
        sim.scene.addlink(self.id, new_parent, "my_style")
        
###################################
class MoveMobile(Thread):
    def __init__(self):
        Thread.__init__(self)
        
    def run(self):
        sim.nodes[25].boot() 
        speed = 0.1
        wait = 10

        pos = {'x':60, 'y':60}
        
        
        for j in range(4):
            for i in range(45):
                pos['x'] = pos['x'] + 2
                pos['y'] = pos['y'] + 2
                sim.nodes[25].move(pos['x'], pos['y'])
                sleep(speed)  

            sleep(wait)
         
            
    
class AliveNodes(Thread):
    def __init__(self):
        Thread.__init__(self)
 
    def run(self):
        for x in xrange(5):
            for y in xrange(5):
                sim.nodes[x*5 + y + 1].boot()        
#                sim.nodes[x*5 + y].boot()        
                sleep(2)

def myScript():
    th = MoveMobile()
    th.start()
    
    th = AliveNodes()
    th.start()
    
###################################
sim = Simulator()
sim.addNode( Gateway((50,50)) )

for x in range(5):
    for y in range(5):
        pos = (100+x*75+random.randint(0,20), 100+y*75+random.randint(0,20))
        sim.addNode( MyMote('build/sim/flood.elf', pos) )

sim.scene.linestyle("my_style", color=[0,0,0] , dash=(1,2,2,2), arrow='head')

sim.run(bootMotes=False, extScript=myScript)

