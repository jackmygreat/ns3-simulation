'''An example to create a fat tree topology:
'''

from config_helper import FattreeGenerator

globalConfig = {
    "randomSeed": 0, 
}

hostPortConfig = {
    # "type": "ns3::DcbNetDevice",
    "pfcEnabled": False
}

# Configuration of a group of hosts. In this topology, we shall have 2 groups of hosts.
hostGroupConfig = {
    # "num": 2,                       # number of hosts in this group
    "ports": [hostPortConfig]       # one port per host
}

switchPortQueueConfig = {
    # PFC configurations
    "pfcReserve": "60 KiB",
    "pfcXon":     "50 KiB",

    # ECN configurations
    "ecnKMin": "40 KiB",
    "ecnKMax": "60 KiB",
    "ecnPMax": 0.6,
}

# Configuration of the switch port that connects to host
switchPortConfig = {
    "pfcEnabled": True,
    "ecnEnabled": True,
    "queues": [switchPortQueueConfig] * 8
}

# Configuration of a group of switches. In this example, only one group is needed 
switchGroupConfig = {
    # "num": 2,                         # number of switches in this group
    "pfcDynamic": False,              # whether enabling PFC dynamic threshold
    "bufferSize": "13180 KB",         # buffer size of the switch
    "queueNum": 8,
    "ports": [switchPortConfig] * 4,  # a list of ports
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
    "dest": 8        # Optional. If not set, application will randomly choose destinations
}

receiverConfig = {
    "nodeIndices": [8],
    "protocolGroup": "RoCEv2",
    "cdf": "WebSearch",
    "load": 0.0,
    "startTime": 1000,
    "stopTime": 6000,
}

with FattreeGenerator(k=4) as topo:
    topo.setHostConfigAll(hostGroupConfig)
    topo.setEdgeConfigAll(switchGroupConfig)
    topo.setAggConfigAll(switchGroupConfig)
    topo.setCoreConfig(switchGroupConfig)
    topo.setLinkConfigAll(linkConfig)
    
    topo.applications.installApplication(senderConfig)
    topo.applications.installApplication(receiverConfig)
    
