import gnupg
import os
class notary:
	def init_server:
	
	def import_keys:
		KEYPATH = './keys/'
		gpg = gnupg.GPG(gnupghome = KEYPATH)	
		Keylist = os.listdir(KEYPATH)

		for i in range (len(Keylist)):
			key_data = open(KEYPATH + Keylist[i]).read()
			gpg.import_keys(key_data)

	def get_files:			

	def verify(file_data):
		verified
		
	def log_all:
	
	def send_sign_with_file:
	
def main():
	init_server()	
	import_keys()
	file = get_files()
	verify(file)