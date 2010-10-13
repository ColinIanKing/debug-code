set terminal png transparent font "arial" 8
set title "4K Write Transfer Rate vs Location on HDD"
set xlabel "Seek Location (MB offset)"
set ylabel "Transfer Rate (MB/sec)"
plot 'iotest-4k-writes-av4.dat' with lines title "Writes (Average 4)", \
     'iotest-4k-writes-av256.dat' with lines title "Writes (Average 256)" \
    
