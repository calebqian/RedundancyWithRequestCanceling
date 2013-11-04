# simluation to study the effect of cancellation of request
# author: Jianxiong Gao, Junle Qian
# University of Illinois at Urbana Champaign
# the model starts with detemrinistric disk access time
import random,  copy
import matplotlib.pyplot as plt

#define minimum disk access time is 5ms, maximum is 25ms
minimum_access_time = 500
access_time_range = 2000

Q = []

# define number of servers N, each is indepedent
# those servers will be uniformly picked
N = 10
# define lamda for Possion process
#propotional to time interval, and recipical of load

LAMBDA = [0.01, 0.02, 0.04, 0.05, 0.07, 0.10, 0.12, 0.13, 0.14, 0.15, 0.20, 0.25, 0.30]
#arrival rate

DQLENGTHS = []
DQLENGTHS_CANCEL = []
DRESPONSE = []
DRESPONSE_CANCEL = []
MAXLENGTHS = []
MAXRESPONSE = []
MAXLENGTHS_CANCEL = []
MAXRESPONSE_CANCEL = []
TIMEUNITS = []
TIMEUNITS_CANCEL = []

# define limit for time units
T = 2000000

# define a total number of jobs
J = 1000

# declare statistic array for qlength
qlengths = []


class AccessJob:
    seqNum = 0
    timestamp = 0
    otherQueue = 0

    def __init__(self, time, other, seq):
        self.seqNum = seq
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

def AreEmptyForAllQueues(queto):
    ret = True
    for i in range (0, N):
        if len(queto[i].sq) > 0:
            ret = False
            break
    return ret

def displayAllQueues(queto):
    for i in range(0, N):
        print "("+str(i)+")"+"["
        qsize = len(queto[i].sq)
        for x in range(0, qsize):
            qelem = queto[i].sq[x]
            print "<"+str(qelem.timestamp)+"|"+str(qelem.otherQueue)+">"+"-"
        print "]"+", "
            

