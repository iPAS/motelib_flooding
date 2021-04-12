
# Flooding Routing

The implementation of routing algorithm in flooding manner.

If a source node want to send a packet to a sink, 
    the packet is broadcasted to node neighbors -- intermediate nodes.
Expectedly, the packet will be forwarded through intermediates, and, 
    finally meeting the sink.

In case a packet is remembered that it was passed,
    an intermidiate will discard -- no forwarding it.


## Quick Start

- Change directory into "motelib", then ```source motelib-vars.sh```.
- Build the project with ```make```.
- Run the simulator: ```python sim.py```
