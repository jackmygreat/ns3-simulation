r'''An example to create a dumbell topology:

host0                host2
    \               /
    switch4 -- switch5
    /               \
host1                host3

'''

from topology_helper import TopologyGenerator

hostPortConfig = {
    "pfcEnabled": True
}

# Configuration of a group of hosts. In this topology, we shall have 2 groups of hosts.
hostGroupConfig = {
    "num": 2,                       # number of hosts in this group
    "ports": [hostPortConfig]       # one port per host
}

# Configuration of the switch port that connects to host
switchPortConfig1 = {
    # PFC configurations
    "queueNum": 3,
    "pfcEnabled": True,
    "pfcReserved": "288 KB",
    "pfcHeadroom": "30 KB",
    "pfcPassThrough": False,

    # ECN configurations
    "ecnEnabled": True,
    "ecnKMin": "200 KB",
    "ecnKMax": "800 KB",
    "ecnPMax": 0.2,
}

# Configuration of the switch port that connects to switch
switchPortConfig2 = {
    # PFC configurations
    "pfcEnabled": True,
    "pfcReserved": "300 KB",
    "pfcHeadroom": "50 KB",

    # ECN configurations
    "ecnEnabled": True,
    "ecnKMin": "220 KB",
    "ecnKMax": "840 KB",
    "ecnPMax": 0.2,
}

# Configuration of a group of switches. In this example, only one group is needed 
switchGroupConfig = {
    "num": 2,                         # number of switches in this group
    "pfcDynamic": False,              # whether enabling PFC dynamic threshold
    "bufferSize": "13180 KB",         # buffer size of the switch
    "ports": [switchPortConfig1] * 2 + [switchPortConfig2],  # a list of ports
}

# Configuration of link between host and switch 
linkConfig1 = {
    "rate": "100 Gbps",
    "delay": "2 us",
}

# Configuration of link between switches 
linkConfig2 = {
    "rate": "100 Gbps",
    "delay": "6 us",
}

with TopologyGenerator() as topo:
    ########################################
    # Create nodes in groups
    ########################################
    # First group of hosts on the left
    hostGroup0 = topo.nodes.addHostGroup(**hostGroupConfig)
    # Second group of hosts on the right
    hostGroup1 = topo.nodes.addHostGroup(**hostGroupConfig)

    # Group of switches, in this example, one group is enough.
    # We unpack the return value to be used later
    switch0, switch1 = topo.nodes.addSwitchGroup(**switchGroupConfig)

    ########################################
    # Connect nodes with links
    ########################################
    topo.links.connectN2One(nNodes=hostGroup0, port=0,
                            one=switch0, nPorts=[0, 1], **linkConfig1)
    topo.links.connectN2One(nNodes=hostGroup1, port=0,
                            one=switch1, nPorts=[0, 1], **linkConfig1)
    topo.links.connectOne2One(node1=switch0, port1=2,
                              node2=switch1, port2=2, **linkConfig2)
