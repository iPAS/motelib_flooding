
# Flooding Routing

The simple implementation of routing algorithm in flooding manner.
Its purpose is only for learning.

If a source node want to send a packet to a sink,
    the packet is broadcasted to node neighbors -- intermediate nodes.
Expectedly, the packet will be forwarded through intermediates, and, 
    finally meeting the sink.

In case a packet is remembered that it was passed,
    an intermidiate will discard -- no forwarding it.


## Quick Start

- Change directory into "motelib", then ```source motelib-vars.sh```.
- Build the project with ```make```.
- Run the simulator: ```python sim-flood.py```, to learn the routing mechnism.


## Simulation Result

### Single Source Testing

[![Single-source Test](https://img.youtube.com/vi/W2n9rcofPhM/0.jpg)](https://youtu.be/W2n9rcofPhM?t=35s "Single-source Test")

### Multiple Sources Testing

[![Multi-source Test](https://img.youtube.com/vi/qLKNlzvt8w4/0.jpg)](https://youtu.be/qLKNlzvt8w4?t=35s "Multi-source Test")
