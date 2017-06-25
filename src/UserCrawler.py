import urllib
import urllib2
import socket
import sys
import errno
import zlib
import logging
import os
import time
from bs4 import BeautifulSoup

class BattleUrl(object): 
    def __init__(self, url, fmt): 
        self.url = url
        self.fmt = fmt

class UserCrawler(object):
    def __init__(self, socketName, socketName2, logger):
        self._headers = self.buildHeaders()
        self._socketName = socketName
        self._socketName2 = socketName2
        self._logger = logger

    # sends a request to the url and requests resources
    def crawl(self, url, headers, body): 
        battleUrls = []
        userNames = [] 
        errcode = 1

        try:
            req = urllib2.Request(url, data=body, headers=headers)
            response = urllib2.urlopen(req)

            content = zlib.decompress(response.read(), 16+zlib.MAX_WBITS)
            
            # we don't want to go TOO fast, so we can slow down a bit 
            # and parse the document here        
            soup = BeautifulSoup(content, 'html.parser')
            lis = soup.find_all('li') 
            
            for li in lis: 
                # take a risk and parse game info, user names, and such with the assumption
                # that we know all information
                battleLink = li.a['href']
                battleFormat = li.small.contents[0]
                player1 = li.strong.contents[0]
                player2 = li.strong.next_sibling.next_sibling.contents[0]
        
                userNames.append(player1) 
                userNames.append(player2)
            
                burl = BattleUrl(battleLink, battleFormat) 
                battleUrls.append(burl)

                self._logger.info("Battle details: " + player1 + " " + player2 + " " + battleLink + " " + battleFormat)
  
        except zlib.error as err: 
            self._logger.exception("Caught zlib error here!")
            errcode = -1
        # except Exception as err: 
        #     # do something interesting here
        #     print "Caught unexpected exception"
        #     print (err)
        #     errcode = -2
             
        return userNames, battleUrls, errcode 

    # requests a url from the URLServer, stuff like that
    # TODO: server should send over url and user names.  This way we can reduce the cost of 
    # read/write by half 
    def request(self):
        sock = None
        sock2 = None 
 
        # watch out for some interesting names (blaze dude)
        try: 
            # connect and handle transmission with User Server
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(self._socketName)

            # now handle transmission with url server
            sock2 = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock2.connect(self._socketName2)

            requestSignal = "req\0" 
            sock.sendall(requestSignal)
            usr = urllib.unquote(sock.recv(1024))

            for i in range(1, 11, 1):
                # add the request body  
                url = "http://replay.pokemonshowdown.com/search/"
                mappings= { 'output': 'html', 'user': usr, 'page': str(i) }
                body = urllib.urlencode(mappings)
 
                newUsers, battleUrls, errcode = self.crawl(url, self._headers, body) 
              
                if errcode == -1: 
                    break 

                # there might be some unicode
                # url server can't handle that, so we quote the request 
                request = "" 
                for newUser in newUsers: 
                    request += urllib.quote(newUser.encode('utf8')) + '\n'
   
                chunks = self.chunkRequest(request, 1024)

                for chunk in chunks:
                    sock.send(chunk)

                request = ""
                # for battleUrl in battleUrls: 
                   # request += battleUrl.url[1:] + '\n'

                # chunksBattle = self.chunkRequest(request, 1024) 
                # for chunkBattle in chunksBattle
                for battleUrl in battleUrls:
                    sock2.send(battleUrl.url[1:] + '\n')


        except OSError as err: 
            self._logger.exception(err)
        except socket.error as err: 
            if sock is not None: 
                sock.close()
            if sock2 is not None: 
                sock2.close() 
            if isinstance(err.args, tuple): 
                print ("[errno %d]" % err[0])
                if err[0] == errno.EPIPE: 
                    print ("disconnected socket")
                    sys.exit(-1)
                else:
                    self._logger.exception(err)
                    sys.exit(-1)
            else: 
                print ("socket err ", err)
        except Exception as err: 
            self._logger.exception(err)
        finally: 
            if sock is not None: 
                sock.close()
            if sock2 is not None: 
                sock2.close()

    def chunkRequest(self, request, chunkSize): 
        # constants
        requestLen = len(request)
        # non-constants
        chunks = []
        c = ""
        currentSize = 0

        for i in range(requestLen + 1): 
            if currentSize > chunkSize: 
                self._logger.error("Something went wrong.  Chunk size too big")

            if currentSize == chunkSize - 10: # keep some buffer at the end 
                c += '\0'
                chunks.append(c)
                c = ""
                currentSize = 0

            if i == requestLen: 
                c += '\0' 
                chunks.append(c)
                break

            c += request[i]
            currentSize += 1

        return chunks 

    def buildHeaders(self): 
        headers = {
            "User-Agent": "TYangBot",
            "Accept-Encoding": "gzip, deflate, sdch"
        }

        return headers

    def retrieve(self):
        count = 0
        # sleep every 20 requests to prevent ddos and let server catch up
        while True:
            if count == 20: 
                time.sleep(40)
                count = 0
            url = self.request()

# test client to request for url here with socket
if __name__ == "__main__": 
    pid = os.getpid()
    logger = logging.getLogger("UserCrawler" + str(pid))
    logger.setLevel(logging.INFO)    

    fh = logging.FileHandler("UserCrawler" + str(pid) + ".log")
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)

    # fork a bunch of crawlers here later 
    # TODO: better arguemnt messange 
   
    # socketName: refers to Username Server
    # socketName2: refers to Battle log url Server 
    socketName = sys.argv[1]
    socketName2 = sys.argv[2]

    logger.info("UrlServlet socketname: " + socketName)
    logger.info("UrlServlet socketname2: " + socketName2)

    wc = UserCrawler(socketName, socketName2, logger)
    wc.retrieve()

