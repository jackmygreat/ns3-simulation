r'''An example to create a dumbell topology:

host0 -- switch3
                \
                 switch6 -- switch5 -- host2
                /   
host1 -- switch4                

'''

from topology_helper import TopologyGenerator

globalConfig = {
    "randomSeed": 0, 
}

hostPortConfig = {
    # "type": "ns3::DcbNetDevice",
    "pfcEnabled": False
}

# Configuration of a group of hosts. In this topology, we shall have 2 groups of hosts.
hostGroupConfig = {
    "num": 3,                       # number of hosts in this group
    "ports": [hostPortConfig]       # one port per host
}

switchPortQueueConfig1 = {
    # PFC configurations
    "pfcXon":     "60 KiB",
    "pfcReserve": "100 KiB",

    # ECN configurations
    "ecnKMin": "200 KiB",
    "ecnKMax": "800 KiB",
    "ecnPMax": 0.2,
}

switchPortQueueConfig2 = {
    # PFC configurations
    "pfcXon":     "60 KiB",
    "pfcReserve": "100 KiB",

    # ECN configurations
    "ecnKMin": "90 KiB",
    "ecnKMax": "150 KiB",
    "ecnPMax": 0.4,
}

# Configuration of the switch port that connects to host
switchPortConfig1 = {
    "pfcEnabled": False,
    "ecnEnabled": True,
    "queues": [switchPortQueueConfig1] * 8
}

# Configuration of the switch port that connects to switch
switchPortConfig2 = {
    "pfcEnabled": True,
    "ecnEnabled": True,
    "queues": [switchPortQueueConfig2] * 8
}

# Configuration of a group of switches. In this example, only one group is needed 
switchGroupConfig1 = {
    "num": 3,                         # number of switches in this group
    "pfcDynamic": False,              # whether enabling PFC dynamic threshold
    "bufferSize": "13180 KB",         # buffer size of the switch
    "queueNum": 8,
    "ports": [switchPortConfig1, switchPortConfig1],  # a list of ports
}

switchGroupConfig2 = {
    "num": 1,
    "pfcDynamic": False,
    "bufferSize": "13180 KB",
    "queueNum": 8,
    "ports": [switchPortConfig2] * 3
}

# Configuration of link between host and switch 
linkConfig = {
    "rate": "100 Gbps",
    "delay": "2 us",
}


senderConfig = {
    "nodeIndices": [0, 1],
    "protocolGroup": "RoCEv2",
    "cdf": "WebSearch",
    "load": 0.9,
    "startTime": 2000,
    "stopTime": 4000,
    "dest": 2
}

receiverConfig = {
    "nodeIndices": [2],
    "protocolGroup": "RoCEv2",
    "cdf": "WebSearch",
    "load": 0.0,
    "startTime": 1000,
    "stopTime": 6000,
}

with TopologyGenerator() as topo:
    
    hosts = topo.nodes.addHostGroup(**hostGroupConfig)
    switches = topo.nodes.addSwitchGroup(**switchGroupConfig1)
    core, = topo.nodes.addSwitchGroup(**switchGroupConfig2)

    for host, switch in zip(hosts, switches):
        topo.links.connectOne2One(host, 0, switch, 0, **linkConfig)

    for i, switch in enumerate(switches):
        topo.links.connectOne2One(core, i, switch, 1, **linkConfig)

    topo.applications.installApplication(senderConfig)
    topo.applications.installApplication(receiverConfig)
    
