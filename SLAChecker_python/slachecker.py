import socket
import sys
import os
import time
import struct
import gnupg

global_filename = "helloworld.gpg"
global_githubid = "jaemoon-sim"
global_keydir   = "./keys"
global_output   = "sla_result.gpg"
global_passphrase= "tlawoans"
def show_usage(arg):
    print "(Usage) %s <IP_Address> <Port_Num>"%(arg)

def initNet(ip, port):
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    try:
        sock.connect((ip,port))
        test = sock.recv(128)
        #print test
        sock.send(global_githubid)   
    except socket.error as e:
        print "(initNet) ",e
        return None
         
    return sock

def auth(s):
    try:
        data = s.recv(2048)
        gpg = gnupg.GPG()
        filename = global_githubid+'.key'
        filepath = os.listdir(global_keydir)
        key_data = open(global_keydir +'/notary.pub').read()#sever public key
        import_result = gpg.import_keys(key_data)
        ver = import_result.fingerprints[0].encode('ascii')
        
        key_data = open(global_keydir+"/"+filename).read()
        import_result = gpg.import_keys(key_data)
        dec = import_result.fingerprints[0].encode('ascii')
        decrypted_data = gpg.decrypt(data, passphrase=global_passphrase)
        
        if not decrypted_data.ok:
            print "(auth) Fail to decrypt data from notary server"    
            return False
 
        key_data = open(global_keydir+'/notary.pub').read()
        import_result = gpg.import_keys(key_data)
        ver = import_result.fingerprints[0].encode('ascii')
        authdata = gpg.encrypt(str(decrypted_data.data), ver, always_trust = True)
        
        if not authdata.ok:
            print "(auth) Fail to encrypt data"
            return False     
        s.sendall(str(authdata))
         
        #print ("decrypted ok : " + decrypted_data.ok)
    except socket.error as e:
        print "(auth) ",e
        return False
    return True

def send_file(s):
    try:
        filesize = struct.pack("<Q", os.path.getsize(global_filename)) 
        with open(global_filename, 'rb') as f:
            s.send(filesize)
            s.send(f.read())
    except (IOError, OSError) as e:
        print "(send_file)",e
        return False
    except socket.error as e:
        print "(send_file)", e
        return False
    return True


def recv_file(s):
    if( s.recv(5) == "XXXXX"):
        print("file is not verified. Please sign first.")
        s.close()
        return
    
    with open(global_output, "wb") as f:
        g = open(global_output,'wb')
        try:
            filesize = struct.unpack("<Q",s.recv(8))[0]         
            f.write(s.recv(filesize))
        except (IOError, OSError) as e:
            print "(recv_file)", e
            return False
        except socket.error as e:
            print "(recv_file)", e
            return False
    return True
    
def main(argv):
    if len(sys.argv) != 3:
        show_usage(sys.argv[0])
        return -1
    try:
        ip = sys.argv[1]
        port = int(sys.argv[2])
    except:
        show_usage(sys.argv[0])
        return -1

    print ">> Creating Socket to %s : %d"%(ip, port) 
    
    sock = initNet(ip, port)
    if sock == None:
        return 0
    print "Success"
    print
    print ">> Handshaking with notary server"
    
    if not auth(sock) :
        sock.close()
        return 1
    
    print "Success"
    print
    print ">> Sending signed file(%s) to notary server"%(global_filename)
    if not send_file(sock):
        sock.close()
        return 1
    print "Success"
    print
    print ">> Receving notarized file(%s) from notary"%(global_output)
    if not recv_file(sock):
        sock.close()
        return 1
    print "Success"
    
    return 0
 
if __name__ == "__main__":
    main(sys.argv)

