import socket
import sys
import os
import time
import struct
import gnupg

def initNet(argv):
	s= socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	port = 8000
        gitid = argv
        s.connect(("127.0.0.1",port))
	test = s.recv(128)
	print test
	s.send(gitid)   

	print 'Done sending'
	return s

def auth(s):
	data = s.recv(2048)
#data = cPickle.loads(r)
	print data

	gpg = gnupg.GPG()
	filename = sys.argv[1]+'.key'
	filepath = os.listdir('[key directory]')
	key_data = open('[key directory]' +'/notary.pub').read()#sever public key
	import_result = gpg.import_keys(key_data)
	ver = import_result.fingerprints[0].encode('ascii')
	key_data = open('[key directory]'+filename).read()
	import_result = gpg.import_keys(key_data)
	dec = import_result.fingerprints[0].encode('ascii')
	print dec
	decrypted_data = gpg.decrypt(data, passphrase='[user private key pass]')
	print decrypted_data.data
					
	key_data = open('[key directory]'+'/notary.pub').read()
	import_result = gpg.import_keys(key_data)
	ver = import_result.fingerprints[0].encode('ascii')
	authdata = gpg.encrypt(str(decrypted_data.data), ver, always_trust = True)
	print authdata.ok
	s.sendall(str(authdata))
	print 'check'
	#print ("decrypted ok : " + decrypted_data.ok)
	s.close()
	return 1

def send(s,argv):
	filename = argv
	filesize = struct.pack("<Q", os.path.getsize(argv))	
	f=open(argv,'rb')  	
	s.send(filesize)
	print 'sending filesize..'	
	s.send(f.read())

#	t= 1
#	while t:	
#		t = f.read(1024)		
#		s.send(t)	
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
		g.write(s.recv(filesize))
		'''
		while (filesize > 0 ):
			if(filesize  < 1025):
				chunk = s.recv(filesize)
				g.write(chunk)
				break	
			chunk = s.recv(1024)
			g.write(chunk)
			filesize -= 1024
			'''
	except:
		print('error occured')
	print 'Done'
	g.close()
	s.close()
	
def main(argv):
	s = initNet(sys.argv[1])
	if 1 != auth(s) :
		print 'error: auth'
		s.close()
		sys.exit(0)
	else :
		print 'auth success'
	
	send(s,argv[1])
	recv(s)
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
