sudo mknod /dev/simple_character_device c 240 0
sudo chmod 777 /dev/simple_character_device
sudo make
insmod my_driver.ko