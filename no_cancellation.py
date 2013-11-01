# simluation to study the effect of cancellation of request
# author: Jianxiong Gao, Junle Qian
# the model starts with detemrinistric disk access time
import random, decimal, numpy
import matplotlib.pyplot as plt

#define minimum disk access time is 5ms, maximum is 25ms
minimum_access_time = 500
access_time_range = 2000


# define number of servers N, each is indepedent
# those servers will be uniformly picked
N = 10
# define lamda for Possion process
lamda = 10.0

# define limit for time units
T = 2000000

# define a total number of jobs
J = 1000

# declare statistic array for qlength
qlengths = []


class AccessJob:
    timestamp = 0
    otherQueue = 0

    def __init__(self, time, other):
        self.timestamp = time
        self.otherQueue = other


class ServerQ:
    processing = 0
    sq = None
    def __init__(self, pro, server_queue):
        self.processing = pro
        self.sq = server_queue



def UniformlyPickDiskAccessTime(_lowerbound, _range):
    return _lowerbound+random.randrange(_range)


def UniformlyPickServer(_num):
    return random.randrange(_num)

def AreEmptyForAllQueues():
    ret = True
    for i in range (0, N):
        if len(Q[i].sq) > 0:
            ret = False
            break
    return ret

def displayAllQueues():
    for i in range(0, N):
        print "("+str(i)+")"+"["
        qsize = len(Q[i].sq)
        for x in range(0, qsize):
            qelem = Q[i].sq[x]
            print "<"+str(qelem.timestamp)+"|"+str(qelem.otherQueue)+">"+"-"
        print "]"+", "
            

##### main block

# define queue for all servers
Q = []
for i in range (0, N):
    server_queue = []
    qlength = []
    sq_obj = ServerQ(0, server_queue)
    Q.append(sq_obj)
    qlengths.append(qlength)

timestamp = numpy.random.poisson(lamda, None)
# initialize all jobs in all queues
for j in range(0, J):
    request_first_server = UniformlyPickServer(N)
    request_second_server = UniformlyPickServer(N-1)
    if request_second_server==request_first_server:
        request_second_server += 1;
    job_first_server = AccessJob(timestamp, request_second_server)
    Q[request_first_server].sq.append(job_first_server)
    job_second_server = AccessJob(timestamp, request_first_server)
    Q[request_second_server].sq.append(job_second_server)
    timestamp += numpy.random.poisson(lamda, None)



### simulate a runtime, each for loop is one unit time

# record response time for multiple jobs
response = {}

for t in range(0, T):
    for qindex in range(0, N):
        qsize = len(Q[qindex].sq)
        qlengths[qindex].append(qsize)
        if Q[qindex].processing > 0:
            Q[qindex].processing -= 1
            if Q[qindex].processing == 0:
                # dequeue
                finished = Q[qindex].sq.pop(0)
                interval = t-finished.timestamp
                if finished.timestamp in response.keys():
                    interval = min(response[finished.timestamp], interval)
                response.update({finished.timestamp:interval})
        elif qsize != 0:
            #queue is not empty
            if Q[qindex].sq[0].timestamp <= t:
                Q[qindex].processing = UniformlyPickDiskAccessTime(minimum_access_time, access_time_range)
                # cancel job in the other queue here
                
                # cancel job in the other queue here
            # else:
                # if this job has not virtually arrvied yet, do nothing
        # else:
            # if this queue is empty, do nothing
        
        
# displayAllQueues()
        
        
        
## statistical summary

# average queue lengths
avg_qlengths = []
for i in range (0, N):
    avg_qlengths.append(float(sum(qlengths[i]))/float(len(qlengths[i])))

plt.bar(range(0,10), avg_qlengths)

# response time
sum_response = 0
avg_response = 0
for x in response:
    sum_response += response[x]

avg_response = float(sum_response)/float(J)
print avg_response