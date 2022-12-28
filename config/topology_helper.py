from topology_pb2 import GlobalConfig, HostPortConfig, HostGroup, PortQueueConfig, \
    SwitchPortConfig, SwitchGroup, AllNodes, Link, Topology, Application
from collections.abc import Iterable
from typing import List, Tuple
import re
from itertools import zip_longest, chain
from copy import deepcopy


class Utils:

    @staticmethod
    def serialize(obj, fname):
        with open(fname, 'wb') as fp:
            fp.write(obj.SerializeToString())

    @staticmethod
    def deserialize(obj, fname):
        with open(fname, 'rb') as fp:
            return obj.ParseFromString(fp.read())


def _setValuesToMessage(messgaeType, valuesDict):
    '''This is an auxilary function that automatically assign values in dict `valuesDict` to a
    Protobuf message `messasgeType` and return a message instance.
    This is used for simplicity to create simple messages.
    '''
    instance = messgaeType()
    for name, value in valuesDict.items():
        if isinstance(value, list):
            getattr(instance, name).extend(value)
        elif isinstance(value, str):
            setattr(instance, name, value.replace(" ", ""))
        else:
            setattr(instance, name, value)
    return instance

    
def globalConfigGenerate(**kwargs):
    return _setValuesToMessage(GlobalConfig, kwargs)


def switchPortGenerate(queueNum: int,
                       pfcEnabled=True,
                       ecnEnabled=True,
                       queues: List[dict]=[]):
    if not isinstance(queues, list):
        raise TypeError(f"Port configuration should be a list of queue configurations")
    if len(queues) != queueNum:
        raise ValueError(f"The number of queues of one port should be the same. "
                         f"Expecting {queueNum} queues, given {len(queues)} queues configurations")
    portConfig = SwitchPortConfig()
    portConfig.pfcEnabled = pfcEnabled
    portConfig.ecnEnabled = ecnEnabled
    portConfig.queues.extend(
        [_setValuesToMessage(PortQueueConfig, conf) for conf in queues]
    )
    return portConfig


class GlobalConfigGenerator:

    def __init__(self):
        self.globalConfig = GlobalConfig()

    def setConfig(self, randomSeed=0):
        self.globalConfig.randomSeed = randomSeed

    def getGlobalConfig(self):
        return self.globalConfig


class NodesGenerator:

    class State:
        BEGIN = 0
        ADDING_HOST = 1
        ADDING_SWITCH = 2

    def __init__(self):
        self.index = 0
        self.hostGroups: List[HostGroup] = []
        self.switchGroups: List[SwitchGroup] = []
        self.state = self.State.BEGIN

    def addToGroup(self, groupsList, group, nodesNum):
        """Set the BaseIndex and NodesNum and add group to the groupList.
        All the nodes will be assigned an incremental index from baseIndex.
        
        :returns: a range of (baseIndex, baseIndex + nodesNum)

        """
        group.baseIndex = self.index
        group.nodesNum = nodesNum
        groupsList.append(group)
        self.index += nodesNum

        return range(group.baseIndex, group.baseIndex + nodesNum)
        
    def addHostGroup(self, num, ports: List[dict]):
        if self.state > self.State.ADDING_HOST:
            raise ValueError("all hosts should be added before switches")
        self.state = self.State.ADDING_HOST
        group = HostGroup()
        group.ports.extend([_setValuesToMessage(HostPortConfig, port) for port in ports])
        return self.addToGroup(self.hostGroups, group, num)

    def addSwitchGroup(self, num: int,
                       pfcDynamic: bool, bufferSize: str,
                       queueNum: int, ports: List[dict]):
        if not isinstance(ports, list):
            raise TypeError("parameter `ports` should be a list")
        self.state = self.State.ADDING_SWITCH
        group = SwitchGroup()
        # if pfcDynamic:
        #     group.pfcDynamicShift = 2
        group.bufferSize = bufferSize.replace(" ", "")
        group.queueNum = queueNum
        group.ports.extend([switchPortGenerate(queueNum, **port) for port in ports])
        return self.addToGroup(self.switchGroups, group, num)

    def getHostIDsFromGroup(self, groupID=0):
        group = self.hostGroups[groupID]
        baseIndex, nodesNum = group.baseIndex, group.nodesNum
        return list(range(baseIndex, baseIndex + nodesNum))

    def getSwitchIDsFromGroup(self, groupID=0):
        group = self.switchGroups[groupID]
        baseIndex, nodesNum = group.baseIndex, group.nodesNum
        return list(range(baseIndex, baseIndex + nodesNum))

    def getAllHosts(self):
        return self.hostGroups

    def getAllSwitches(self):
        return self.switchGroups

    def getNodesNum(self):
        num = 0
        for group in chain(self.hostGroups, self.switchGroups):
            num += group.nodesNum
        return num

    def getAllNodes(self):
        allNodes = AllNodes()
        allNodes.num = self.getNodesNum()
        allNodes.hostGroups.extend(self.hostGroups)
        allNodes.switchGroups.extend(self.switchGroups)
        return allNodes

    @staticmethod
    def getNamesFromGroup(group, prefix):
        baseIndex, nodesNum = group.baseIndex, group.nodesNum
        return [f"{prefix}{i}"
                for i in range(baseIndex, baseIndex + nodesNum)]

    def getHostsNamesFromGroup(self, groupID=0, prefix="h"):
        group = self.hostGroups[groupID]
        return self.getNamesFromGroup(group, prefix)

    def getSwitchNamesFromGroup(self, groupID=0, prefix="s"):
        group = self.switchGroups[groupID]
        return self.getNamesFromGroup(group, prefix)


