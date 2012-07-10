#!/bin/bash
fname=$1
echo "Working on file $fname";

/opt/local/bin/gnuplot <<EOF
set term post color
set out "${fname}_plotmon.ps"

set y2tics border;
set grid
set xtics format "%10.0f"
set xlabel "Time (MJD secs)
set key autotitle columnhead

set ylabel "Fro. norm"
set y2label "CasA WSF theta (deg)"
set title "Raw visibility power Vs. CasA WSF position estimations"
pl "$fname" using 1:3 w lp, "" using 1:(\$7*180/3.14159265) axes x1y2 lc 3 with l
set y2label "CasA WSF phi (deg)"
pl "$fname" using 1:3 w lp, "" using 1:(\$8*180/3.14159265) axes x1y2 lc 4 with l

set y2label "CygA flux"
set title "Raw visibility power Vs. CygA estimated flux"
pl "$fname" using 1:3 w lp, "" using 1:26 axes x1y2 lc 3 with l

set y2label "CygA WSF theta (deg)"
set title "Raw visibility power Vs. CygA WSF position estimations"
pl "$fname" using 1:3 w lp, "" using 1:(\$11*180/3.14159265) axes x1y2 lc 3 with l
set y2label "CygA WSF phi (deg)"
pl "$fname" u 1:3 w lp, "" u 1:(\$12*180/3.14159265) axes x1y2 lc 4 with l

set y2label "TauA flux"
set title "Raw visibility power Vs. TauA estimated flux"
pl "$fname" using 1:3 w lp, "" using 1:27 axes x1y2 lc 3 with l

set y2label "TauA WSF theta (deg)"
set title "Raw visibility power Vs. TauA WSF position estimations"
pl "$fname" using 1:3 w lp, "" using 1:(\$15*180/3.14159265) axes x1y2 lc 3 with l
set y2label "TauA WSF phi (deg)"
pl "$fname" u 1:3 w lp, "" u 1:(\$16*180/3.14159265) axes x1y2 lc 4 with l

set y2label "VirA flux"
set title "Raw visibility power Vs. VirA estimated flux"
pl "$fname" using 1:3 w lp, "" using 1:28 axes x1y2 lc 3 with l

set y2label "VirA WSF theta (deg)"
set title "Raw visibility power Vs. VirA WSF position estimations"
pl "$fname" using 1:3 w lp, "" using 1:(\$19*180/3.14159265) axes x1y2 lc 3 with l
set y2label "VirA WSF phi (deg)"
pl "$fname" u 1:3 w lp, "" u 1:(\$20*180/3.14159265) axes x1y2 lc 4 with l

set title "Raw visibility power Vs. Sun estimated flux"
set y2label 
set y2tics;
pl "$fname" using 1:3 w lp
set y2label "Sun estimated flux"
pl "$fname" using 1:3 w lp, "" using 1:29 axes x1y2 lc 3 with lp

set y2label "Sun WSF theta (deg)"
set title "Raw visibility power Vs. Sun WSF position estimations"
pl "$fname" using 1:3 w lp, "" using 1:(\$23*180/3.14159265) axes x1y2 lc 3 with l
set y2label "Sun WSF phi (deg)"
pl "$fname" u 1:3 w lp, "" u 1:(\$24*180/3.14159265) axes x1y2 lc 4 with l

set y2label "Gain norm"
set title "Raw visibility power Vs. Gain norm"
pl "$fname" using 1:3 w lp, "" using 1:32 axes x1y2 lc 3 with lp
EOF
