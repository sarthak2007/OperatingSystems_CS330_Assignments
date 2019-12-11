#! /bin/sh

cd ~/Documents/CS330/Assignment3/Ass3/src
make -j4
cp gemOS.kernel ../../../Assignment2/gem5/gemos/binaries
cd ../../../Assignment2/gem5
build/X86/gem5.opt configs/example/fs.py --kernel=$M5_PATH/binaries/gemOS.kernel --mem-size=2048MB
# cd ~/Documents/CS330/Assignment2/gem5
# export M5_PATH=~/Documents/CS330/Assignment2/gem5/gemos