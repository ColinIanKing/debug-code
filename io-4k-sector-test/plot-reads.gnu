set terminal png transparent font "arial" 8
set title "4K Read Transfer Rate vs Location on HDD"
set xlabel "Seek Location (MB offset)"
set ylabel "Transfer Rate (MB/sec)"
plot 'iotest-4k-reads-av4.dat' with lines title "Reads (Average 4)", \
     'iotest-4k-reads-av256.dat' with lines title "Reads (Average 256)" \
    
