import base64
file = open("signature_encrypt64.flag", "r");
encoded = file.read()
data = base64.b64decode(encoded)
print(data)

