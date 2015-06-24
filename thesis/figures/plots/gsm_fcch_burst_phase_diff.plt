set term tikz \
  scale 0.45, 0.45 \
  fontscale 0.45
set xlabel "Time $\\sq{ms}$"
set output "figures/plots/gsm_fcch_burst_phase_diff.tex"
plot "figures/plots/data/gsm_fcch_burst_phase_diff.dat" using ($1/2e6*1e3):($2) with lines title ""