This directory contains all the software implementation of the lsal algorithm:
    1. lsal.c         -> original lsal algorithm
    2. lsal_opt.c     -> first optimization (optimal algorithm)
    3. lsal_opt_pad.c -> second optimization (second optimal algorithm, simpler code)

In the same directory there is also a run_all.sh file that you can use to compile, execute and create graphs for different values of N,M
In order to get the graphs for the different optimizations execute the bash script:
$   chmod +x run_all.sh
$   ./run_all.sh

