from motesim import Simulator, Mote, Gateway, LEDS_CONFIG
from motesim.SimGateway import BROADCAST_ADDR, Gateway as SimGateway
from motesim.MoteSim import strToList, listToStr

import sys
import random
from time import sleep

from threading import Thread

FLOOD_MSG_TYPE = 0x01
REPORT_MSG_TYPE = 0x22

gws = []
gw_positions = [( 320, 55 ), ( 320, 320 )]
line_styles = {
    0: {
        'name': 'gw9_style',
        'color': [ 0, 0, 1. ]
    },
    1: {
        'name': 'gw10_style',
        'color': [ 1., 0, 0 ]
    },
    2: {
        'name': 'node8_style',
        'color': [ .2, .8, .2 ]
    },
}
simgws = []

nodes = []
firmware = 'build/sim/test_comm.elf'

payload = [ i for i in range(90) ]


def node_label_3l(id, seqno, hop):
    return '%d\n%d,%d' % ( id, seqno, hop )


def node_label_2l(id, seqno):
    return '%d\n%d' % ( id, seqno )


###################################
class MySimGateway(SimGateway):
    '''
    Based on flood message structure defined in flood.h,
    typedef struct
    {
        uint8_t seqNo;
        uint8_t hopCount;
        Address originSource;
        Address finalSink;
    } RoutingHeader;
    '''

    ###################
    def __init__(self,
                 device = 'localhost:30000',
                 autoListen = True,
                 sim = None):
        self.msgSeqNo = 0
        SimGateway.__init__(self,
                            device = device,
                            autoListen = autoListen,
                            sim = sim)

    ###################
    def receive(self, source, msgType, msg):
        if isinstance(msg, str):
            msg = strToList(msg)

        hdr = msg[0:6]
        seq_no = int(hdr[0])
        hop_count = int(hdr[1])
        origin = int(hdr[2]) + int(hdr[3]) * 256
        sink = int(hdr[4]) + int(hdr[5]) * 256

        self.debug('MySimGateway received from=%x type=%d: %s' %
                   ( source, msgType, msg ))

        if msgType == FLOOD_MSG_TYPE:
            self.debug('MySimGateway got flood seqNo %d from node %d' %
                       ( seq_no, source ))

        # Process REPORT_MSG_TYPE
        elif msgType == REPORT_MSG_TYPE:     # and seq_no > self.frameId:
            if seq_no > self.msgSeqNo:
                self.debug('MySimGateway changes seqNo from current %d to %d' %
                           (self.msgSeqNo, seq_no))
                self.msgSeqNo = seq_no

        sim.scene.nodelabel(self.nodeId,
                            node_label_2l(self.nodeId, self.msgSeqNo))

    ###################
    def send_to(self, dest, msg):
        if isinstance(msg, str):
            msg = strToList(msg)

        self.msgSeqNo += 1
        sim.scene.nodelabel(self.nodeId,
                            node_label_2l(self.nodeId, self.msgSeqNo))

        msg = [
            self.msgSeqNo, FLOOD_MSG_TYPE, self.nodeId % 256, self.nodeId / 256,
            dest % 256, dest / 256
        ] + msg
        SimGateway.send(self,
                        dest = BROADCAST_ADDR,
                        msgType = FLOOD_MSG_TYPE,
                        msg = msg)
        self.debug('MySimGateway broadcasts: %s' % (msg))


###################################
class MyGateway(Gateway):

    ###################
    def debug(self, msg):
        # if msg.find('Gateway starts listening on port') >= 0:
        #     self.wait_simgw_port = msg.split(' ')[5]  # Server port
        #     Gateway.debug(self, 'port: %s %s' % (self.wait_simgw_port, self.listen_port))
        Gateway.debug(self, msg)

    ###################
    def receiveRadioMsg(self, msg, rssi):
        self.debug('MyGateway received with rssi=%d: %s' % ( rssi, msg ))
        Gateway.receiveRadioMsg(self, msg, rssi)


###################################
class MyMote(Mote):

    ###################
    def debug(self, msg):
        # Mote.debug(self, msg)  # XXX: for debugging
        txt = msg.split(' ')

        if msg.find('Tx to finalSink') >= 0:     # In flood_send_to(..)
            self.seqno = int(txt[6])
            sim.scene.nodelabel(
                self.id, node_label_3l(self.id, self.seqno, self.besthop))

        elif msg.find('Change parent from') >= 0:     # In set_besthop(..)
            self.old_parent = int(txt[3])
            self.new_parent = int(txt[5])
            self.origin = int(txt[8])
            if self.old_parent != 0xFFFF:
                try:
                    sim.scene.dellink(self.id, self.old_parent,
                                      line_styles[self.origin]['name'])
                except:
                    pass
            sim.scene.addlink(self.id, self.new_parent,
                              line_styles[self.origin]['name'])

        elif msg.find('Change seqNo from') >= 0:     # In on_receive(..)
            self.seqno = int(txt[6])
            sim.scene.nodelabel(
                self.id, node_label_3l(self.id, self.seqno, self.besthop))

        elif msg.find('New best hop') >= 0:     # In set_besthop(..)
            self.besthop = int(txt[3])
            sim.scene.nodelabel(
                self.id, node_label_3l(self.id, self.seqno, self.besthop))

        else:
            Mote.debug(self, msg)

    ###################
    def boot(self):
        self.seqno = 0
        self.besthop = 255
        sim.scene.nodelabel(self.id, node_label_2l(self.id, 0))
        Mote.boot(self)

    ###################
    def shutdown(self):
        # Delete all fan-in / fan-out of the node
        for n in sim.nodes.values():
            if n.id != self.id:
                for style in line_styles.values():
                    try:
                        sim.scene.dellink(n.id, self.id, style['name'])
                    except:
                        pass
                    try:
                        sim.scene.dellink(self.id, n.id, style['name'])
                    except:
                        pass

        Mote.shutdown(self)


