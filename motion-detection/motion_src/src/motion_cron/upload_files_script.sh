#check if connection else restart network interfaces
date +%H:%M:%S  >> ../../log/log_upload
if  ping -q -c 1 localhost > /dev/null ; then
	#rsync -av -e 'ssh -p 4000' /home/pi/opencv/pics/. cedric1q@cedricve.be:public_html/secure/pics/ >> /home/pi/opencv/log_upload
	rsync -xav ../../pics/ ../motion_web/pics >> ../../log/log_upload
	printf "\n\n" >> ../../log/log_upload;
else
	printf "connection lost\n\n" >> ../../log/log_upload;
	sudo /etc/init.d/networking restart
fi




