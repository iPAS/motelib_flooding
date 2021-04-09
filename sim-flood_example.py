from motesim import Simulator, Mote, Gateway
import random

###################################
sim = Simulator()
sim.addNode( Gateway(), (50, 50) )
for x in range(5):
    for y in range(5):
        pos = (100+x*75+random.randint(0,20), 100+y*75+random.randint(0,20))
        sim.addNode( Mote('build/sim/flood.elf'), pos )

sim.run()
