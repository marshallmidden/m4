#!/bin/bash -ex
./working.pl 5.8.3 <working.input >A.jgr
jgraph -P A.jgr >A.ps
