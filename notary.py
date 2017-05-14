import gnupg
import os
from pprint import pprint
def import_keys():
	KEYPATH = './keys/'
	gpg = gnupg.GPG(gnupghome = KEYPATH)	
	Keylist = os.listdir(KEYPATH)
	for i in range (len(Keylist)):
		key_data = open(KEYPATH + Keylist[i]).read()
		gpg.import_keys(key_data)
	#pprint (gpg.list_keys())
def init_server():
	return
def get_files():			
	return
def verify():		
	return
def log_all():
	return
def send_sign_with_file():
	return
def main():
	import_keys()
	#init_server()	
	#file = get_files()
	#verify(file)
	#log_all()	
	#send _ sign_ with file
	#log
	
if __name__ == "__main__":
    main()

	