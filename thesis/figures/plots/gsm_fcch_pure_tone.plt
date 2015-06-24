set term tikz \
  scale 0.45, 0.45 \
  fontscale 0.45
set view 75,15
set ticslevel 0
set xtics 0,0.25,1 offset 0,-1
set xlabel "Time $\\sq{ms}$" offset -1, -2
set yrange [-0.03:0.08]
set ytics -0.03,0.03,0.08 offset 0.5,-0.5
set ylabel "I" offset 0, -1
set ztics -0.06,0.02,0.04 offset 3, 0
set zlabel "Q"
set output "figures/plots/gsm_fcch_pure_tone.tex"
splot "figures/plots/data/gsm_fcch_pure_tone.dat" using ($1/2e6*1e3):($2):($3) with lines title ""