# # # #
# CSCI 4210 Operating Systems
# Project #1 - Schedule Algorithm Simulator
# Ayushi Mishra, Anish Ravi, Alexander Schwartzberg

# # # #
# Import dependencies

import sys
import json # TODO - remove this dependency, just here for debug
# from first_come_first_serve import first_come_first_serve
# from round_robin import round_robin

# # # #
# Helper functions

# Helper function for creating the time log text
def timeLog(time):
    return 'time ' + str(int(time)) + 'ms: '

# Returns an empty result structure for each algorithm
def newResult():
    result = {
        'name': '',
        'avg_cpu_burst': 0,
        'avg_wait': 0,
        'avg_turnaround': 0,
        'total_ctx_switch': 0,
        'total_preemptions': 0
    }
    return result

# Generates the queue log string, i.e. `[Q <empty>]`
def queueLog(queue):
    currentQueue = []
    for process in queue:
        currentQueue.append(process['proc_id'])
    # " ".join(currentQueue)
    if len(queue) == 0:
        return ' [Q <empty>]'
    else:
        return ' [Q ' + ' '.join(currentQueue) + ']'

# Gets a single process in a queue by its proc_id
def getProcess(queue, proc_id):
    for p in queue:
        if (p['proc_id'] == proc_id):
            return p

# Removes a single process from a queue by its proc_id
def removeFromQueue(queue, proc_id):
    Q = []
    for p in queue:
        if p['proc_id'] != proc_id:
            Q.append(p)
    return Q

def addToQueue(queue, process):
    Q = []
    for p in queue:
        Q.append(p)
    Q.append(process)
    Q = sorted(Q, key=lambda x: x['time_remaining'], reverse=False)
    return Q

# Returns a boolean representing whether or not the process is currently in the queue
def inQueue(queue, proc_id):
    currentlyInQueue = False
    for p in queue:
        if p['proc_id'] == proc_id:
            currentlyInQueue = True
    return currentlyInQueue

#function to format how the queue looks at a given time
def getElementsInList(Q):
    if(len(Q) > 0):
        output = "[Q"
        for alph in Q:
            output += " " + alph

        output += "]"
        return output

    return "[Q <empty>]"

def addElementsToQ(Q, processes, currTime):
    #Check to see which processes aren't in the Q
    #If not in the Q, and the process has arrived
    #If the process still has bursts left -> append it
    items = processes.items()
    items = sorted(items, key=lambda tup: tup[1])
    for key, value in items:
        if(key not in Q):
            if(value[0] <= currTime):
                if(value[2] > 0):
                    Q.append(key)
                    elementsInList = getElementsInList(Q)
                    if(value[4] > 0):
                        print(timeLog(currTime) + "Process " + key + " completed I/O; added to ready queue " + elementsInList)
                    else:
                        print(timeLog(currTime) + "Process " + key + " arrived and added to ready queue " + elementsInList)

    return Q

def addElementsToQSRT(Q, processes, currTime, currProcess, t_cs):
    #Check to see which processes aren't in the Q
    #If not in the Q, and the process has arrived
    #If the process still has bursts left -> append it
    
    #Since we're iterating through a dict and order matters -> sort the elements from the dict and traverse those
    items = processes.items()
    #Sort based on remaining time
    items = sorted(items, key=lambda tup: tup[1])
    for key, value in items:  
        if(key not in Q):
            if(value[0] <= currTime):
                if(value[2] > 0):
                    if(currProcess is not None and value[6] + t_cs < processes[currProcess][6]):
                        elementsInList = getElementsInList(Q)
                        if(value[4] > 0):
                            print("time " + str(int(currTime)) + "ms: Process " + key + " completed I/O and will preempt " + currProcess + " " + elementsInList)
                        else:
                            print("time " + str(int(currTime)) + "ms: Process " + key + " arrived and will preempt " + currProcess + " " + elementsInList)
                        Q.insert(0, key)
                        for i in range(len(Q)):
                            if(processes[currProcess][6] < processes[Q[i]][6]):
                                Q.insert(i, currProcess)
                                break
                        if(currProcess not in Q):
                            Q.append(currProcess)
                        #print(Q)
                    else:
                        if(len(Q) == 0):
                            Q.append(key)
                        else:
                            for i in range(len(Q)):
                                if(value[6] < processes[Q[i]][6]):
                                    Q.insert(i, key)
                                    break
                            if(key not in Q):
                                Q.append(key)
                        elementsInList = getElementsInList(Q)
                        if(value[4] > 0):
                            print("time " + str(int(currTime)) + "ms: Process " + key + " completed I/O; added to ready queue " + elementsInList)
                        else:
                            print("time " + str(int(currTime)) + "ms: Process " + key + " arrived and added to ready queue " + elementsInList)  
    return Q  
    
