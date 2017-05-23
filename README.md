# TeamFour
Notary Program

Usage

    Execution Order : Notary -> Launcher -> Client

    Server :  (sudo) python notary.py [Key Directory]
    
    Client :  python client.py [file] [Github ID]

    Client1 : sudo python client1.py [requse github id] -> only for authentication function


# Protocol
A. Before start getting a file, Notary Program authenticate the user using PGP keys.

    1. N(Notary) asks client's github ID
    
    2. C(Client) sends github ID.
    
    3. N searches the client's public key in the key repository. If no public key is searched, close the session.
    
    4. N generates a large random Number, sign it with N's private key and encrypts it with client's public key.
    
    5. N sends the encrypted message and C decrypts the message and verify the message.
    
    6. C decrypts it with her/his private key to get the original random number.
    
    7. C encrypts the random number with the server's public key and send the encrypted message back to the server.
    
    8. N verifies the encrypted message.
        

B. If C succeeds to authenticate, Notary program gets a signed file from client.

    1. C signs a file with her/his private key and send to the N
        
    2. N verifies sign and signs it with N's private Key
    
    3. If file is Executable, N encodes signed file with base64 and sends it to Launcher
    
    4. N sends signed file to C
   
# Misc

1. This program is written in Python 2.X

2. The program has the function that imports keys but you should import private key by shell commands since python gnupg library doesn't support typing the passphrase when importing the private key.
