#!/bin/bash
if [ $# -eq 0 ]; then
	echo "Usage: otest-4k-reads-graph.sh logfile"
	echo "   (logfile produced by iotest program)"
	exit 0
fi
if [ -f $1 ]; then
	grep "Read from offset" $1 | awk -v N=4 -f iotest-average.awk > iotest-4k-reads-av4.dat
	grep "Read from offset" $1 | awk -v N=256 -f iotest-average.awk > iotest-4k-reads-av256.dat
	gnuplot plot-reads.gnu > iotest-4k-reads.png
	rm iotest-4k-reads-av4.dat iotest-4k-reads-av256.dat
else
	echo "No such file $1"
fi