def addElementsToQRR(Q, processes, currTime, rr_add, currProcess=None):
    #Check to see which processes aren't in the Q
    #If not in the Q, and the process has arrived
    #If the process still has bursts left -> append it
    for key in sorted(processes):
        value = processes[key]
        if(key not in Q and key != currProcess):
            if(value[0] <= currTime):
                if(value[2] > 0):
                    if (value[6] > 0 or rr_add == "END"):
                        Q.append(key)
                    else:
                        Q.insert(0, key)
                    elementsInList = getElementsInList(Q)
                    if (value[6] == 0):
                        if(value[4] > 0):
                            print(timeLog(currTime) + "Process " + key + " completed I/O; added to ready queue " + elementsInList)
                        else:
                            print(timeLog(currTime) + "Process " + key + " arrived and added to ready queue " + elementsInList)

    return Q

def checkIfAllProcessesDone(processes):
    for key, value in processes.items():
        if(value[2] > 0):
            return False
    return True

def get_fcfs_processes(input_file):

  file = open(input_file,'r')

  processes = dict()
  #Extract info from input file
  for line in file:
      if(len(line) > 0 and line[0] != '#'):
          elements = line.split('|')
          for i in range(1,len(elements)):
              elements[i] = int(elements[i])
              processes[line[0]] = elements[1:]
              #append another line to track how many processes have been completed
              processes[line[0]].append(0) #[4]
              #append another line to track total wait time
              processes[line[0]].append(0) #[5]

  file.close()
  return processes

def get_srt_processes(input_file):
    #Extract info from input file

    file = open(input_file,'r')
    processes = dict()
    for line in file:
        if(len(line) > 0 and line[0] != '#'):
            elements = line.split('|')        
            for i in range(1,len(elements)):
                elements[i] = int(elements[i])
                processes[line[0]] = elements[1:]
                #append another line to track how many processes have been completed
                processes[line[0]].append(0) #[4]
                #append another line to track total wait time
                processes[line[0]].append(0) #[5]
                #append another line to track amount of time used in current CPU burst [6]
                processes[line[0]].append(processes[line[0]][1])
                #append another line store the initial arrival time for each unique burst 
                #this will be used for the turnaround time calculation [7]
                processes[line[0]].append(processes[line[0]][0])
                #append another line to store turnaround time
                processes[line[0]].append(0) #[8]

    file.close()
    return processes
  
def get_rr_processes(input_file):
    file = open(input_file,'r')

    processes = dict()

    #Extract info from input file
    for line in file:
        if(len(line) > 0 and line[0] != '#'):
            elements = line.split('|')
            for i in range(1,len(elements)):
                elements[i] = int(elements[i])
                processes[line[0]] = elements[1:]
                #append another line to track how many processes have been completed
                processes[line[0]].append(0) #[4]
                #append another line to track total wait time
                processes[line[0]].append(0) #[5]
                processes[line[0]].append(0) #[6] - remaining time after preemption
                processes[line[0]].append(0) #[7] - turnaround time
                processes[line[0]].append([]) #[8]
                #this will be used for the turnaround time calculation [9]
                processes[line[0]].append(processes[line[0]][0])
                #append another line to store turnaround time [10]
                processes[line[0]].append(0) #[8]

    file.close()
    return processes

