#!/usr/bin/env bash
#Beck, 7/16/21a
#This file has not been tested.
THIS_FILE="install_carla.sh:"
CARLA_FILE=CARLA_0.9.11.tar.gz


#Use tmp directory in root folder
cd /tmp
#rm -f $CARLA_FILE

#curl -O https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/$CARLA_FILE
#Go get the file if we don't already have it here.
if ![[ -e $CARLA_FILE ]]; then
  CMD="curl -O https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/$CARLA_FILE"
  echo $CMD
  $CMD
fi 
  

#rm -rf carla_tmp

#Create directory and and -p is don't fail if it's already there
mkdir -p carla_tmp
cd carla_tmp

#tar xvf ../$CARLA_FILE PythonAPI/
CMD="tar xvf ../$CARLA_FILE PythonAPI/"
echo "$THIS_FILE $CMD"
$CMD

#easy_install PythonAPI/carla/dist/carla-0.9.11-py3.7-linux-x86_64.egg || true
#python3 -v -m pip install PythonAPI/carla/dist/carla-0.9.11-py3.7-linux-x86_64.egg

#Install carla and remove temporary files if that succeeds
CMD="python3 -v -m pip install PythonAPI/carla/dist/carla-0.9.11-py3.7-linux-x86_64.egg"
echo "$THIS_FILE $CMD"
$CMD \
&& cd .. \
&& rm -rf /tmp/$CARLA_FILE \
&& rm -rf carla_tmp

echo "$THIS_FILE Done."