class LinksGenerator:

    @staticmethod
    def createLink(node1, port1, node2, port2, rate, delay):
        link = Link()
        link.node1 = node1
        link.node2 = node2
        link.port1 = port1
        link.port2 = port2
        link.rate = rate.replace(" ", "")
        link.delay = delay.replace(" ", "")
        return link
            
    def __init__(self):
        self.links = []

    def connectOne2One(self, node1: int, port1: int,
                       node2: int, port2: int,
                       rate: str, delay: str):
        if not isinstance(port1, int) or not isinstance(port2, int)\
           or port1 <0 or port2 < 0:
            raise ValueError("port number should be non-negative integer value")
        link = LinksGenerator.createLink(node1, port1, node2, port2, rate, delay)
        self.links.append(link)

    def connectN2One(self, nNodes: Iterable[int], port: int,
                     one: int, nPorts: Iterable[int],
                     rate: str, delay: str):
        """Connect N nodes to one node (choose the same port of N nodes).

        :param nodes: a list of nodes
        :param nport: the port index of each node in nodes
        :param one: the node to be connected
        :param ports: the list of ports of `one` to be connected to. Should be the same length as nodes
        :param rate: link rate string
        :param delay: link delay string
        :returns: None

        """
        if not isinstance(nNodes, Iterable):
            raise ValueError(f"the 1st parameters for connectN2One() should be an iterable object (i.e., list-like)")
        if not isinstance(nPorts, Iterable):
            raise ValueError(f"the 4th parameters for connectN2One() should be an iterable object (i.e., list-like)")
        
        for node, p in zip_longest(nNodes, nPorts):
            if node is None or p is None:
                raise ValueError("nodes and ports has different length")
            self.connectOne2One(one, p, node, port, rate, delay)

    def connectM2N(self, nodes1: Iterable[int], ports1: Iterable[int],
                   nodes2: Iterable[int], ports2: Iterable[int],
                   rate: str, delay: str):
        """Connect M nodes to N nodes.

        :param nodes1: a list of M nodes
        :param ports1: a list of N ports of nodes1 that will be connected to nodes2
        :param nodes2: a list of N nodes
        :param ports2: a list of M ports of nodes2 that will be connected to nodes2
        :param rate: link rate string
        :param delay: link delay string
        :returns: None

        """
        if not isinstance(nodes1, Iterable) or not isinstance(nodes2, Iterable):
            raise ValueError(f"the parameters for connectN2One() should be an iterable object (i.e., list-like)")
        for node, p in zip_longest(nodes1, ports2):
            if node is None or p is None:
                raise ValueError("nodes1 and ports2 has different length")
            self.connectN2One(nodes2, p, node, ports1, rate, delay)        

    def getAllLinks(self):
        return self.links

class ApplicationsGenerator:
    
    def __init__(self):
        self.apps = []

    def installApplication (self, conf):
        self.apps.append(_setValuesToMessage(Application, conf))

    def getApplications(self):
        return self.apps
        

