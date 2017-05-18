import socket
import sys
import os
import time
import struct
def send(argv):
	s= socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	port = 8000
	filename = argv
	filesize = struct.pack("<Q", os.path.getsize(argv))	
	
	s.connect(("localhost",port))
	f=open(argv,'rb')  	
#	print 'sending filesize..'
	s.send(filesize)	
#	print 'sending...'
	
	
	t= 1
	while t:	
		t = f.read(1024)		
		s.send(t)	
	f.close()

	print 'Done sending'
	print 'Waiting the file'
	return s
	
def recv(s):
	
	filename = sys.argv[1] + '.gpg'
	g = open(filename,'wb')
	if( s.recv(5) == "XXXXX"):
		print("file is not verified. Please sign first.")
		s.close()
		return
	
	
	try:
		filesize = struct.unpack("<Q",s.recv(8))[0]			
		while (filesize > 0 ):
			if(filesize  < 1025):
				chunk = s.recv(filesize)
				g.write(chunk)
				break	
			chunk = s.recv(1024)
			g.write(chunk)
			filesize -= 1024
			
	except:
		print('error occured')
	print 'Done'
	g.close()
	s.close()
	
def main(argv):
	s = send(argv[1])
	recv(s)
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
