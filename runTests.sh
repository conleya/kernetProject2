#!/bin/bash

./serv.out &
sleep .3
for i in {0..1}
do
	./client.out
	echo $i
done