class TopologyGenerator:

    def __init__(self, output="config/topology.bin"):
        '''Initiate the generator.
        Notice: you should never manually assign another string to output as it should be
        consistent with the `ProtobufTopologyLoader::m_protoBinaryName` in
        src/protobuf-topology/helper/protobuf-topology-loader.h
        '''
        self.globalConfig = GlobalConfigGenerator()
        self.nodes = NodesGenerator()
        self.links = LinksGenerator()
        self.applications = ApplicationsGenerator()
        self.outputFile = output

    def __enter__(self):
        return self

    def __exit__(self, ex_type, ex_value, ex_traceback):
        self.serializeToDefault()
        
    def getTopology(self):
        topo = Topology()
        topo.globalConfig.CopyFrom(self.globalConfig.getGlobalConfig())
        topo.nodes.CopyFrom(self.nodes.getAllNodes())
        topo.links.extend(self.links.getAllLinks())
        topo.applications.extend(self.applications.getApplications())
        return topo

    def serializeTo(self, fname: str):
        """Serialize the topology configuration to file which is later parsed by
        ns-3

        :param fname: output file name
        :returns: None

        """
        Utils.serialize(self.getTopology(), fname)

    def serializeToDefault(self):
        self.serializeTo(self.outputFile)

    def __str__(self):
        return f"{self.nodes.getAllNodes()}\n{self.links.getAllLinks()}"


