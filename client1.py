import socket
import sys
import os
import time
import struct
import cPickle
import gnupg

def send(argv):
	s= socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	port = 8000
	gitid = argv
	
	s.connect(("127.0.0.1",port))
	test = s.recv(128)
	print test
	s.send(gitid)	

	print 'Done sending'
	return s


def recv(s):
	data = s.recv(2048)
#data = cPickle.loads(r)
	print data

	gpg = gnupg.GPG()
	filename = sys.argv[1]+'.key'
	filepath = os.listdir('./keys/')
	key_data = open('keys/notary.pub').read()#sever public key
	import_result = gpg.import_keys(key_data)
	ver = import_result.fingerprints[0].encode('ascii')
	key_data = open('./keys/'+filename).read()
	import_result = gpg.import_keys(key_data)
	dec = import_result.fingerprints[0].encode('ascii')
	print dec
	decrypted_data = gpg.decrypt(data, passphrase='[client private key pass]')
	print decrypted_data.data
	
	key_data = open('keys/notary.pub').read()
	import_result = gpg.import_keys(key_data)
	ver = import_result.fingerprints[0].encode('ascii')
	authdata = gpg.encrypt(str(decrypted_data.data), ver, always_trust = True)
	print authdata.ok
	s.sendall(str(authdata))
	print 'check'
	#print ("decrypted ok : " + decrypted_data.ok)
	s.close()
	
def main(argv):
	s = send(argv[1])	
	recv(s)
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
