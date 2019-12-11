#!/bin/bash

make serial_hash parallel_hash

./serial_hash $1 $2 1 > ./serial/serial.output

sort thread-1.out > ./serial/check.output
rm "thread-1.out"

./parallel_hash $1 $2 $3 > ./parallel/parallel.output

if [ -f "./parallel/check.output.temp" ]
then
    rm "./parallel/check.output.temp"
fi

for i in `seq 1 $3`
do
    cat "thread-$i.out" >> ./parallel/check.output.temp
    rm "thread-$i.out"
done

sort ./parallel/check.output.temp > ./parallel/check.output
rm ./parallel/check.output.temp

cut -f1-4 -d " " serial/serial.output > serial/serial1.output
cut -f1-4 -d " " parallel/parallel.output > parallel/parallel1.output
cat serial/serial.output > tmp1
cat parallel/parallel.output > tmp2
head -n1 tmp1 > serial/serial.output
head -n1 tmp2 > parallel/parallel.output
rm tmp1 tmp2                         
echo "Checking Output 1st line Diff"

diff ./parallel/parallel.output ./serial/serial.output

echo "Checking Output Diff"

diff ./parallel/parallel1.output ./serial/serial1.output

echo "Checking Thread Diff"

diff ./parallel/check.output ./serial/check.output

echo "Checking Diff Completed"
