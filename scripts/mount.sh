#!/bin/bash

sudo umount /mnt/pmem
sudo mkdir -p /mnt/pmem
sudo mkfs.ext4 /dev/pmem0
sudo mount -o dax /dev/pmem0 /mnt/pmem
sudo chmod a+rwx /mnt/pmem
