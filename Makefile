all: flagUpdater/flag_updater
	sudo mkdir -p /var/ctf/
	sudo chmod 777 /var/ctf/
flagUpdater/flag_updater :  
	gcc flagUpdater/flag_updater.c -o flagUpdater/flag_updater 
clean :
	rm -rf flagUpdater/flag_updater