# Writes the algorithm statistics to a file
def writeOutput(output_file, results):
    with open(output_file, 'w') as f:
        for each in results:
            f.write('Algorithm ' + each['name'] + '\n')
            f.write('-- average CPU burst time: ' + str(format(each['avg_cpu_burst'], '.2f')) + ' ms' + '\n')
            f.write('-- average wait time: ' + str(format(each['avg_wait'], '.2f')) + ' ms' + '\n')
            f.write('-- average turnaround time: ' + str(format(each['avg_turnaround'], '.2f')) + ' ms' + '\n')
            f.write('-- total number of context switches: ' + str(int(each['total_ctx_switch'])) + '\n')
            f.write('-- total number of preemptions: ' + str(int(each['total_preemptions'])) + '\n')

    f.closed

# Parses input file
def parseInput(input_file):
    processes = []
    with open(input_file, 'r') as f:
        process_data = f.read().split("\n")

        for line in process_data:
            line = line.strip()

            if (not line or line[0] == '#'):
                continue

            process = {}
            line = line.split('|')

            # Core attributes
            process['proc_id'] = line[0]
            process['arrival_time'] = int(line[1])
            process['burst_time'] = int(line[2])
            process['burst_count'] = int(line[3])
            process['io_time'] = int(line[4])

            # Helper attributes
            process['time_remaining'] = int(line[2])
            process['burst_remaining'] = process['burst_count']
            process['io_wait'] = 0
            process['arrived'] = False
            process['done'] = False
            processes.append(process)

    f.closed
    return processes

# # # #

# A process is defined as a program in execution - processes are in one of the following states:
READY = 'READY' # READY - in the ready queue, ready to use CPU
RUNNING = 'RUNNING' # RUNNING - actively using the CPU
BLOCKED = 'BLOCKED' # BLOCKED - blocked on I/O

# Processes in the READY state reside in a simple queue called the READY QUEUE.
# This queue is based on a configurable CPI scheduling algorithm.
READY_QUEUE = []

# # # #

# SIMULATION CONFIGURATION

# Define n as the number of processes to simulate
n = 10

# Define t_cs as the time in milliseconds tahat it takes to perform a context switch
# Use a default value of 8
t_cs = 8

# NOTE - a context switch occurs each time a process leaves the CPU is replaced by another process. The first half of the context switch time (half of t_cs) is the time required to remove the given process from the CPU - the second hald of the context switch time is the time required to bring the next process in to use the CPI.

# RR Algorithm
# Define the time slice value, measured in milliseconds
# Default value of 80
t_slice = 200

# # # # #

