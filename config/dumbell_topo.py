r'''An example to create a dumbell topology:

host0                host2
    \               /
    switch4 -- switch5
    /               \
host1                host3

'''

from topology_helper import TopologyGenerator
from flows_helper import FlowsGenerator

globalConfig = {
    "randomSeed": 0
}

hostPortConfig = {
    # "type": "ns3::DcbNetDevice",
    "pfcEnabled": False
}

# Configuration of a group of hosts. In this topology, we shall have 2 groups of hosts.
hostGroupConfig = {
    "num": 2,                       # number of hosts in this group
    "ports": [hostPortConfig]       # one port per host
}

switchPortQueueConfig1 = {
    # PFC configurations
    # "pfcReserved": "288 KiB",
    # "pfcHeadroom": "30 KiB",
    "pfcReserve": "1000B",
    "pfcXon": "1000B",

    # ECN configurations
    "ecnKMin": "200 KiB",
    "ecnKMax": "800 KiB",
    "ecnPMax": 0.2,
}


switchPortQueueConfig2 = {
    # PFC configurations
    # "pfcReserve": "300 KiB",
    # "pfcHeadroom": "50 KiB",
    "pfcReserve": "1000B",
    "pfcXon": "1000B",

    # ECN configurations
    "ecnKMin": "220 KiB",
    "ecnKMax": "840 KiB",
    "ecnPMax": 0.2,
}

# Configuration of the switch port that connects to host
switchPortConfig1 = {
    "pfcEnabled": False,
    "pfcPassThrough": False,
    "ecnEnabled": True,
    "queues": [switchPortQueueConfig1] * 2
}

# Configuration of the switch port that connects to switch
switchPortConfig2 = {
    "pfcEnabled": True,
    "pfcPassThrough": False,
    "ecnEnabled": True,
    "queues": [switchPortQueueConfig2] * 2
}

# Configuration of a group of switches. In this example, only one group is needed 
switchGroupConfig = {
    "num": 2,                         # number of switches in this group
    "pfcDynamic": False,              # whether enabling PFC dynamic threshold
    "bufferSize": "13180 KB",         # buffer size of the switch
    "queueNum": 2,
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
    topo.globalConfig.setConfig(**globalConfig)

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

with FlowsGenerator() as flowgen:
    flowgen.addFlow(srcNode=0, srcPort=0, dstNode=2, dstPort=0,
                    size=8*1024, arriveTime=10_000, priority=0)
    flowgen.addFlow(srcNode=0, srcPort=0, dstNode=3, dstPort=0,
                    size=4*1024, arriveTime=10_010, priority=0)
    flowgen.addFlow(srcNode=1, srcPort=0, dstNode=2, dstPort=0,
                    size=2*1024, arriveTime=10_020, priority=0)
    flowgen.addFlow(srcNode=1, srcPort=0, dstNode=3, dstPort=0,
                    size=1*1024, arriveTime=10_030, priority=0)
