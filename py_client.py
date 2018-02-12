#!/usr/bin/env python

import socket
import sys

#create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 2000)
print >>sys.stderr, 'connecting to %s port %s' % server_address
client_socket.connect(server_address)

while 1:
    data = client_socket.recv(512)
    if (data):
        file_list = data.split(",")
        current_file = file_list[0]

        for line in open(current_file):
            print (line)
