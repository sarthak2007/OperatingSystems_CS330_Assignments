#! /bin/sh

cd ~/Documents/CS330/Assignment2/OS_CS330_Assignment_2-master-74142bb70987a9ece2389665671106766b5c7bc8/gemOS/src
make -j4
cp gemOS.kernel ../../../gem5/gemos/binaries
cd ../../../gem5
build/X86/gem5.opt configs/example/fs.py --kernel=$M5_PATH/binaries/gemOS.kernel --mem-size=2048MB
# cd ~/Documents/CS330/Assignment2/gem5
# export M5_PATH=~/Documents/CS330/Assignment2/gem5/gemos