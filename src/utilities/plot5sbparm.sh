#!/bin/bash
sb0="59756469/srcpos_59756469_1415-1420.txt";
sb1="59951782/srcpos_59951782_1415-1420.txt";
sb2="60147094/srcpos_60147094_1415-1420.txt";
sb3="60342407/srcpos_60342407_1415-1420.txt";
sb4="60537719/srcpos_60537719_1415-1420.txt";
echo "Working on all srcpos files in folder.";

/opt/local/bin/gnuplot <<EOF
set term post color
set out "5sb_plotmon.ps"

set grid
set xtics format "%10.0f"
set xlabel "Time (MJD secs)
set key autotitle columnhead

set ylabel "Fro. norm"
set title "Raw visibility power"
pl "$sb0" using 1:3 t "59756469" w l lc 3, "$sb1" using 1:3 t "59951782" w l lc 4, "$sb2" u 1:3 t "60147094" w l lc 7, "$sb3" u 1:3 t "60342407" w l lc 8, "$sb4" u 1:3 t "60537719" w l lc 9;

#####  Flux estimates #####
set ylabel "CygA. Flux"
set title "CygA Flux estimates"
pl "$sb0" using 1:26 t "59756469" w l lc 3, "$sb1" using 1:26 t "59951782" w l lc 4, "$sb2" u 1:26 t "60147094" w l lc 7, "$sb3" u 1:26 t "60342407" w l lc 8, "$sb4" u 1:26 t "60537719" w l lc 9;

set ylabel "TauA. Flux"
set title "TauA Flux estimates"
pl "$sb0" using 1:27 t "59756469" w l lc 3, "$sb1" using 1:27 t "59951782" w l lc 4, "$sb2" u 1:27 t "60147094" w l lc 7, "$sb3" u 1:27 t "60342407" w l lc 8, "$sb4" u 1:27 t "60537719" w l lc 9;

set ylabel "VirA. Flux"
set title "VirA Flux estimates"
pl "$sb0" using 1:28 t "59756469" w l lc 3, "$sb1" using 1:28 t "59951782" w l lc 4, "$sb2" u 1:28 t "60147094" w l lc 7, "$sb3" u 1:28 t "60342407" w l lc 8, "$sb4" u 1:28 t "60537719" w l lc 9;

set ylabel "Sun. Flux"
set title "Sun Flux estimates"
pl "$sb0" using 1:29 t "59756469" w l lc 3, "$sb1" using 1:29 t "59951782" w l lc 4, "$sb2" u 1:29 t "60147094" w l lc 7, "$sb3" u 1:29 t "60342407" w l lc 8, "$sb4" u 1:29 t "60537719" w l lc 9;

######  Position estimates #######
set ylabel "CasA WSF theta (Deg)"
set title "CasA WSF position estimates"
pl "$sb0" using 1:(\$7*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$7*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$7*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$7*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$7*180/3.14159265) t "60537719" w l lc 9;

set ylabel "CygA WSF theta (Deg)"
set title "CygA WSF position estimates"
pl "$sb0" using 1:(\$11*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$11*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$11*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$11*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$11*180/3.14159265) t "60537719" w l lc 9;

set ylabel "TauA WSF theta (Deg)"
set title "TauA WSF position estimates"
pl "$sb0" using 1:(\$15*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$15*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$15*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$15*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$15*180/3.14159265) t "60537719" w l lc 9;

set ylabel "VirA WSF theta (Deg)"
set title "VirA WSF position estimates"
pl "$sb0" using 1:(\$19*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$19*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$19*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$19*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$19*180/3.14159265) t "60537719" w l lc 9;

set ylabel "Sun WSF theta (Deg)"
set title "Sun WSF position estimates"
pl "$sb0" using 1:(\$23*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$23*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$23*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$23*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$23*180/3.14159265) t "60537719" w l lc 9;

######## Phi estimates
set ylabel "CasA WSF phi (Deg)"
set title "CasA WSF position estimates"
pl "$sb0" using 1:(\$8*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$8*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$8*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$8*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$8*180/3.14159265) t "60537719" w l lc 9;

set ylabel "CygA WSF phi (Deg)"
set title "CygA WSF position estimates"
pl "$sb0" using 1:(\$12*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$12*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$12*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$12*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$12*180/3.14159265) t "60537719" w l lc 9;

set ylabel "TauA WSF phi (Deg)"
set title "TauA WSF position estimates"
pl "$sb0" using 1:(\$16*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$16*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$16*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$16*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$16*180/3.14159265) t "60537719" w l lc 9;

set ylabel "VirA WSF phi (Deg)"
set title "VirA WSF position estimates"
pl "$sb0" using 1:(\$20*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$20*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$20*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$20*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$20*180/3.14159265) t "60537719" w l lc 9;

set ylabel "Sun WSF phi (Deg)"
set title "Sun WSF position estimates"
pl "$sb0" using 1:(\$24*180/3.14159265) t "59756469" w l lc 3, "$sb1" using 1:(\$24*180/3.14159265) t "59951782" w l lc 4, "$sb2" u 1:(\$24*180/3.14159265) t "60147094" w l lc 7, "$sb3" u 1:(\$24*180/3.14159265) t "60342407" w l lc 8, "$sb4" u 1:(\$24*180/3.14159265) t "60537719" w l lc 9;

EOF
