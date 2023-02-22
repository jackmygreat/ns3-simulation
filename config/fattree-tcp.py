'''Bifrost TCP simulation
Two 4-ary Fat-trees connected through two DCI switches.
'''

from topology_helper import FattreeGenerator

hostPortConfig = {
    "pfcEnabled": False
}

# Configuration of a group of hosts. In this topology, we shall have 2 groups of hosts.
hostGroupConfig = { 
    "ports": [hostPortConfig]       # one port per host
}

switchPortConfig = {
    "pfcEnabled": False,
    "ecnEnabled": False,
    "queues": [{}] * 8
}

switchGroupConfig = {
    "pfcDynamic": False,              # whether enabling PFC dynamic threshold
    "bufferSize": "13180 KB",         # buffer size of the switch
    "queueNum": 8,
    "ports": [switchPortConfig] * 4,  # a list of ports
}

coreGroupConfig = {
    "pfcDynamic": False,
    "bufferSize": "13180 KB",
    "queueNum": 8,
    "ports": [switchPortConfig] * 5
}

dciGroupConfig = {
    "num": 2,
    "pfcDynamic": False,
    "bufferSize": "13180 KB",
    "queueNum": 8,
    "ports": [switchPortConfig] * 5
}
 
# Configuration of link intra-DC 
linkConfig = {
    "rate": "100 Gbps",
    "delay": "1 us",
}

longLinkConfig = {
    "rate": "400 Gbps",
    "delay": "3 ms" # 600 km
}

senderConfig = {
    "nodeIndices": list(range(32)), # all hosts
    "appName": "PreGeneratedApplication",
    "fileName": "config/flow.csv",
    "protocolGroup": "TCP",
    "startTime": "1 s",
    "stopTime": "1100 ms",
}

receiverConfig = {
    "nodeIndices": list(range(32)), # all hosts
    "appName": "PacketSink",
    "startTime": "900 ms",
    "stopTime": "1400 ms",
}

def addLinksToFatTree(topo, hostGroups, edgeGroups, aggGroups, coreGroup):
    halfRadix = topo.getNHostsPerToR()
    for podIndex in range(topo.getNPods()):
        # 1. connect hosts to ToR
        hosts = hostGroups[podIndex]
        for i in range(0, len(hosts), halfRadix):
            tor = edgeGroups[podIndex][i // halfRadix]
            topo.links.connectN2One(nNodes=hosts[i:i+halfRadix], port=0,
                                    one=tor, nPorts=range(halfRadix),
                                    **topo.linkPodTier[podIndex][0])
        # 2. connect ToRs to Aggregate switches
        tors, aggs = edgeGroups[podIndex], aggGroups[podIndex]
        topo.links.connectM2N(nodes1=tors, ports1=range(halfRadix, halfRadix * 2),
                              nodes2=aggs, ports2=range(halfRadix),
                              **topo.linkPodTier[podIndex][1])
        # 3. connect Aggregate switches to Core switches
        for i in range(0, len(coreGroup), halfRadix):
            agg = aggGroups[podIndex][i // halfRadix]
            topo.links.connectN2One(nNodes=coreGroup[i:i+halfRadix], port=podIndex,
                                    one=agg, nPorts=range(halfRadix, halfRadix * 2),
                                    **topo.linkPodTier[podIndex][2])


topo = FattreeGenerator(k=4)

topo.setHostConfigAll(hostGroupConfig)
topo.setEdgeConfigAll(switchGroupConfig)
topo.setAggConfigAll(switchGroupConfig)
topo.setCoreConfig(coreGroupConfig, checkRadix=False)
topo.setLinkConfigAll(linkConfig)
    
topo.applications.installApplication(senderConfig)
topo.applications.installApplication(receiverConfig)

hostGroups1 = [topo.nodes.addHostGroup(**config) for config in topo.hostPod]
hostGroups2 = [topo.nodes.addHostGroup(**config) for config in topo.hostPod]

# create switches in first topo
edgeGroups, aggGroups = [], []
for edge, agg in zip(topo.edgeSwitchPod, topo.aggSwitchPod):
    edgeGroups.append(topo.nodes.addSwitchGroup(**edge))
    aggGroups.append(topo.nodes.addSwitchGroup(**agg))
coreGroup1 = topo.nodes.addSwitchGroup(**topo.coreSwitch)

addLinksToFatTree(topo, hostGroups1, edgeGroups, aggGroups, coreGroup1)

# create switches in second topo
edgeGroups, aggGroups = [], []
for edge, agg in zip(topo.edgeSwitchPod, topo.aggSwitchPod):
    edgeGroups.append(topo.nodes.addSwitchGroup(**edge))
    aggGroups.append(topo.nodes.addSwitchGroup(**agg))
coreGroup2 = topo.nodes.addSwitchGroup(**topo.coreSwitch)

addLinksToFatTree(topo, hostGroups2, edgeGroups, aggGroups, coreGroup2)

# connect DCI
dci1, dci2 = topo.nodes.addSwitchGroup(**dciGroupConfig)        
topo.links.connectN2One(nNodes=coreGroup1, port=4,
                        one=dci1, nPorts=range(4), **linkConfig)
topo.links.connectN2One(nNodes=coreGroup2, port=4,
                        one=dci2, nPorts=range(4), **linkConfig)
topo.links.connectOne2One(node1=dci1, port1=4, node2=dci2, port2=4, **longLinkConfig)

topo.serializeToDefault() # write file