class FattreeGenerator(TopologyGenerator):
    '''3-tier Fat tree generator'''

    def __init__(self, k, *args):
        if k <= 0 or k % 2 == 1:
            raise ValueError("the k of k-ary Fat tree should be a postive even number")
        self.k = k
        # configuration of node groups
        self.hostPod = [None] * self.getNPods()
        self.edgeSwitchPod = [None] * self.getNPods()
        self.aggSwitchPod = [None] * self.getNPods()
        self.coreSwitch = None
        #configuration of link groups
        self.linkPodTier = [[None, None, None] for _ in range(self.getNPods())]
        super().__init__(*args)

    def setHostConfigOfPod(self, podIndex, config, copy=True):
        self._checkPodIndex(podIndex)
        if copy:
            config = deepcopy(config)
        config["num"] = self.getNHostsPerPod()
        self.hostPod[podIndex] = config

    def setHostConfigAll(self, config):
        config = deepcopy(config)
        for pod in range(self.getNPods()):
            self.setHostConfigOfPod(pod, config, False)

    def setEdgeConfigOfPod(self, podIndex, config, copy=True):
        self._checkPodIndex(podIndex)
        self._checkSwitchPorts(config)
        if copy:
            config = deepcopy(config)
        config["num"] = self.getNEdgeSwitchesPerPod()
        self.edgeSwitchPod[podIndex] = config

    def setEdgeConfigAll(self, config):
        config = deepcopy(config)
        for pod in range(self.getNPods()):
            self.setEdgeConfigOfPod(pod, config, False)

    def setAggConfigOfPod(self, podIndex, config, copy=True):
        self._checkPodIndex(podIndex)
        self._checkSwitchPorts(config)
        if copy:
            config = deepcopy(config)
        config["num"] = self.getNAggSwitchesPerPod()
        self.aggSwitchPod[podIndex] = config

    def setAggConfigAll(self, config):
        config = deepcopy(config)        
        for pod in range(self.getNPods()):
            self.setAggConfigOfPod(pod, config, False)

    def setCoreConfig(self, config):
        self._checkSwitchPorts(config)
        config = deepcopy(config)
        config["num"] = self.getNCoreSwitches()
        self.coreSwitch = config

    def setLinkConfigOfPodTier(self, podIndex, tierIndex, config, copy=True):
        self._checkPodIndex(podIndex)
        self._checkTierIndex(tierIndex)
        if copy:
            config = deepcopy(config)
        self.linkPodTier[podIndex][tierIndex] = config

    def setLinkConfigOfTier(self, tierIndex, config, copy=True):
        if copy:
            config = deepcopy(config)
        for pod in range(self.getNPods()):
            self.setLinkConfigOfPodTier(pod, tierIndex, config, False)

    def setLinkConfigAll(self, config):
        config = deepcopy(config)        
        for tier in [0, 1, 2]:
            self.setLinkConfigOfTier(tier, config, False)

    def createFattree(self):
        # create nodes
        hostGroups = [self.nodes.addHostGroup(**config) for config in self.hostPod]
        edgeGroups, aggGroups = [], []
        for edge, agg in zip(self.edgeSwitchPod, self.aggSwitchPod):
            edgeGroups.append(self.nodes.addSwitchGroup(**edge))
            aggGroups.append(self.nodes.addSwitchGroup(**agg))
        coreGroup = self.nodes.addSwitchGroup(**self.coreSwitch)

        # add links
        halfRadix = self.getNHostsPerToR()
        for podIndex in range(self.getNPods()):
            # 1. connect hosts to ToR
            hosts = hostGroups[podIndex]
            for i in range(0, len(hosts), halfRadix):
                tor = edgeGroups[podIndex][i // halfRadix]
                self.links.connectN2One(nNodes=hosts[i:i+halfRadix], port=0,
                                        one=tor, nPorts=range(halfRadix),
                                        **self.linkPodTier[podIndex][0])
            # 2. connect ToRs to Aggregate switches
            tors, aggs = edgeGroups[podIndex], aggGroups[podIndex]
            self.links.connectM2N(nodes1=tors, ports1=range(halfRadix, halfRadix * 2),
                                  nodes2=aggs, ports2=range(halfRadix),
                                  **self.linkPodTier[podIndex][1])
            # 3. connect Aggregate switches to Core switches
            for i in range(0, len(coreGroup), halfRadix):
                agg = aggGroups[podIndex][i // halfRadix]
                self.links.connectN2One(nNodes=coreGroup[i:i+halfRadix], port=podIndex,
                                        one=agg, nPorts=range(halfRadix, halfRadix * 2),
                                        **self.linkPodTier[podIndex][2])
            
    def getNCoreSwitches(self) -> int:
        '''Get the number of the core switches.'''
        return (self.k // 2) ** 2

    def getNPods(self) -> int:
        '''Get the number of pods.'''
        return self.k

    def getNAggSwitchesPerPod(self) -> int:
        '''Get the number of the aggregate switches per pod.'''
        return self.k // 2

    def getNEdgeSwitchesPerPod(self) -> int:
        '''Get the number of the edge switches per pod.'''
        return self.k // 2

    def getNHostsPerToR(self) -> int:
        '''Get the number of hosts per ToR (i.e., edge switch).'''
        return self.k // 2

    def getNHostsPerPod(self) -> int:
        '''Get the number of hosts per pod'''
        return self.getNHostsPerToR() * self.getNEdgeSwitchesPerPod()

    def getNHosts(self) -> int:
        '''Get the total number of hosts in the Fat tree'''
        return self.getNHostsPerPod * self.getNPods()

    def getNToRs(self) -> int:
        '''Get the number of ToRs in the Fat tree'''
        return self.getNEdgeSwitchesPerPod() * self.getNPods()

    def getToRofHost(self, host) -> int:
        return host // self.getNHostsPerToR()

    def _checkPodIndex(self, index):
        if index < 0 or index > self.getNPods():
            raise ValueError(f"Pod index out of range. You input {index} but {self.k}-ary Fat tree has {self.getNPods()} pods.")

    def _checkTierIndex(self, index):
        if index not in {0, 1, 2}:
            raise ValueError(f"the tier of Fat tree should be 0, 1 or 2, not {index}")

    def _checkSwitchPorts(self, config):
        if "ports" not in config:
            raise KeyError("Switch config must have 'ports' field")
        if len(config["ports"]) != self.k:
            raise ValueError(f"Switch radix must be k. Your switch has "
                             f"{len(config['ports'])} ports but the k is {self.k}")

class Units:

    @staticmethod
    def parsePattern(numberPattern, unitPattern, string) -> Tuple[float, str]:
        """Parse a string in format '{number} {unit}'

        :param numberPattern: the number pattern (e.g. r"\d+(\.\d+)?")
        :param unitParttern: the unit pattern
        :param string: the string to be parsed 
        :returns Tuple[float, str]: parsed result in tuple (number, unit) 

        """
        string = string.strip() # strip whitespaces
        
        pattern = f"{numberPattern}\\s*{unitPattern}"
        if not re.match(pattern, string):
            raise ValueError(f"'{string}' cannot be parsed")

        number = re.search(numberPattern, string) # must be not None
        unit = re.search(unitPattern, string) # must be not None

        if number is None or unit is None :
            raise ValueError("Unknown error when parsing the string")
        
        number = float(number.group()) 
        unit = unit.group()
        
        return number, unit

    @staticmethod
    def parseBytes(size) -> int:
        '''Parse a string of size in units of:
            (B, KB, MB, GB, TB) to bytes
        '''
        digitsPattern = r"\d+(\.\d+)?"
        unitPattern = r"[KMGT]?B"
        number, unit = Units.parsePattern(digitsPattern, unitPattern, size)
        
        unitMapper = { 
            "B": 1,
            "KB": 1000,
            "MB": 1000**2,
            "GB": 1000**3,
            "TB": 1000**4,
            "KiB": 1024,
            "MiB": 1024**2,
            "GiB": 1024**3,
            "TiB": 1024**4
        }
        
        return int(number * unitMapper[unit])
