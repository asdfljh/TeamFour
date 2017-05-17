import socket
import sys

s= socket.socket()
port = 8000

s.connect(("127.0.0.1",port))
f=open(sys.argv[1],'rt')
print 'sending...'
l = f.read(1024)
while(l):
	print 'Sending..'
	s.send(l)
	l = f.read(1024)
f.close()

print 'Done sending'
print 'Waiting the file'

f = open('temp2','wb')  
t = s.recv(1024)
while (t):
	f.write(t)
	t= c.recv(1024)

f.close()

print 'Done'
s.close()
