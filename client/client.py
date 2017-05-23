import socket
import sys
import os
import time
import struct
import gnupg

global_keypath = "../keys/"
global_passphrase = "007dudrhkd"

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
    print "Recv: data"+data

    gpg = gnupg.GPG()
    filename = sys.argv[2]+'.key'
    filepath = os.listdir(global_keypath)
    key_data = open(global_keypath +'/notary.pub').read()#sever public key
    import_result = gpg.import_keys(key_data)
    ver = import_result.fingerprints[0].encode('ascii')
    key_data = open(global_keypath+filename).read()
    import_result = gpg.import_keys(key_data)
    dec = import_result.fingerprints[0].encode('ascii')
    print "fingerprint"+dec
    decrypted_data = gpg.decrypt(data, passphrase= global_passphrase)
    print "decrypted data:"+decrypted_data.data

    key_data = open(global_keypath+'/notary.pub').read()
    import_result = gpg.import_keys(key_data)
    ver = import_result.fingerprints[0].encode('ascii')
    authdata = gpg.encrypt(str(decrypted_data.data), ver, always_trust = True)
    print authdata.ok
    s.sendall(str(authdata))
    print 'check'

    return 1

def send(s,argv):
    filename = argv
    filesize = struct.pack("<Q", os.path.getsize(argv))
    f=open(argv,'rb')
    s.send(filesize)
    print 'sending filesize..'
    s.send(f.read())
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
    except:
        print('error occured')
    print 'Done'
    g.close()
    s.close()

def main(argv):
    s = initNet(sys.argv[2])
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
