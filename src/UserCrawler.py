import urllib
import urllib2
import socket
import sys
import zlib
from bs4 import BeautifulSoup

class BattleUrl(object): 
    def __init__(self, url, fmt): 
        self.url = url
        self.fmt = fmt

class UserCrawler(object):
    def __init__(self, socketName, socketName2):
        self._headers = self.buildHeaders()
        self._socketName = socketName
        self._socketName2 = socketName2

    # sends a request to the url and requests resources
    def crawl(self, url, headers, body): 
        battleUrls = []
        userNames = [] 
        errcode = 1

        try:
            req = urllib2.Request(url, data=body, headers=headers)
            print (body)
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
  
        except zlib.error as err: 
            print "Caught zlib error here!"            
            print "Todo: send error response to URL server and DON'T flush that url / user"
            errcode = -1
        except Exception as err: 
            # do something interesting here
            print "Caught unexpected exception"
            print (err)
            errcode = -2
             
        return userNames, battleUrls, errcode 

    # requests a url from the URLServer, stuff like that
    # TODO: server should send over url and user names.  This way we can reduce the cost of 
    # read/write by half 
    def request(self):
        # watch out for some interesting names (blaze dude)
        try: 
            # connect and handle transmission with User Server
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            print (self._socketName)
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
                    # print (chunk) 
                    sock.send(chunk)

                # don't need to chunk it as long as the battleUrl is fewer than 1024 bytes 
                # (or whatever size it is on the UrlServer2 end)
                # for battleUrl in battleUrls: 
                #    battleUrl.url += '\0\n'
                #    sock2.send(battleUrl.url)
                #    print (battleUrl.url)
                request = ""
                for battleUrl in battleUrls: 
                    request += battleUrl.url
 
                chunksBattle = self.chunkRequest(request, 1024) 
                
                for chunkBattle in chunksBattle: 
                    sock2.send(chunk) 

            sock.close()
            sock2.close()
                            
        except OSError as err: 
            print (err)

    def chunkRequest(self, request, chunkSize): 
        # constants
        requestLen = len(request)
        # non-constants
        chunks = []
        c = ""
        currentSize = 0

        for i in range(requestLen + 1): 
            if currentSize > chunkSize: 
                print ("something went wrong")
                sys.exit(-1)

            if currentSize == chunkSize:
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
        while True:
            url = self.request()

# test client to request for url here with socket
if __name__ == "__main__": 
    # fork a bunch of crawlers here later 
    # TODO: better arguemnt messange 
   
    # socketName: refers to Username Server
    # socketName2: refers to Battle log url Server 
    socketName = sys.argv[1]
    socketName2 = sys.argv[2]

    wc = UserCrawler(socketName, socketName2)
    wc.retrieve()
