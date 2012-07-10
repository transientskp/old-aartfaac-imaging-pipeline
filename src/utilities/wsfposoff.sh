#!/bin/bash
fname=$1
echo "Working on file $fname";

/opt/local/bin/gnuplot << EOF
set term post color
set out "${fname}_wsfposoff.ps"

set y2tics border;
set grid
set xtics format "%10.0f"
set xlabel "Time (MJD secs)
set key autotitle columnhead

set ylabel "Position (deg)"
set title "CasA Catalog positions Vs. WSF position estimate"
pl "$fname" using 1:(\$5*180/3.14159265) w lp, "" using 1:(\$7*180/3.14159265) lc 3 with l
pl "$fname" using 1:(\$6*180/3.14159265) w lp, "" using 1:(\$8*180/3.14159265) lc 3 with l
#unset multitplot

set title "CygA Catalog positions Vs. WSF position estimate"
pl "$fname" using 1:(\$9*180/3.14159265) w lp, "" using 1:(\$11*180/3.14159265) lc 3 with l
pl "$fname" u 1:(\$10*180/3.14159265) w lp, "" u 1:(\$12*180/3.14159265) lc 4 with l

set title "TauA Catalog positions Vs. WSF position estimate"
pl "$fname" using 1:(\$13*180/3.14159265) w lp, "" using 1:(\$15*180/3.14159265) lc 3 with l
set y2label "TauA WSF phi (deg)"
pl "$fname" using 1:(\$14*180/3.14159265) w lp, "" using 1:(\$16*180/3.14159265) lc 3 with l


set title "VirA Catalog positions Vs. WSF position estimate"
pl "$fname" using 1:(\$17*180/3.14159265) w lp, "" using 1:(\$19*180/3.14159265) lc 3 with l
pl "$fname" u 1:(\$18*180/3.14159265) w lp, "" u 1:(\$20*180/3.14159265) lc 4 with l


set title "Sun Catalog positions Vs. WSF position estimate"
pl "$fname" using 1:(\$21*180/3.14159265) w lp, "" using 1:(\$23*180/3.14159265) lc 3 with l
pl "$fname" u 1:(\$22*180/3.14159265) w lp, "" u 1:(\$24*180/3.14159265) lc 4 with l