def first_come_first_served(processes, t_cs):
    elementsInList = getElementsInList([])
    print("time 0ms: Simulator started for FCFS " + elementsInList)

    Q = []
    currTime = 0
    Q = addElementsToQ(Q, processes, currTime)

    #Continually run each process then check to see if new processes need to be added to the Q

    #done is a boolean flag that tells whether all the processes in the dict have finished or not -> primarily used if there is a gap between a set of processes finishing and a new one arriving (would make the Q empty temporarily but doesn't mean that all processes have finished)
    done = False

    while(not done):
        Q = addElementsToQ(Q, processes, currTime)
        while(Q):
            currProcess = Q.pop(0)
            # <proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>

            #wait time is amount of time in ready queue
            #curr time - arrival time (time when put in ready queue)
            waitTime = currTime - processes[currProcess][0]
            processes[currProcess][5] += waitTime

            burstTime = processes[currProcess][1]
            ioTime = processes[currProcess][3]
            processes[currProcess][0] = currTime + burstTime + ioTime + t_cs
            processes[currProcess][2] -= 1
            processes[currProcess][4] += 1

            finTime = currTime + t_cs / 2
            while(currTime < finTime):
                Q = addElementsToQ(Q, processes, currTime)
                currTime += 1



            elementsInList = getElementsInList(Q)
            print("time " + str(int(currTime)) + "ms: Process " + currProcess + " started using the CPU " + elementsInList)




            finTime = currTime + burstTime
            while(currTime < finTime):
                Q = addElementsToQ(Q, processes, currTime)
                currTime += 1
            elementsInList = getElementsInList(Q)

            if(processes[currProcess][2] == 0):
                print("time " + str(int(currTime)) + "ms: Process " + currProcess + " terminated " + elementsInList)
            else:
                print("time " + str(int(currTime)) + "ms: Process " + currProcess + " completed a CPU burst; " + str(processes[currProcess][2]) + (" burst" if (processes[currProcess][2] == 1) else " bursts") + " to go " + elementsInList)
                print("time " + str(int(currTime)) + "ms: Process " + currProcess + " switching out of CPU; will block on I/O until time " + str(int(currTime + ioTime + t_cs/2)) + "ms " + elementsInList)

            finTime = currTime + t_cs / 2
            while(currTime < finTime):
                Q = addElementsToQ(Q, processes, currTime)
                currTime += 1

        done = checkIfAllProcessesDone(processes)

        if(not done):
            currTime += 1

    print("time " + str(int(currTime)) + "ms: Simulator ended for FCFS")

    #for FCFS # context switches = total # of processes
    cSwitches = 0

    #take the average of all the wanted info
    avgBurst = 0
    avgWait = 0

    for burst in processes.values():
        avgBurst += burst[1] * burst[4]
        avgWait += burst[5]
        cSwitches += burst[4]

    avgBurst /= cSwitches
    avgWait /= cSwitches

    #turnaround time = cpu burst time + wait time + t_cs
    avgTT = avgBurst + avgWait + t_cs

    # Puts statistics into result dict
    result = newResult()
    result['name'] = 'FCFS'
    result['avg_cpu_burst'] = round(avgBurst,2)
    result['avg_wait'] = round(avgWait,2)
    result['avg_turnaround'] = round(avgTT,2)
    result['total_ctx_switch'] = cSwitches
    result['total_preemptions'] = 0
    return result

# # # # #

