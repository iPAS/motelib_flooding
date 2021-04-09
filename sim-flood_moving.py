from motesim import Simulator, Mote, Gateway
from motesim.SimGateway import Gateway as GW

import random
from time import sleep

from threading import Thread
sim_port = ''
sequence = 0
gw       = None


###################################
class MyGateway(Gateway):
    def debug(self, msg):
        if msg.find('Gateway starts listening on port') >= 0:
            txt = msg.split(' ')
            global sim_port
            sim_port = txt[5]
            global gw
#            gw = GW('localhost:' + sim_port) # Could't connect from here. Why ?

        Gateway.debug(self, msg)


###################################
class MyMote(Mote):
    def debug(self, msg):
        if msg.find('Change parent from') >= 0:
            txt = msg.split(' ')
            self.old_parent = int(txt[3])
            self.new_parent = int(txt[5])

            if self.old_parent != 0xFFFF:
                sim.scene.dellink(self.id, self.old_parent, 'my_style')
            sim.scene.addlink(self.id, self.new_parent, 'my_style')

        Mote.debug(self, msg)

    def shutdown(self):
        try:
            sim.scene.dellink(self.id, self.new_parent, 'my_style')
        except:
            pass
        Mote.shutdown(self)


###################################
def gw_send():
    global sequence, gw
    sequence += 1
    gw.send(0xFFFF, 1, [sequence & 0xFF, (sequence >> 8) & 0xFF, 0])

def nodes_up(nodes):
    for n in nodes:
        sim.nodes[n].boot()

def nodes_down(nodes):
    for n in nodes:
        sim.nodes[n].shutdown()


###################################
class AliveNodes(Thread):
    def __init__(self):
        Thread.__init__(self)
    def run(self):
        for x in xrange(5):
            for y in xrange(5): # Boot up all nodes, except '0'. It's gateway.
                sim.nodes[x*5 + y + 1].boot()
                sleep(2)

def myScript():
    global gw, sim_port
    gw = GW('localhost:' + sim_port)
#    sequence = 0
#    gw.send(0xFFFF, 0x11, [0, 0, 0]) # Reset msgSeq when gateway was born

    ###############
    th = AliveNodes()
    th.start()

    sim.nodes[25].boot()
    speed = 0.1
    pos = {'x':60, 'y':60}

    for j in xrange(4): # Move & route making
        for i in xrange(45):
            pos['x'] = pos['x'] + 2
            pos['y'] = pos['y'] + 2
            sim.nodes[25].move(pos['x'], pos['y'])
            sleep(speed)

        gw_send()
        sleep(10)

    th.join()

    ###############
    print '--- Down middle nodes ---'
    nodes_down([7,8,12,13])
    sleep(.5); gw_send(); sleep(5)

    print '--- Up middle nodes ---'
    nodes_up([7,8,12,13])
    sleep(.5); gw_send(); sleep(5)

    print '--- Kill nodes ---'
    nodes_down([3,7,11,4,8,12,16])
    sleep(.5); gw_send(); sleep(5)

    print '--- Birth nodes ---'
    nodes_up([3,7,11,4,8,12,16])
    sequence = 0
    sleep(.5); gw_send(); sleep(5)


###################################
if __name__ == '__main__':
    sim = Simulator()
    sim.addNode( MyGateway(), (50,50) )

    for x in range(5):
        for y in range(5):
            pos = (100 + x*75 + random.randint(0,20),
                100 + y*75 + random.randint(0,20))
            sim.addNode( MyMote('build/sim/flood.elf'), pos)

    sim.scene.linestyle("my_style", color=[0,0,0] , dash=(1,2,2,2), arrow='head')
    raw_input('Press ENTER key to start...'); sleep(3)

    sim.run(bootMotes=False, script=myScript)
