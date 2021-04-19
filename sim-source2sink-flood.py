from motesim import Simulator, Mote, Gateway
from motesim.SimGateway import Gateway as SimGateway
from motesim.MoteSim import strToList, listToStr

import random
from time import sleep

from threading import Thread


sim_port = ''
sequence = 0
gw       = None
nodes    = []


###################################
class MySimGateway(SimGateway):

    ###################
    def receive(self, source, msgType, msg):
        '''
        Gets called when a message is received from radio.  This method
        should be overridden to perform appropriate actions.

        @param source
        Contains the address of the message's source

        @param msgType
        Contains the 8-bit type value

        @param msg
        Points to a string containing the message
        '''
        print '<<< MyGateway received from %x: type=%d, msg=%s >>>' % (source, msgType, strToList(msg))


###################################
class MyGateway(Gateway):

    ###################
    def debug(self, msg):
        if msg.find('Gateway starts listening on port') >= 0:
            global sim_port
            sim_port = msg.split(' ')[5]  # Server port
        Gateway.debug(self, msg)

    ###################
    def receiveRadioMsg(self, msg, rssi):
        self.debug('Gateway received message with rssi %d: %s' % (rssi, msg))
        Gateway.receiveRadioMsg(self, msg, rssi)


###################################
class MyMote(Mote):

    ###################
    def debug(self, msg):
        txt = msg.split(' ')

        def update_label(idno, seqno, besthop):
            sim.scene.nodelabel(self.id, '%d:%d\n%3d' % (idno, seqno, besthop))

        if msg.find('Change parent from') >= 0:
            self.old_parent = int(txt[3])
            self.new_parent = int(txt[5])
            if self.old_parent != 0xFFFF:
                sim.scene.dellink(self.id, self.old_parent, 'my_style')
            sim.scene.addlink(self.id, self.new_parent, 'my_style')

        elif msg.find('Change sequence from') >= 0:
            self.seqno = int(txt[5])
            update_label(self.id, self.seqno, self.besthop)

        elif msg.find('NEW best hop') >= 0:
            self.besthop = int(txt[3])
            update_label(self.id, self.seqno, self.besthop)

        else:
            Mote.debug(self, msg)

    ###################
    def boot(self):
        self.seqno   = 0
        self.besthop = 255
        sim.scene.nodelabel(self.id, '%d:%d\n ' % (self.id, 0))
        Mote.boot(self)

    ###################
    def shutdown(self):
        # Delete fan-out
        try:
            sim.scene.dellink(self.id, self.new_parent, 'my_style')
        except: 
            pass

        # Delete fan-in
        # for n in sim.nodes:
        #     if n.id > 0:
        #         try:
        #             sim.scene.dellink(n.id, self.id, 'my_style')
        #         except:
        #             pass
        Mote.shutdown(self)


###################################
def gw_send():
    global sequence, gw
    sequence += 1
    sim.scene.nodelabel(0, '%d:%d\n ' % (0, sequence))

    msg_type = 1
    msg = [sequence & 0xFF, (sequence >> 8) & 0xFF, 5]
    gw.send(dest=0xFFFF, msgType=msg_type, msg=msg)  # Broadcasting 'sequence' all 0xFFFF with FLOOD_MSG_TYPE


def nodes_up(nodes):
    for n in nodes:
        sim.nodes[n].boot()


def nodes_down(nodes):
    for n in nodes:
        sim.nodes[n].shutdown()


def set_sequence(v):
    global sequence
    sequence = v
    sim.scene.nodelabel(0, '%d:%d\n ' % (0, sequence))


def reset_sequence():
    set_sequence(0)


###################################
def script():
    print '<<< Script gets started >>>'

    global gw, sim_port
    while sim_port == '': sleep(1)  # Wait for MyGateway.debug() ...
    gw = MySimGateway('localhost:'+sim_port)

    for n in range(len(nodes)+1):
        sim.scene.nodescale(n, 1.6)
        sim.scene.nodelabel(n, '%d:%d\n ' % (n, 0))

    ###############
    # class WakeupNodesThread(Thread):

    #     ###################
    #     def __init__(self):
    #         Thread.__init__(self)

    #     ###################
    #     def run(self):
    #         for n in range(len(nodes)):
    #             sim.nodes[n].boot()
    #             sleep(1)

    # th = WakeupNodesThread()
    # th.start()

    # sim.nodes[25].boot()
    # speed = 0.1
    # pos = {'x':60, 'y':60}

    # for j in xrange(4): # Move & route making
    #     for i in xrange(45):
    #         pos['x'] = pos['x'] + 2
    #         pos['y'] = pos['y'] + 2
    #         sim.nodes[25].move(pos['x'], pos['y'])
    #         sleep(speed)

    #     gw_send()
    #     sleep(10)

    # th.join()

    ###############
    def reports():
        sleep(.5); gw_send(); sleep(3)
        # sleep(.5); gw_send(); sleep(3)
        # sleep(.5); gw_send(); sleep(3)

    ###############
    print '<<< Flood routing protocol testing : node_label ==> (idno, seqno, besthop) >>>'

    print '--- Up all nodes ---'
    nodes_up(range(len(nodes)))
    
    
    reports()

    # print '--- Down middle nodes ---'
    # nodes_down([7,8,12,13])
    # reports()

    # print '--- Up middle nodes ---'
    # nodes_up([7,8,12,13])
    # reports()

    # print '--- Down some nodes ---'
    # nodes_down([1,2,6,3,7,11,4,8,12,16])
    # reports()

    # print '--- Up some nodes ---'
    # nodes_up([1,2,6])
    # reports()

    # print '--- Emulate Gateway dead by reset_sequence() ___'
    # reset_sequence()  # Emulate Gateway dead
    # reports()

    # print '--- Up all ---'
    # nodes_up([3,7,11,4,8,12,16])
    # reports()

    # print '--- Emulate Gateway dead by reset_sequence() ___'
    # reset_sequence()  # Emulate Gateway dead
    # reports()

    print '--- --- ---'


###################################
if __name__ == '__main__':
    sim = Simulator()

    for x in range(3):
        for y in range(3):
            pos = (100 + x*75 + random.randint(0,20),
                   100 + y*75 + random.randint(0,20))
            node = MyMote('build/sim/flood.elf', txRange=100, panid=0x22, channel=0x11)
            sim.addNode(node, pos)
            nodes.append(node)

    sim.addNode(MyGateway(), (300,300))

    sim.scene.linestyle("my_style", color=[0,0,0] , dash=(1,2,2,2), arrow='head')
    sleep(1)

    sim.run(bootMotes=False, script=script)
