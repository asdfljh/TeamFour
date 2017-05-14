import gnupg
import os
import sys
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
				log_all("file_error")
				print 'No such file or directory, usage : python notary.py <Key directory> + \'/\''		
				sys.exit(1)
	#pprint (gpg.list_keys())
def init_server():
	return
	
def get_files():			
	return
	
def verify(file_stream,sock):
	verified = gpg.verify_file(stream)
	if not verified:
		raise ValueError("Signature could not be verified!")		
		sock.close()		
	return verified
	
def log_all(error_string):	
	return
	
def send_file_with_sign(s_sock, s_file, s_sign):
	signed_file = gpg.sign_file(s_file)
	stream = s_file.read(1024)
	while stream:		
		s_sock.send(stream)
		stream = s_file.read(1024)
	s.send("Your file with sign has been successfully transfered!")
	s_sock.close()
	return
	
def main(argv):
	import_keys(argv[1]) 
	#sock = init_server()	  # get a sock object
	#file = get_files()	
	if (verifiy(file,sock))
		send_sign_with_file (sock, file, sign)	
	
if __name__ == "__main__":
    main(sys.argv)
	# usage = python notary.py <Key directory>
	