##### main block
def singleSimulation(lamda):
    global qlengths
    qlengths = []
    global Q
    Q = []
    print "Starting simulation for lambda equal to "+str(lamda)
    for i in range (0, N):
        server_queue = []
        qlength = []
        sq_obj = ServerQ(0, server_queue)
        Q.append(sq_obj)
        qlengths.append(qlength)
    
    timestamp = random.expovariate(lamda)
    # initialize all jobs in all queues
    for j in range(0, J):
        request_first_server = UniformlyPickServer(N)
        request_second_server = UniformlyPickServer(N-1)
        if request_second_server==request_first_server:
            request_second_server += 1;
        job_first_server = AccessJob(timestamp, request_second_server, j)
        Q[request_first_server].sq.append(job_first_server)
        job_second_server = AccessJob(timestamp, request_first_server, j)
        Q[request_second_server].sq.append(job_second_server)
        my_interval = random.expovariate(lamda)
      #  print my_interval
        timestamp += my_interval
    
    
    
    ### simulate a runtime, each for loop is one unit time
    
    # record response time for multiple jobs
    
    

    # two rounds, first round with cancel, second round without
    for rnd in range(0, 2):
        print "started round # "+str(rnd)+" :"
        response = {}
        t = 0l
        copyQlen = copy.deepcopy(qlengths)
        copyQ = copy.deepcopy(Q)
        if rnd == 0:
            cancel = True
        else:
            cancel = False
        while AreEmptyForAllQueues(copyQ) == False:
            for qindex in range(0, N):
                job_queue = copyQ[qindex].sq
                qsize = len(job_queue)
                counter = 0l
                for item in range(0, qsize):
                    if job_queue[item].timestamp <= t:
                        counter += 1
                
                copyQlen[qindex].append(counter)
                
                if copyQ[qindex].processing > 0:
                    copyQ[qindex].processing -= 1
                    if copyQ[qindex].processing == 0:
                        # dequeue
                        finished = copyQ[qindex].sq.pop(0)
                        interval = t-finished.timestamp
                        if finished.timestamp in response.keys():
                            interval = min(response[finished.timestamp], interval)
                        response.update({finished.timestamp:long(interval)})
                elif qsize != 0:
                    # queue is not empty
                    if copyQ[qindex].sq[0].timestamp <= t:
                        copyQ[qindex].processing = UniformlyPickDiskAccessTime(minimum_access_time, access_time_range)
                        # cancel job in the other queue here
                        if cancel == True:
                            current_job = copyQ[qindex].sq[0]
                            other_queue = copyQ[current_job.otherQueue].sq
                            other_qsize = len(other_queue)
                            if other_qsize!=0:
                                # when the other queue is not empty, check if the current job is in the other queue
                                # ingore head of queue, since it is being processed, we cannot cancel it anyways
                                for i in range(1, other_qsize):
                                    other_queue_job = other_queue[i];
                                    if current_job.seqNum == other_queue_job.seqNum:
                                        # it is found, check if it is head of queue
                                        other_queue.pop(i)
                                        # break, assuming seqNum won't be duplicated between two jobs in the same queue
                                        break
                                # else not found, do nothing
            t+=1      
                                        
                                
                        
                        # cancel job in the other queue here
                    # else:
                        # if this job has not virtually arrvied yet, do nothing
                # else:
                    # if this queue is empty, do nothing
                
                
        # displayAllQueues()
                
                
                
        ## statistical summary
        
        # average queue lengths
        avg_qlengths = []
        max_qlengths = []
        for i in range (0, N):
            avg_qlengths.append(sum(copyQlen[i])/float(len(copyQlen[i])))
            max_qlengths.append(max(copyQlen[i]))
        avgqlens = sum(avg_qlengths)/float(len(avg_qlengths))    
        avg_max_lens = sum(max_qlengths)/float(len(max_qlengths))
        # response time
        sum_response = 0
        avg_response = 0
        max_response = 0
        for x in response:
            if response[x] > max_response:
                max_response = response[x]
            sum_response += response[x]
        
        avg_response = sum_response/float(len(response))
        
        if cancel == True:
            global TIMEUNITS_CANCEL
            TIMEUNITS_CANCEL.append(t)
            temp_dql = DQLENGTHS_CANCEL
            temp_dqr = DRESPONSE_CANCEL
            temp_dqml = MAXLENGTHS_CANCEL
            temp_dqmr = MAXRESPONSE_CANCEL
        else:
            global TIMEUNITS
            TIMEUNITS.append(t)
            temp_dql = DQLENGTHS
            temp_dqr = DRESPONSE
            temp_dqml = MAXLENGTHS
            temp_dqmr = MAXRESPONSE
        temp_dql.append(avgqlens)
        temp_dqr.append(avg_response)
        temp_dqml.append(avg_max_lens)
        temp_dqmr.append(max_response)
        print "Avg avg qlength: "+str(avgqlens) +" avg maxqlength: "+str(avg_max_lens)+ " Avg response time: "+str(avg_response)+" max response time: "+str(max_response)
        
        
    
for lam in range(0, len(LAMBDA)):
    singleSimulation(LAMBDA[lam])

    
plt.figure(1)
plt.plot(LAMBDA, DQLENGTHS_CANCEL, 'b')
plt.plot(LAMBDA, DQLENGTHS, 'r')
plt.figure(2)
plt.plot(LAMBDA, DRESPONSE_CANCEL, 'b')
plt.plot(LAMBDA, DRESPONSE, 'r')
plt.figure(3)
plt.plot(LAMBDA, MAXLENGTHS, 'r')
plt.plot(LAMBDA, MAXLENGTHS_CANCEL, 'b')
plt.figure(4)
plt.plot(LAMBDA, MAXRESPONSE_CANCEL, 'b')
plt.plot(LAMBDA, MAXRESPONSE, 'r')
plt.figure(5)
plt.plot(LAMBDA, TIMEUNITS_CANCEL, 'b')
plt.plot(LAMBDA, TIMEUNITS, 'r')