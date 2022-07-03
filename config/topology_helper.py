from topology_pb2 import HostPortConfig, HostGroup, SwitchPortConfig, SwitchGroup, \
    AllNodes, Link, Topology
from collections.abc import Iterable
from typing import List, Tuple
import re
from itertools import zip_longest, chain


class Utils:

    @staticmethod
    def serialize(obj, fname):
        with open(fname, 'wb') as fp:
            fp.write(obj.SerializeToString())

    @staticmethod
    def deserialize(obj, fname):
        with open(fname, 'rb') as fp:
            return obj.ParseFromString(fp.read())


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
            "KB": 1024,
            "MB": 1024**2,
            "GB": 1024**3,
            "TB": 1024**4
        }
        
        return int(number * unitMapper[unit])

    
def hostPortGenerate(pfcEnabled=False):
    portConfig = HostPortConfig()
    portConfig.pfcEnabled = pfcEnabled
    return portConfig


def switchPortGenerate(
        queueNum=2,
        pfcEnabled=False, pfcReserved=None, pfcHeadroom=None, pfcPassThrough=False,
        ecnEnabled=False, ecnKMin=None, ecnKMax=None, ecnPMax=None,
):
    portConfig = SwitchPortConfig()
    portConfig.queueNum = queueNum
        
    portConfig.pfcEnabled = pfcEnabled
    if pfcEnabled:
        portConfig.pfcReserved = pfcReserved
        portConfig.pfcHeadroom = pfcHeadroom
        portConfig.pfcPassThrough = pfcPassThrough
            
    portConfig.ecnEnabled = ecnEnabled
    if ecnEnabled:
        portConfig.ecnKMin = ecnKMin
        portConfig.ecnKMax = ecnKMax
        portConfig.ecnPMax = ecnPMax
            
    return portConfig  
    

class NodesGenerator:

    def __init__(self):
        self.index = 0
        self.hostGroups: List[HostGroup] = []
        self.switchGroups: List[SwitchGroup] = []

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
        group = HostGroup()
        group.ports.extend([hostPortGenerate(**port) for port in ports])
        return self.addToGroup(self.hostGroups, group, num)

    def addSwitchGroup(self, num: int,
                       pfcDynamic: bool, bufferSize: str, ports: List[dict]):
        if not isinstance(ports, list):
            raise TypeError("parameter `ports` should be a list")
        
        group = SwitchGroup()
        group.pfcDynamic = pfcDynamic
        group.bufferSize = Units.parseBytes(bufferSize)
        group.ports.extend([switchPortGenerate(**port) for port in ports])
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
        link.rate = rate
        link.delay = delay
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
            self.connectN2One(nodes2, p, nodes, ports1, rate, delay)        

    def getAllLinks(self):
        return self.links


class TopologyGenerator:

    def __init__(self, output="config/topology.bin"):
        '''Initiate the generator.
        Notice: you should never manually assign another string to output as it should be
        consistent with the `ProtobufTopologyLoader::m_protoBinaryName` in
        src/protobuf-topology/helper/protobuf-topology-loader.h
        '''
        self.nodes = NodesGenerator()
        self.links = LinksGenerator()
        self.outputFile = output

    def __enter__(self):
        return self

    def __exit__(self, ex_type, ex_value, ex_traceback):
        self.serializeToDefault()
        
    def getTopology(self):
        topo = Topology()
        topo.nodes.CopyFrom(self.nodes.getAllNodes())
        topo.links.extend(self.links.getAllLinks())
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
