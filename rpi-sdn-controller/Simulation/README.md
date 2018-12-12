# Mininet Simulation 
The simulation of the SDN network was done in Mininet VM (http://mininet.org/). 

## mf2063_external_controller_sim.py
This file is used to an external physical controller. To get it to work has the IP-address be changed to the targeted device IP. The IP can also be the VM's IP-address if the controller is install inside the VM. 

```
c0=net.addController(name='c0',
                      controller=RemoteController,
                      ip='TARGET DEVICE IP',
                      protocol='tcp',
                      port=6633)
```

