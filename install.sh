apt-get install git
apt-get install gnupg
wget https://bootstrap.pypa.io/get-pip.py
python get-pip.py
pip install python-magic
pip install python-gnupg
git clone -b Notary-Program https://github.com/KAIST-IS521/TeamFour
mkdir /var/log/notary/
rm get-pip.py
cd TeamFour