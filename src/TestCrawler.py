import urllib
import urllib2
import socket
import sys
import zlib
from bs4 import BeautifulSoup

class TestCrawler(object): 
    def __init__(self): 
        pass

    def requestUrl(self): 
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect("test.socket") 

        # test = "" 
        # for i in range(1024): 
        #    test += str(i) + " "

        # print (test)
        # test += '\0'
        # make sure we null terminate our strings!
        
        test = "hi\0" 
        sock.sendall(test)
        msg = sock.recv(1024)        
        print (msg)

        teststr = ""
        for i in range(2048): 
            teststr += str(i) + " "

        teststr += '\0'

#        print (teststr)

        # chuck the data
        chunks = []
        c = ""
        for i in range(len(teststr) + 1): 
            if len(c) > 1023: 
                print ("something went wrong")
                sys.exit(-1)

            if len(c) == 1023:
                c += '\0'
                chunks.append(c)
                c = ""

            if i == len(teststr): 
                c += '\0' 
                chunks.append(c)
                break

            c += teststr[i]
         
        for chunk in chunks:
            print (chunk) 
            sock.send(chunk)
            
        sock.close()


