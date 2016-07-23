#!/bin/bash
clear
echo "Clearing data files and reseting database."
sudo rm -R /home/cedric/motion/motion-detection/motion_src/src/motion_detect/data
sudo mkdir /home/cedric/motion/motion-detection/motion_src/src/motion_detect/data
sudo chmod 777 -R /home/cedric/motion/motion-detection/motion_src/src/motion_detect/data
sudo git checkout /home/cedric/motion/motion-detection/motion_src/src/motion_detect/database/motion.db
echo "Clearing cameras data."
sudo git checkout /home/cedric/motion/motion-detection/motion_src/src/motion_detect/database/motion.db
if [ -d /home/cedric/motion/motion-detection/motion_src/src/motion_web/pics/camera* ]; then
    sudo rm -R /home/cedric/motion/motion-detection/motion_src/src/motion_web/pics/camera*
    echo "Removing cameras data..."    
fi
