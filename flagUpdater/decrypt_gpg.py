import socket, sys, re, os
import json
import gnupg
import base64

input_passphrase = "notary897"

global gpg
global flag
global host
homedir = '/home/vagrant/.gnupg'
try:
  gpg = gnupg.GPG(gnupghome=homedir) 
except TypeError:
  gpg = gnupg.GPG(homedir=homedir)

def decrypt(data):
    objdata = gpg.decrypt(data, passphrase=input_passphrase, always_trust=True)
    if objdata.ok != True :
        return False
    return str(objdata)

file = open("received_encrypt.flag.gpg", "r");
recvdata = file.read()

plain = decrypt(recvdata)

print(plain)
