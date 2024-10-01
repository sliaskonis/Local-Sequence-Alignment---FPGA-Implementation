import subprocess
import matplotlib.pyplot as plt
import numpy as np

# Define your arrays
N = [32, 256, 512]
M = [65536, 131072, 300000]

outputs = []
outputs_opt = []
outputs_opt2 = []

# Execute the C program with arguments from N and M arrays
for n in N:
    for m in M:
        result = subprocess.run(["./lsal", str(n), str(m)], capture_output=True, text=True)
        outputs.append(float(result.stdout))

        result2 = subprocess.run(["./lsal_opt", str(n), str(m)], capture_output=True, text=True)
        outputs_opt.append(float(result2.stdout))

        result3 = subprocess.run(["./lsal_opt_pad", str(n), str(m)], capture_output=True, text=True)
        outputs_opt2.append(float(result3.stdout))

    # ******************** PLOT GRAPHS ********************
    # Plot the results
    plt.figure()
    plt.plot(M, outputs, 'o-', label=f'Base lsal')
    plt.plot(M, outputs_opt2, 'o-', label=f'Optimized lsal with if')
    plt.plot(M, outputs_opt, 'o-', label=f'Optimized lsal with padding')
    plt.title(f'Plot for n = {n}')
    plt.xlabel('m number')
    plt.ylabel('Time (ms)')
    plt.legend()
    fig = plt.gcf()
    plt.show()
    fig.savefig(f'x86_plot_{n}.png')

    # ******************* BAR GRAPHS *******************
    plt.figure()
    bar_width = 0.25
    index = np.arange(len(M))
    plt.bar(index, outputs, bar_width, label='Base lsal')
    plt.bar(index + bar_width, outputs_opt2, bar_width, label='Optimized lsal with if')
    plt.bar(index + 2*bar_width, outputs_opt, bar_width, label='Optimized lsal with padding')
    plt.title(f'Plot for n = {n}')
    plt.xlabel('m number')
    plt.ylabel('Time (s)')
    plt.xticks(index + bar_width / 2, M)  # Center the x-axis labels
    plt.legend()
    fig = plt.gcf()
    plt.show()
    fig.savefig(f'x86_bar_{n}.png')

    # Clear the outputs array
    outputs.clear()
    outputs_opt.clear()
    outputs_opt2.clear()