###################################
def nodes_up(ids):
    for i in ids:
        sim.nodes[i].boot()


def nodes_down(ids):
    for i in ids:
        sim.nodes[i].shutdown()


def nodes_push_button(ids, hold_duration = 1.):
    for i in ids:
        sim.nodes[i].pressButton()
    sleep(hold_duration)
    for i in ids:
        sim.nodes[i].releaseButton()


def set_simgw_sequence(simgw, seqno):
    simgw.msgSeqNo = seqno
    sim.scene.nodelabel(simgw.nodeId, node_label_2l(simgw.nodeId,
                                                    simgw.msgSeqNo))


###################################
def simulation_script():
    global gws, simgws, LEDS_CONFIG

    # Beautify the network graph
    NEW_LED_CONFIG = [
        [[ 10, 2 ], [ 1.0, 0.0, 0.0 ]],     # [ [ 5, 5], [1.0, 0.0, 0.0] ],
        [[ 10, 7 ], [ 0.2, 0.8, 0.2 ]],     # [ [ 0, 5], [0.8, 0.8, 0.0] ],
        [[ 10, 13 ], [ 0.0, 0.0, 1.0 ]],     # [ [-5, 5], [0.0, 0.9, 0.0] ],
    ]
    for i, cfg in enumerate(LEDS_CONFIG):
        # cfg[0] = map(lambda x: x+5, cfg[0])
        cfg[0] = NEW_LED_CONFIG[i][0]
        cfg[1] = NEW_LED_CONFIG[i][1]

    for n in range(len(nodes) + len(gws)):     # Plus one for the gw
        sim.scene.nodescale(n, 2.)
        sim.scene.nodelabel(n, node_label_2l(n, 0))

    # Get started!
    SEPARATOR = '\n<<<' + '-' * 80

    print '<<< Script gets started >>>'
    for gw in gws:
        simgws.append(
            MySimGateway('localhost:{}'.format(gw.listen_port), sim = sim))

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
    print SEPARATOR
    print '<<<--- Flood routing protocol testing : node_label ==> (idno, seqno, besthop) --->>>'

    simgw0 = simgws[0]
    simgw1 = simgws[1]

    print SEPARATOR
    print '<<<--- Up all nodes --->>>'
    nodes_up(range(len(nodes)))
    sleep(1)
    simgw0.send_to(dest = 0, msg = payload)
    sleep(4)
    simgw1.send_to(dest = 0, msg = payload)
    sleep(4)
    simgw0.send_to(dest = 0, msg = payload)
    sleep(4)
    simgw1.send_to(dest = 0, msg = payload)
    sleep(10)

    print SEPARATOR
    print '<<<--- Node: #7 #8 are dead / Gateway is dead then alive / seqNo is reset --->>>'
    nodes_down([ 7, 8 ])
    set_simgw_sequence(simgw1, 0)     # Emulate Gateway dead
    sleep(1)
    simgw1.send_to(dest = 0, msg = payload)     # Send out but no route
    sleep(10)

    print SEPARATOR
    print '<<<--- Gateway and its adjustcent nodes #7,8 are rebooted / seqNo is reset --->>>'
    nodes_up([ 7, 8 ])
    set_simgw_sequence(simgw1, 0)     # Emulate Gateway dead
    sleep(1)
    simgw1.send_to(dest = 0, msg = payload)
    sleep(4)
    simgw1.send_to(dest = 0, msg = payload)
    sleep(10)

    NODE_SRC = 8
    print SEPARATOR
    print '<<<--- Node starts sending as an originSource --->>>'
    for i in range(4):
        print
        print '<<<--- push %d --->>>' % i
        nodes_push_button([NODE_SRC])
        sleep(4)

    sleep(10)
    print SEPARATOR
    raw_input('Press ENTER key to quit...')
    sim.tkplot.tk.quit()


###################################
if __name__ == '__main__':
    sim = Simulator()

    # Sensor nodes placement
    # Most of them just provide the relay function, only the 0th node that is the sink.
    for x in range(3):
        for y in range(3):
            pos = (100 + x * 75 + random.randint(0, 20),
                   100 + y * 75 + random.randint(0, 20))
            node = MyMote(firmware, txRange = 100, panid = 0x22, channel = 0x11)
            sim.addNode(node, pos)
            nodes.append(node)

    # Gateways placement
    # Actually, they are the sources which send packets to the zeroth node.
    for x in range(2):
        gw = MyGateway()
        sim.addNode(gw, gw_positions[x])
        gws.append(gw)

    ids_to_styles = [gws[0].id, gws[1].id, nodes[-1].id]
    for x, i in enumerate(ids_to_styles):
        sim.scene.linestyle(line_styles[x]['name'],
                            color = line_styles[x]['color'],
                            dash = ( 1, 2, 2, 2 ),
                            arrow = 'head')
        line_styles[i] = line_styles.pop(x)


    # Waitting...
    sleep(1)

    if len(sys.argv
          ) > 1 and sys.argv[1] == '--wait':     # Waiting a key to start
        raw_input('Press ENTER key to start...')

    # Running the simulation
    sim.run(bootMotes = False, script = simulation_script)
