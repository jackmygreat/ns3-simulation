from flows_pb2 import Flow, Flows

class Utils:

    @staticmethod
    def serialize(obj, fname):
        with open(fname, 'wb') as fp:
            fp.write(obj.SerializeToString())

    @staticmethod
    def deserialize(obj, fname):
        with open(fname, 'rb') as fp:
            return obj.ParseFromString(fp.read())
        

class FlowsGenerator:

    '''Generate flows to protobuf

    Example:
    with FlowsGenerator() as flowgen:
        flowgen.addFlow(protocol="udp", srcNode=0, srcPort=0, dstNode=2, dstPort=0,
                        size=1024*1024, arriveTime=10_000, priority=0)
        flowgen.addFlow(protocol="udp", srcNode=0, srcPort=0, dstNode=3, dstPort=0,
                        size=512*1024, arriveTime=10_010, priority=0)
        flowgen.addFlow(protocol="udp", srcNode=1, srcPort=0, dstNode=2, dstPort=0,
                        size=256*1024, arriveTime=10_020, priority=0)
        flowgen.addFlow(protocol="udp", srcNode=1, srcPort=0, dstNode=3, dstPort=0,
                        size=128*1024, arriveTime=10_030, priority=0)    
    '''

    def __init__(self, output="config/flows.bin"):
        '''Initiate the flows generator.
        Notice: you should never manually assign another string to output as it should be
        consistent with the `ProtobufFlowsLoader::m_protoBinaryName` in
        src/protobuf-topology/helper/protobuf-flows-loader.h
        '''
        self.flowList = [] # list of `FLow`
        self.outputFile = output
        
    def __enter__(self):
        return self

    def __exit__(self, ex_type, ex_value, ex_traceback):
        self.serializeToDefault()

    def addFlow(self, srcNode: int, srcPort: int, dstNode: int, dstPort: int,
                size: int, arriveTime: int, priority: int):
        '''Add a flow.
        srcNode, srcPort, dstNode and dstPort are indices starting from zero.
        size is in unit of bytes.
        arriveTime is in unit of microseconds.
        priority is usually in range of [0, 7].

        Notice that the helper does not check whether the parameters are valid.
        It is the user's job to take care of them.
        '''
        flow = Flow()
        flow.srcNode = srcNode
        flow.srcPort = srcPort
        flow.dstNode = dstNode
        flow.dstPort = dstPort
        flow.size = size
        flow.arriveTime = arriveTime
        flow.priority = priority
        self.flowList.append(flow)

    def getFlows(self):
        flows = Flows()
        flows.flows.extend(self.flowList)
        return flows

    def serializeTo(self, fname: str):
        """Serialize the topology configuration to file which is later parsed by
        ns-3

        :param fname: output file name
        :returns: None

        """
        Utils.serialize(self.getFlows(), fname)

    def serializeToDefault(self):
        self.serializeTo(self.outputFile)
