import gnupg
import magic
import os
import sys
import socket
import logging
import logging.handlers
import base64
import struct
import random
import cPickle
from pprint import pprint

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
		'''
		
		if ext == '.pub' or ext == '.key': 
			try:
				key_data = open(KEYPATH + filename).read()
				gpg.import_keys(key_data)
			except IOError:
				logging.exception("file_error")
				print ("No such file or directory, usage : python notary.py <Key directory>")
				sys.exit(1)
	return gpg

def init_server():

	try:
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	except (socket.error, e):
		logging.exception("Socket error")
		return -1
		
	try:
		s.bind(("",8000))
	except socket.error, e:
		logging.exception("Bind error")
		return -1;
	
	return s

def get_files(conn):

	try:
		logging.info('Receiving File..')		
		recv_file = open('temp','wb')		
		filesize = struct.unpack("<Q", conn.recv(8))[0]		
		
		
		while (filesize > 0 ):
			if(filesize  < 1025):
				chunk = conn.recv(filesize)
				recv_file.write(chunk)
				break
			chunk = conn.recv(1024)	
			recv_file.write(chunk)
			filesize -= 1024
		
		logging.info('File transmission finished')

	except IOError:
		logging.exception("Open error")		
		return -1

	except:
		logging.exception("Unexpected error, file transmission failed")		
		return -1

	recv_file.close()
	return 1
	
def verify(gpg,conn):
	file_verify = open('temp','rb')	
	verified = gpg.verify_file(file_verify)
	Success = -1
	
	if not verified:
		#raise ValueError("Signature could not be verified!")		
		#	sock.close()
		conn.send("XXXXX")
		file_verify.close()
		Success = -1
	else:
		conn.send("YYYYY")
		Success = 1

	file_verify.close()
	return Success
	
	
def send_file_with_sign(c, gpg):

	s_file = open('temp') 
	sign = gpg.sign_file(s_file,passphrase = "notary897",output = "temp.gpg")	

	sign_file = open('temp.gpg') # Signed file
	filesize = struct.pack("<Q", os.path.getsize('temp.gpg'))
	c.send(filesize)   # Send File size

	Success = -1
	stream = 1

	try:
		while stream:
			stream = sign_file.read(1024)
			c.send(stream)
			
	except:
		logging.exception("Unexpected error, file transmission failed")
		return Success
		
	#s_sock.send("Your file with sign has been successfully transfered!")
	#s_sock.close()
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
		
	except (socket.error, e):
		logging.exception("Socket error")
		return -1	
	
	f = open(NAME,'wb')
	f.write(TEXT)
	
	filesize = struct.pack("<Q", os.path.getsize(NAME))
	s.send(filesize)   # Send File size	
	
	stream = 1	
	try:
		while stream:		
			s.send(stream)
			stream = f.read(1024)			
	except:
		logging.exception("Unexpected error, file transmission failed")
		return -1
	
	
	return 1

def loggingconfig():

	# Initiate Logging handler
	logging.basicConfig(filename='/var/log/notary/test.log',filemode='w',level=logging.DEBUG)
	
	console = logging.StreamHandler()
	console.setLevel(logging.ERROR)
	
	# set a format which is simpler for console use
	formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
	console.setFormatter(formatter)
	# add the handler to the root logger
	logging.getLogger('').addHandler(console)
	logger = logging.getLogger(__name__)
	# Initiate FileHandler and Streamhandler
	fileHandler = logging.FileHandler('/var/log/notary/test.log')
	streamHandler = logging.StreamHandler()

def authuser(c,path):
	c.send('Send your github ID')
	gitid = c.recv(256) # github ID might be shorter than 256 bits	
	gitname = gitid + ".pub"
	gpgauth = gnupg.GPG() #important thing is key order
	filenames = os.listdir(path)
	if gitname in filenames:
		for filename in filenames:
			if gitname == filename : 
				key_data1 = open(path+gitname).read()
				import_result = gpgauth.import_keys(key_data1)
				public = import_result.fingerprints[0].encode('ascii')
			if filename == "notary.key" :
				key_data2 = open(path+filename).read()
				import_result = gpgauth.import_keys(key_data2)		
				private = import_result.fingerprints[0].encode('ascii')
	else :
		print ("There is no key for such id")
		return -1

	number = random.getrandbits(512)
	encrypted_data = gpgauth.encrypt(str(number), public, sign=private, passphrase = "notary897")
	#to_send = struct.pack('>Q',len(encrypted_data)) + encrypted_data
	c.sendall(str(encrypted_data))
	authrandom = c.recv(2048)

	if not authrandom :
		print "check"
		return -1
	
	print ("Received encrypted randomnumber")
	
	key_data2 = open(path+filename).read()
	import_result = gpgauth.import_keys(key_data2)
	verified = gpgauth.decrypt(authrandom, passphrase="notary897")
	
	if not verified.ok :
		print(verified.status)
		return -1
	if str(verified) == str(number) :
		return 0
	else :
	 	print("Random number isn't same")
	 	return -1


def errorInConnect(c,comment):
	logging.info('%s', comment)
	c.close()


def main(argv):
	# We will run this program with sudo command.	
	loggingconfig()
	#logging.basicConfig(filename='/var/log/notary/test.log',filemode='w',level=logging.DEBUG)
	#logging.basicConfig(level=logging.DEBUG)
	gpg = import_keys(argv[1]) 			
		
	
	sock = init_server()
	print ('Initiating Server...')
	if sock == -1 : 	  # get a sock object
		logging.error('Cannot init server')
		sys.exit(1)

	while True:		
		sock.listen(1)
		print ('Server is Listening...')

		connect, address = sock.accept()	
		logging.info('Connetion established with %s', address)
		print ('Connect from', address)
		auth = authuser(connect,argv[1])
		
		if auth < 0 :
			errorInConnect(connect,'Authenticaion fail')
			continue
		print ('Auth success')
	
		f = get_files(connect)
		
		if f < 0 :
			errorInConnect(connect,'Get File error')
			continue
		print ('Get a file')
		
		if verify(gpg,connect) < 0:
			errorInConnect(connect,'Verify error')
			continue
		print ('Done verify')

		if send_file_with_sign(connect, gpg) < 0:
			errorInConnect(connect,'Send file with sign error')
			continue
		print ('Sign and Send')

		#if file_is_executable(f,address) < 0:
		# We don't need to make this module. 
			
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
