from motesim import Simulator, Mote, Gateway
from motesim.SimGateway import BROADCAST_ADDR, Gateway as SimGateway
from motesim.MoteSim import strToList, listToStr

import random
from time import sleep

from threading import Thread


FLOOD_MSG_TYPE  = 0x01
REPORT_MSG_TYPE = 0x22

sim_port = ''
gw       = None
nodes    = []
firmware = 'build/sim/test_comm.elf'


###################################
class MySimGateway(SimGateway):

    ###################
    def __init__(self, device='localhost:30000', autoListen=True, sim=None):
        self.msgSeqNo = 0
        SimGateway.__init__(self, device=device, autoListen=autoListen, sim=sim)

    ###################
    def receive(self, source, msgType, msg):
        self.debug('MySimGateway received from=%x type=%d: %s' % (source, msgType, strToList(msg)))

    ###################
    def send_to_nodeid(self, node_id, msg):
        self.msgSeqNo += 1
        sim.scene.nodelabel(self.nodeId, '%d:%d\n' % (self.nodeId, self.msgSeqNo))

        # Based on flood message structure defined in flood_routing.h,
        # typedef struct flood_msg
        # {
        #     uint8_t floodSeqNo;
        #     uint8_t hopCount;
        # } FloodMsg;
        if isinstance(msg, str):
            msg = strToList(msg)
        msg = [self.msgSeqNo, 1] + msg
        SimGateway.send(self, dest=BROADCAST_ADDR, msgType=FLOOD_MSG_TYPE, msg=msg)


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
        self.debug('MyGateway received with rssi=%d: %s' % (rssi, msg))
        Gateway.receiveRadioMsg(self, msg, rssi)


###################################
class MyMote(Mote):

    ###################
    def debug(self, msg):
        txt = msg.split(' ')

        if msg.find('Change parent from') >= 0:  # In set_besthop(..)
            self.old_parent = int(txt[3])
            self.new_parent = int(txt[5])
            if self.old_parent != 0xFFFF:
                sim.scene.dellink(self.id, self.old_parent, 'my_style')
            sim.scene.addlink(self.id, self.new_parent, 'my_style')

        elif msg.find('Change seqNo from') >= 0:  # In on_receive(..)
            self.seqno = int(txt[6])
            sim.scene.nodelabel(self.id, '%d:%d\n%3d' % (self.id, self.seqno, self.besthop))

        elif msg.find('New best hop') >= 0:  # In set_besthop(..)
            self.besthop = int(txt[3])
            sim.scene.nodelabel(self.id, '%d:%d\n%3d' % (self.id, self.seqno, self.besthop))

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
    # Beautify the network graph
    for n in range(len(nodes)+1):  # Plus one for the gw
        sim.scene.nodescale(n, 2.)
        sim.scene.nodelabel(n, '%d:%d\n ' % (n, 0))  

    # Get started!
    print '<<< Script gets started >>>'

    while sim_port == '':
        sleep(1)  # Wait for MyGateway.debug() ...

    global gw
    gw = MySimGateway('localhost:'+sim_port, sim=sim)


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
    print '<<<--- Flood routing protocol testing : node_label ==> (idno, seqno, besthop) --->>>'

    print '<<<--- Up all nodes --->>>'
    nodes_up(range(len(nodes)))

    gw.send_to_nodeid(node_id=0, msg=[0x55, 0x55])
    sleep(3)
    gw.send_to_nodeid(node_id=0, msg=[0x55, 0x55])

    # print '--- Down middle nodes ---'
    # nodes_down([7,8,12,13])
    # print '--- Up middle nodes ---'
    # nodes_up([7,8,12,13])
    # print '--- Down some nodes ---'
    # nodes_down([1,2,6,3,7,11,4,8,12,16])
    # print '--- Up some nodes ---'
    # nodes_up([1,2,6])
    # print '--- Emulate Gateway dead by reset_sequence() ___'
    # reset_sequence()  # Emulate Gateway dead
    # print '--- Up all ---'
    # nodes_up([3,7,11,4,8,12,16])
    # print '--- Emulate Gateway dead by reset_sequence() ___'
    # reset_sequence()  # Emulate Gateway dead

    print '-' * 20


###################################
if __name__ == '__main__':
    sim = Simulator()

    for x in range(3):
        for y in range(3):
            pos = (100 + x*75 + random.randint(0,20),
                   100 + y*75 + random.randint(0,20))
            node = MyMote(firmware, txRange=100, panid=0x22, channel=0x11)
            sim.addNode(node, pos)
            nodes.append(node)

    sim.addNode(MyGateway(), (320,320))

    sim.scene.linestyle("my_style", color=[0,0,0] , dash=(1,2,2,2), arrow='head')
    sleep(1)

    sim.run(bootMotes=False, script=script)
