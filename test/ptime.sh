#!/bin/bash 
# Mesure le temps en nanoseconde de la commande passée en paramètre
t1=$(date +%s%N)
$*
t2=$(date +%s%N)
let $[dt=$t2-$t1]
echo "$dt ns"