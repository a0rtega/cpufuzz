#!/bin/bash

while [ 1 ]; do timeout -s 9 300 ./cpufuzz 10.0.2.2 9999; pkill -9 cpufuzz; done

