from .utils import Utils, NodeGenerator


def createSwitches():
    ...


def main():
    nodeGen = NodeGenerator()
    nodeGen.addHostGroup(hostsNum=4)
    nodeGen.addSwitchGroup(switchesNum=2)
    
    

if __name__ == '__main__':
    main()
