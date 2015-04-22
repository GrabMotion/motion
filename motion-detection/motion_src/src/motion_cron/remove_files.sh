#!/bin/bash
#remove all files older than 1 days

date +%H:%M:%S  >> ../../log/log_upload
echo "removed all files, older than 1days" >> ../../log/log_upload
find ../../pics/* -mmin +1440 -exec sudo rm -rf {} \; >>  ../../log/log_upload
printf "\n\n" >> ../../log/log_upload
