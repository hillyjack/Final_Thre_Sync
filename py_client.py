#!/usr/bin/env python

def sig_handler(a,b):
    global run
    global index
    global last_index

    run = False
    if(index == last_index):
        index = 0
    else:
        index += 1

    print >> sys.stderr, 'moving to file- ', file_list[index]


import socket
import sys
import os
import signal

index = 0
run = False
last_index = 0
basePath = "../"

#Handle signal
signal.signal(signal.SIGUSR1, sig_handler)

#create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 2006)
print >>sys.stderr, 'connecting to %s port %s' % server_address
client_socket.connect(server_address)
client_socket.send(str(os.getpid()))

print >>sys.stderr, 'waiting for data'
data = client_socket.recv(512)
client_socket.close()
print >>sys.stderr, 'got it!'

if (data):
    print >>sys.stderr, data, "\n"
    file_list = data.split(",")
    last_index = len(file_list) - 1
else:
    print >> sys.stderr, "no data"
    exit()

while 1:
    current_file = file_list[index]
    fullPath = os.path.join(basePath, current_file)
    run = True

    for line in open(fullPath):
        if run:
            line = line.rstrip("\n")
            print line
            sys.stdout.flush()
        else:
            break

    data = None