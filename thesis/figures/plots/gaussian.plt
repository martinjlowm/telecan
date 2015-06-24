set term tikz \
  size 12.5cm,5cm

set xlabel "Time $\\sq{\\mu s}$"
set output "figures/plots/gaussian.tex"

set samples 400
set ytics 0.2

T = 3.69
delta(BT) = sqrt(log(2))/(2*pi*BT)
Gaussian(x, BT) = (1/(sqrt(2*pi)*delta(BT)*T)) * exp((-(x**2))/(2*(delta(BT)**2)*(T**2)))

g1(x) = Gaussian(x, 0.1)
g2(x) = Gaussian(x, 0.3)
g3(x) = Gaussian(x, 0.5)
g4(x) = Gaussian(x, 1)

plot g1(x) title "$BT = 0.1$", \
     g2(x) title "$BT = 0.3$" lw 3, \
     g3(x) title "$BT = 0.5$", \
     g4(x) title "$BT = 1$"
