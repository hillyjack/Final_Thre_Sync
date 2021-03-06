#!/usr/bin/env python

import socket
import sys
import os
import signal
from time import sleep

def sig_handler(a,b):
    global run
    global index
    global last_index

    run = False
    if(index == last_index):
        index = 0
    else:
        index += 1

    print >> sys.stderr, 'moving to file-', file_list[index]

index = 0
run = False
last_index = 0
# basePath = "../questions-files/"      # IDE
basePath = "./questions-files/"        # CLI

#Handle signal
signal.signal(signal.SIGUSR1, sig_handler)

#create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 2000)
print >>sys.stderr, "\n", 'connecting to %s port %s' % server_address
client_socket.connect(server_address)
client_socket.send(str(os.getpid()))

print >>sys.stderr, 'waiting for data'
data = client_socket.recv(512)
client_socket.close()

if (data):
    print >>sys.stderr,"data- ", data, "\n"
    file_list = data.split(",")
    last_index = len(file_list) - 1
else:
    print >> sys.stderr, "no data"
    exit()

while 1:
    current_file = file_list[index]
    fullPath = os.path.join(basePath, current_file)
    object = open(fullPath)
    run = True

    for line in object:
        if run:
            sleep(2)
            line = line.rstrip("\n")
            # print >> sys.stderr, 'Python line- ', line
            try:
                print line
                sys.stdout.flush()
            except IOError:
                pass
        else:
            object.close()
            break
