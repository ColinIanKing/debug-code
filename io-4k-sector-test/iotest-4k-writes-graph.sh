#!/bin/bash
if [ $# -eq 0 ]; then
	echo "Usage: otest-4k-writes-graph.sh logfile"
	echo "   (logfile produced by iotest program)"
	exit 0
fi
if [ -f $1 ]; then
	grep "Written to offset" $1 | awk -v N=4 -f iotest-average.awk > iotest-4k-writes-av4.dat
	grep "Written to offset" $1 | awk -v N=256 -f iotest-average.awk > iotest-4k-writes-av256.dat
	gnuplot plot.gnu > iotest-4k-writes.png
	rm iotest-4k-writes-av4.dat iotest-4k-writes-av256.dat
else
	echo "No such file $1"
fi
