# simluation to study the effect of cancellation of request
# author: Jianxiong Gao, Junle Qian
# the model starts with detemrinistric disk access time
import random, decimal, Queue

#define minimum disk access time is 5ms, maximum is 25ms
minimum_access_time = 500
access_time_range = 2000
access_time_precision = 100

# define number of servers N, each is indepedent
# those servers will be uniformly picked
N = 10
lamda = 1.0

class AccessJob:
    seqNum = 0
    otherQueue = 0
    def __init__(self, seq, qindex):
        self.seqNum = seq
        self.otherQueue = qindex

class ServerQ:
    processing = 0
    sq = None
    def __init__(self, pro, server_queue):
        self.processing = pro
        self.sq = server_queue



def UniformlyPickDiskAccessTime(_lowerbound, _range, _precision):
    return (decimal.Decimal(_lowerbound+random.randrange(_range)))/_precision


def UniformlyPickServer(_num):
    return random.randrange(_num)

def AreEmptyForAllQueues():
    ret = True
    for i in range (0, N):
        if Q[i].sq.empty() == False:
            ret = False
            break
    return ret

### main block

# define queue for all servers
Q = []
for i in range (0, N):
    server_queue = Queue.Queue()
    sq_obj = ServerQ(0, server_queue)
    Q.append(sq_obj)


