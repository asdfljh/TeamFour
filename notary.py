import gnupg
import magic
import os
import sys
import socket
import logging
import base64

def import_keys(argv):
	KEYPATH = argv	
	gpg = gnupg.GPG(gnupghome = KEYPATH)	
	filenames = os.listdir(KEYPATH)	
	for filename in filenames:
		fullname = os.path.join(KEYPATH,filename)
		ext = os.path.splitext(fullname)[-1]
		
		'''
		 .pub is public key, and .key is secret key
		 Please be aware if you import secret key first,
		 you should type passphrase of notary key.
		 If you want to get it, Please inquire to KAISTGUN
		'''
		
		if ext == '.pub' or ext == '.key': 
			try:
				key_data = open(KEYPATH + filename).read()
				gpg.import_keys(key_data)
			except IOError:
				logging.exception("file_error")
				print "No such file or directory, usage : python notary.py <Key directory>"
				sys.exit(1)
	return gpg

def init_server():

	try:
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	except socket.error, e:
		logging.exception("Socket error")
		return -1
		
	try:
		s.bind(("",8000))
	except socket.error, e:
		logging.exception("Bind error")
		return -1;
	
	return s
	
def get_files(c,sock):	
	try:
		f = open('temp','wb')  		
		#while (t):
		t = c.recv(1024)
		f.write(t)
		#	t= c.recv(1024)		
		logging.info('File transmission finished')
	except IOError:
		logging.exception("Open error")		
		sock.close()
		f = -1
	except:
		logging.exception("Unexpected error, file transmission failed")
		sock.close()		
		f = -1
	f.close()
	return f
	
def verify(sock,gpg):
	f = open('temp','rb')	
	verified = gpg.verify_file(f)
	Success = -1
	
	if not verified:
		#raise ValueError("Signature could not be verified!")		
		sock.close()
	else:
		Success = 1
	f.close()
	return Success
	
	
def send_file_with_sign(s_sock, gpg):
	s_file = open('temp','rb') 
	signed_file = gpg.sign_file(s_file) #gpg is not global
	stream = s_file.read(1024)
	Success = -1
	
	try:
		while stream:		
			s_sock.send(stream)
			stream = s_file.read(1024)
			
	except:
		logging.exception("Unexpected error, file transmission failed")
		s_sock.close()
		return Success
		
	#s_sock.send("Your file with sign has been successfully transfered!")
	s_sock.close()
	Success = 1
	
	return Success
	
def send_executable(s_file,IP):	

	Host = 1     # Designated local host
	Port = 8001

	# Check the head of a file
	if (magic.from_buffer(s_file.read())[:3] != 'ELF' ):
		return -1		
	# Get the time
	timestring = time.ctime(time.time())
	'''
	Please Following JSON format
		\begin{lstlisting}
	{
	"name": "name of the program",
	"body": "base64 encoded program executable"
	}
		\end{lstlisting}
	'''	
	NAME = "FROM_"+timestring+"IP_"+IP	
	file_b64encode = base64.b64encode(s_file.read())
	TEXT = "{\n\"name\": \"" + NAME + "\",\n\"body\": \"" + file_b64encode + "\"\n}"
	
	try:
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		s.connect((host, port))
		
	except socket.error, e:
		logging.exception("Socket error")
		return -1	
	
	f = open(NAME,'wb')
	f.write(TEXT)
	stream = f.read(1024)
	
	try:
		while stream:		
			s.send(stream)
			stream = f.read(1024)			
	except:
		logging.exception("Unexpected error, file transmission failed")
		return -1
		
	s.send("Your file with sign has been successfully transfered!")
	s_sock.close()	
	
	return 1
	
def main(argv):
	# We will run this program with sudo command.	
	logging.basicConfig(filename='/var/log/notary/test.log',filemode='w',level=logging.DEBUG)
	
	gpg = import_keys(argv[1]) 			
		
	while True:
		sock = init_server()
		
		if sock == -1 : 	  # get a sock object
			logging.error('Cannot init server')
			sys.exit(1)
			
		sock.listen(1)
		print 'Server is Listening...'
		connect, address = sock.accept()	
		logging.info('Connetion established with %s', address)
		print 'Connect from', address
		
		f = get_files(connect,sock)
		
		if f < 0 :
			continue
		print 'Get a file'
		
		if verify(sock,gpg) < 0:
			continue
		print 'Done verify'
		if send_file_with_sign(sock, gpg) < 0:
			continue
		print 'Sign and Send'
		#if file_is_executable(f,address) < 0:
		#	continue
			
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
