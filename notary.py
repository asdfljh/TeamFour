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
from datetime import datetime

def import_keys(argv):
    KEYPATH = argv
    gpg = gnupg.GPG(gnupghome = KEYPATH)
    filenames = os.listdir(KEYPATH)
    for filename in filenames:
        fullname = os.path.join(KEYPATH,filename)
        ext = os.path.splitext(fullname)[-1]

        '''
        .pub is public key, and .key is secret key
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
    # Initiating Server..
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
    # Get a file from a client and save it as 'temp'
    try:
        logging.info('Receiving File..')
        recv_file = open('./Download/temp','w')
        filesize = struct.unpack("<Q", conn.recv(8))[0]
        recv_file.write(conn.recv(filesize))

        logging.info('File transmission finished')
        recv_file.close()

    except IOError:
        logging.exception("Open error")
        recv_file.close()
        return -1

    except:
        logging.exception("Unexpected error, file transmission failed")
        recv_file.close()
        return -1

    return 1

def verify(gpg,conn):
    # We verify downloaded file
    file_verify = open('./Download/temp','rb')
    verified = gpg.verify_file(file_verify)
    Success = -1

    if not verified:
    # We send a specific string about authentication
        conn.send("XXXXX")
        file_verify.close()
        Success = -1
        print("Not a valid file")
    else:
        conn.send("YYYYY")
        Success = 1

    file_verify.close()
    return Success


def send_file_with_sign(c, gpg, address):

    # Before return our signed file we resotre the signed file.
    en_file = open('./Download/temp','r')
    de_file = open('./Download/temp_clear','w')
    # temp_clear is a decrypted file
    decrypt = gpg.decrypt(en_file.read()).data
    de_file.write(decrypt)
    # We sign the decrypted file with our private key
    sign = gpg.sign(decrypt,passphrase = "notary897",output = "./Download/temp.gpg")
    sign_file = open('./Download/temp.gpg') # Signed file
    # Below functions are about JSON format.
    # JSON format is:

    '''
   {
   "name": "FROM_[CLIENT-IP]_[YEAR]_[MONTH]_[HOUR]_[MIN]_[Microsecond]",
   "body": "[base64 encoded program executable]"
   }

    '''

    tm = datetime.now()
    timestring = str(tm.year)+"_"+str(tm.month)+"_"+str(tm.hour)+"_"+str(tm.    minute)+"_"+str(tm.microsecond)
    NAME = "FROM_"+str(address)+"_"+timestring
    file_b64encode = base64.b64encode(sign_file.read())
    TEXT = "{\"name\": \"" + NAME + "\",\"body\": \"" + file_b64encode + "\"    }"


    filesize = struct.pack("<Q",size(TEXT))
    c.send(filesize)   # Send File size first

    Success = -1
    stream = 1
    # Send file as a JSON format
    try:
        stream = TEXT
        c.send(stream)

    except:
        logging.exception("Unexpected error, file transmission failed")
        return Success

    Success = 1
    en_file.close()
    de_file.close()
    sign_file.close()

    return Success


def Isexecutable():
    # This fuction checks the file is executable or not
    de_file = open('./Download/temp_clear')
    if (magic.from_buffer(de_file.read())[:3] != 'ELF' ):
        print('Not Executable')
        de_file.close()
        return -1
    de_file.close()
    return 1


def file_is_executable(gpg):
    IP = ''     # Designated local host
    port = 8001

    try:
        print("connect..")
        s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        s.connect((IP, port))

    except socket.error,e :
        logging.exception("Socket error")
        return -1

    try:
        print ('Sending Executable....')
        stream = TEXT
        s.send(stream)

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

    #Searches the public key matches a user's name
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
    # Generate a big random bits, sign and encrypt it.
    number = random.getrandbits(512)
    encrypted_data = gpgauth.encrypt(str(number), public, sign=private, passphrase = "")
    c.sendall(str(encrypted_data))
    authrandom = c.recv(2048)

    if not authrandom :
        print "check"
        return -1

    print ("Received encrypted randomnumber")

    # Verifies a message from Client
    key_data2 = open(path+filename).read()
    import_result = gpgauth.import_keys(key_data2)
    verified = gpgauth.decrypt(authrandom, passphrase="")

    if not verified.ok :
        print(verified.status)
        return -1
    if str(verified) == str(number) :
        return 0
    else :
        print("Random number isn't same")
        return -1

# If an error is occured logs it
def errorInConnect(c,comment):
    logging.info('%s', comment)
    c.close()

def main(argv):

    # We will run this program with sudo command.
    # Log all events
    loggingconfig()

    # 0. Import public and private keys first
    gpg = import_keys(argv[1])

    # Initiaiting Server
    sock = init_server()
    print ('Initiating Server...')
    if sock == -1 :       # get a sock object
        logging.error('Cannot init server')
        sys.exit(1)

    while True:
        # Server is keep working
        sock.listen(1)
        print ('Server is Listening...')

        connect, address = sock.accept()
        logging.info('Connetion established with %s', address)
        print ('Connect from', address[0])

        # 1. Server is authenticate a user
        auth = authuser(connect,argv[1])
        if auth < 0 :
            errorInConnect(connect,'Authenticaion fail')
            continue
        print ('1. Auth success')

        # 2. Server gets a file from a user
        f = get_files(connect)
        if f < 0 :
            errorInConnect(connect,'Get File error')
            continue
        print ('2. Got a file from Client')

        # 3. Server verifies a file given by a user
        if verify(gpg,connect) < 0:
            errorInConnect(connect,'Verify error')
            continue
        print ('3. Verification Success')

        # 4. Server signs the file and return the JSON file to the user
        if send_file_with_sign(connect, gpg, address[0]) < 0:
            errorInConnect(connect,'Send file with sign error')
            continue
        print ('4. Sign and Send')
        connect.close()

        return

# Below comments are about communication with our Launcher
# Is this necessary?
'''
        if Isexecutable() > 0:
            if file_is_executable(gpg) < 0:
                continue
    return
'''

if __name__ == "__main__":
    main(sys.argv)
