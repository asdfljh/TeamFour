import gnupg
import os
import sys
import socket
import logging
from pprint import pprint
def import_keys(argv):
	KEYPATH = argv	
	gpg = gnupg.GPG(gnupghome = KEYPATH)	
	filenames = os.listdir(KEYPATH)	
	for filename in filenames:
		fullname = os.path.join(KEYPATH,filename)
		ext = os.path.splitext(fullname)[-1]
		if ext == '.pub':
			try:
				key_data = open(KEYPATH + filename).read()
				gpg.import_keys(key_data)
			except IOError:
				logging.exception("file_error")
				print "No such file or directory, usage : python notary.py <Key directory>"
				sys.exit(1)
	#pprint (gpg.list_keys())
def init_server():
	try:
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	except socket.error, e:
		logging.exception("Socket error")
		return 0
	try:
		s.bind(("",8000))
	except socket.error, e:
		logging.exception("Bind error")
		return 0;
	s.listen(7)
	print "Listening..."
	return s
	
def get_files(c):
	try:
		f = open('temp','wb')  
		t = c.recv(1024)
		while (t):
			f.write(t)
			t= c.recv(1024)
		f.close()
		logging.info('File transmission finishes')
	except IOError:
		logging.exception("Open error")
		return 0
	except:
		logging.exception("Unexpected error")
		return 0
	return f
	
def verify(file_stream,sock):
	verified = gpg.verify_file(stream)
	if not verified:
		raise ValueError("Signature could not be verified!")		
		sock.close()		
	return verified
	
def log_all(error_string):	
	return
	
def send_file_with_sign(s_sock, s_file, s_sign):
	signed_file = gpg.sign_file(s_file) #gpg is not global
	stream = s_file.read(1024)
	while stream:		
		s_sock.send(stream)
		stream = s_file.read(1024)
	s.send("Your file with sign has been successfully transfered!")
	s_sock.close()
	return
	
def main(argv):
	logging.basicConfig(filename='/var/log/notary/test.log',filemode='w',level=logging.DEBUG)
	#import_keys(argv[1]) 
	sock = init_server()	  # get a sock object
	if sock==0 :
		sys.exit(1)
	connect, address = sock.accept()
	logging.info('Connetion established with %s', address)
	print 'Connect from', address
	f = get_files(connect)
	if f==0 :
		sys.exit(1)
	#if verify(f,sock):
		#send_sign_with_file (sock, f, sign)	
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>