# Shortest Remaining Time Algorithm
def shortest_remaining_time(processes, t_cs):
    elementsInList = getElementsInList([])
    print("time 0ms: Simulator started for SRT " + elementsInList)
    
    numPreemptions = 0
    contextSwitches = 0
    
    Q = []
    currTime = 0
    Q = addElementsToQSRT(Q, processes, currTime, None, t_cs)
    
    #Continually run each process then check to see if new processes need to be added to the Q
    
    #done is a boolean flag that tells whether all the processes in the dict have finished or not -> primarily used if there is a gap between a set of processes finishing and a new one arriving (would make the Q empty temporarily but doesn't mean that all processes have finished)
    done = False
    
    while(not done):
        Q = addElementsToQSRT(Q, processes, currTime, None, t_cs)
        while(Q):
            contextSwitches += 1
        
            currProcess = Q.pop(0)
            # <proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>

            #wait time is amount of time in ready queue
            #curr time - arrival time (time when put in ready queue)
            waitTime = currTime - processes[currProcess][0]
            processes[currProcess][5] += waitTime        
        
            burstTime = processes[currProcess][6]
            ioTime = processes[currProcess][3]
            processes[currProcess][0] = currTime + burstTime + ioTime + t_cs
        
            #time for context switching
            finTime = currTime + t_cs / 2
            while(currTime < finTime):
                Q = addElementsToQSRT(Q, processes, currTime, currProcess, t_cs)     
                currTime += 1
                
            elementsInList = getElementsInList(Q)
            if(processes[currProcess][1] != processes[currProcess][6]):
                print("time " + str(int(currTime)) + "ms: Process " + currProcess + " started using the CPU with " + str(processes[currProcess][6]) + "ms remaining "+ elementsInList)
            else:
                print("time " + str(int(currTime)) + "ms: Process " + currProcess + " started using the CPU " + elementsInList)
              
            #cpu burst time
            finTime = currTime + burstTime            
            newProcess = False
            while(currTime < finTime):
                Q = addElementsToQSRT(Q, processes, currTime, currProcess, t_cs)
                if(len(Q) > 0 and processes[Q[0]][6] + t_cs < processes[currProcess][6]):
                    processes[currProcess][0] = currTime + t_cs/2
                    processes[Q[0]][0] += t_cs/2
                    newProcess = True
                    numPreemptions += 1
                    break
                
                currTime += 1
                processes[currProcess][6] -= 1
        
            if(not newProcess): 
                #reset the tracker for time left in CPU burst
                processes[currProcess][6] = processes[currProcess][1] 
                
                processes[currProcess][2] -= 1
                processes[currProcess][4] += 1
                
                turnaroundTime = currTime + t_cs/2 - processes[currProcess][7]
                processes[currProcess][8] += turnaroundTime
                processes[currProcess][7] = currTime + ioTime + t_cs/2
                    
                elementsInList = getElementsInList(Q)            
                
                if(processes[currProcess][2] == 0):
                    print("time " + str(int(currTime)) + "ms: Process " + currProcess + " terminated " + elementsInList)
                else:
                    if(processes[currProcess][2] > 1):
                        print("time " + str(int(currTime)) + "ms: Process " + currProcess + " completed a CPU burst; " + str(processes[currProcess][2]) + " bursts to go " + elementsInList)
                    else:
                        print("time " + str(int(currTime)) + "ms: Process " + currProcess + " completed a CPU burst; 1 burst to go " + elementsInList)
                    print("time " + str(int(currTime)) + "ms: Process " + currProcess + " switching out of CPU; will block on I/O until time " + str(int(currTime + ioTime + t_cs/2)) + "ms " + elementsInList)             
            
            #time for context switching
            finTime = currTime + t_cs / 2
            while(currTime < finTime):
                Q = addElementsToQSRT(Q, processes, currTime, None, t_cs)                    
                currTime += 1
            Q = addElementsToQSRT(Q, processes, currTime, None, t_cs)
                
        done = checkIfAllProcessesDone(processes)
        
        if(not done):
            currTime += 1

    print("time " + str(int(currTime)) + "ms: Simulator ended for SRT")
    
    #print("\n\n--------STATISTICS-------\n\n")
    #print(processes)
    #for FCFS # context switches = total # of processes
    numProcesses = 0
    
    #take the average of all the wanted info
    avgBurst = 0
    avgWait = 0
    avgTT = 0
    #print (processes)
    for burst in processes.values():
        avgBurst += burst[1] * burst[4]
        avgWait += burst[5]
        avgTT += burst[8]
        numProcesses += burst[4]
    
    avgBurst /= numProcesses     
    avgWait /= numProcesses
    avgTT /= numProcesses
    
    '''print("-- average CPU burst time: " + str(round(avgBurst,2)) + " ms\n")   
    print("-- average wait time: " + str(round(avgWait,2)) + " ms\n")   
    print("-- average turnaround time: " + str(round(avgTT,2)) + " ms\n")   
    print("-- total number of context switches: " + str(contextSwitches) + "\n")   
    #FCFS does not have preemptions
    print("-- total number of preemptions: " + str(numPreemptions))
    '''
    # # # # #
    # Calculate statistics

    #--------------------------------------------------------------#
    

    #for p in processes:
    #    burstCount = burstCount + p['burst_count']
    #    burstTotal = burstTotal + (p['burst_count'] * p['burst_time'])
    #    waitTotal = waitTotal + p['io_time']

    # Calculates averages
    #avg_burst = round((burstTotal / burstCount), 2)
    #total_ctx_switch = int(burstCount + result['total_preemptions'])
    #avg_wait = waitTotal / total_ctx_switch

    # Puts statistics into result dict
    result = newResult()
    result['name'] = 'SRT'
    result['avg_cpu_burst'] = avgBurst
    result['total_ctx_switch'] = contextSwitches
    result['avg_wait'] = avgWait
    result['avg_turnaround'] = avgTT
    result['total_preemptions'] = int(numPreemptions)
    return result

