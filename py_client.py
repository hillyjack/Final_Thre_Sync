#!/usr/bin/env python

import socket
import sys
import os

basePath = "/home/ubuntu/CLionProjects/Final_Thre_Sync"

#create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 2004)
print >>sys.stderr, 'connecting to %s port %s' % server_address
client_socket.connect(server_address)

while 1:
    print >>sys.stderr, 'waiting for data'
    data = client_socket.recv(512)
    #client_socket.close()
    print >>sys.stderr, 'got it!'
    if (data):
        print >>sys.stderr, data, "\n"
        file_list = data.split(",")
        fullPath = os.path.join(basePath,file_list[0])
        current_file = fullPath

        for line in open(current_file):
            #print >>sys.stderr, "sending line",ord(chr(len(line)))," ",line
            #line = line.rstrip("\n")
            #print chr(len(line)),
            print line
            sys.stdout.flush()