# # # # #

def round_robin(processes, t_cs=8, t_slice=80, rr_add="END"):
    elementsInList = getElementsInList([])
    print("time 0ms: Simulator started for RR " + elementsInList)

    Q = []
    currTime = 0
    Q = addElementsToQRR(Q, processes, currTime, rr_add)

    total_preemptions = 0

    #Continually run each process then check to see if new processes need to be added to the Q

    #done is a boolean flag that tells whether all the processes in the dict have finished or not -> primarily used if there is a gap between a set of processes finishing and a new one arriving (would make the Q empty temporarily but doesn't mean that all processes have finished)
    done = False

    # <proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>
    #  0         0                      1                2            3

    while(not done):
        Q = addElementsToQRR(Q, processes, currTime, rr_add)
        while(Q):

            startTime = currTime
            currProcess = Q.pop(0)
            processes[currProcess][7] = currTime
            for i in range(int(t_cs/2)):
                Q = addElementsToQRR(Q, processes, currTime, rr_add, currProcess)
                currTime += 1


            #wait time is amount of time in ready queue
            #curr time - arrival time (time when put in ready queue)
            waitTime = startTime - processes[currProcess][0]
            processes[currProcess][5] += waitTime



            elementsInList = getElementsInList(Q)


            if (processes[currProcess][6] > 0):
                print(timeLog(currTime) + "Process " + currProcess + " started using the CPU with " + str(int(processes[currProcess][6])) + "ms remaining " + elementsInList)
                # burstTime = processes[currProcess][6]

            else:
                print(timeLog(currTime) + "Process " + currProcess + " started using the CPU " + elementsInList)
                # burstTime = processes[currProcess][1]

            nopreempt = True
            while (nopreempt):
                if (processes[currProcess][6] > 0):
                    burstTime = processes[currProcess][6]
                else:
                    burstTime = processes[currProcess][1]

                if (burstTime > t_slice):
                    finTime = currTime + t_slice
                    processes[currProcess][6] = burstTime - t_slice
                    processes[currProcess][0] = finTime + t_cs/2
                else:
                    nopreempt = False
                    finTime = currTime + burstTime
                    ioTime = processes[currProcess][3]
                    processes[currProcess][2] -= 1
                    processes[currProcess][4] += 1
                    processes[currProcess][0] = finTime + ioTime + t_cs/2
                    processes[currProcess][6] = 0

                while(currTime < finTime):
                    Q = addElementsToQRR(Q, processes, currTime, rr_add)
                    currTime += 1

                elementsInList = getElementsInList(Q)

                if (len(Q) == 0 and processes[currProcess][6] > 0):
                    # time 1446ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]
                    print(timeLog(currTime) + "Time slice expired; no preemption because ready queue is empty " + elementsInList)

                else:
                    nopreempt = False
                    # if (processes[currProcess][]):
                    #     Q.append(currProcess)


            if(processes[currProcess][2] == 0):
                print(timeLog(currTime) + "Process " + currProcess + " terminated " + elementsInList)
                
                turnaroundTime = currTime + t_cs/2 - processes[currProcess][9]
                processes[currProcess][10] += turnaroundTime
                processes[currProcess][9] = currTime + ioTime + t_cs/2
                
                # turnaround = currTime - processes[currProcess][7]
                # processes[currProcess][8].append(turnaround)
            elif(processes[currProcess][6] > 0):
                # time 172ms: Time slice expired; process B preempted with 305ms to go [Q A]
                print(timeLog(currTime) + "Time slice expired; process " + currProcess + " preempted with " + str(int(processes[currProcess][6])) + "ms to go " + elementsInList)
                total_preemptions += 1
                # Q.append(currProcess)
            else:
            
                turnaroundTime = currTime + t_cs/2 - processes[currProcess][9]
                processes[currProcess][10] += turnaroundTime
                processes[currProcess][9] = currTime + ioTime + t_cs/2
            
                print(timeLog(currTime) + "Process " + currProcess + " completed a CPU burst; " + str(processes[currProcess][2]) + (" burst" if (processes[currProcess][2] == 1) else " bursts") + " to go " + elementsInList)
                print(timeLog(currTime) + "Process " + currProcess + " switching out of CPU; will block on I/O until time " + str(int(currTime + ioTime + t_cs/2)) + "ms " + elementsInList)
                # turnaround = currTime - processes[currProcess][7]
                # processes[currProcess][8].append(turnaround)



            # currTime += t_cs/2
            for i in range(int(t_cs/2)):
                Q = addElementsToQRR(Q, processes, currTime, rr_add)
                currTime += 1

            turnaround = currTime - processes[currProcess][7]
            processes[currProcess][8].append(turnaround)


        done = checkIfAllProcessesDone(processes)

        if(not done):
            currTime += 1

    print(timeLog(currTime) + "Simulator ended for RR")

    #for FCFS # context switches = total # of processes
    cSwitches = 0.0

    #take the average of all the wanted info
    avgBurst = 0.0
    avgWait = 0.0
    avgTT = 0.0
    for burst in processes.values():
        avgBurst += float(burst[1] * burst[4])
        avgWait += float(burst[5])
        avgTT += burst[10]
        cSwitches += float(burst[4])
        # print burst[8] 
        # avgTT += float(sum(burst[8])) / float(len(burst[8]))

    avgBurst /= cSwitches
    avgWait /= cSwitches
    avgTT /= cSwitches
    #avgTT /= float(len(processes))

    #turnaround time = cpu burst time + wait time + t_cs
    #avgTT = avgBurst + avgWait + (((float(t_cs/2)) * (total_preemptions-len(processes))) / float(len(processes.values())))
    # avgTT = (((float(t_cs)) * total_preemptions) / float(len(processes.values())))
    # avgTT = 
    # avgTT = avgBurst + avgWait + t_cs

    # Puts statistics into result dict
    result = newResult()
    result['name'] = 'RR'
    result['avg_cpu_burst'] = round(avgBurst,2)
    result['avg_wait'] = round(avgWait,2)
    result['avg_turnaround'] = round(avgTT,2)
    result['total_ctx_switch'] = cSwitches + total_preemptions
    result['total_preemptions'] = int(total_preemptions)
    return result

# # # # #

# COMMAND LINE ARGUMENTS
# python project1.py <input-file> <stats-output-file> [<rr-add>]

# Handle arguments
ARGV = sys.argv
input_file = ARGV[1]
output_file = ARGV[2]

if (len(ARGV) > 3):
    rr_add = ARGV[3]
else:
    rr_add = "END"

# TODO - detect missing arguments, display an error message as follows on stderr:
#   ERROR: Invalid arguments
#   USAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]

# TODO - detect an error in the input file format, display an error message as follows on stderr:
#   ERROR: Invalid input file format

# Parses processes from input file
READY_QUEUE = parseInput(input_file)

# Store the results of each scheduling algorithm
results = []
results.append(first_come_first_served(get_fcfs_processes(input_file), t_cs))
print()
results.append(shortest_remaining_time(get_srt_processes(input_file), t_cs))
#results.append(shortest_remaining_time(READY_QUEUE, t_cs))
print()
results.append(round_robin(get_rr_processes(input_file), t_cs, rr_add=rr_add))

# # # #

# Writes process result output to file
# TODO - must write output to file
writeOutput(output_file, results)