import matplotlib.pyplot as plt

file_name = "timeAndInterval.txt"

periods = [1000000, 100000, 10000, 1000, 100]
phase_duration = 60 * 1_000_000_000

timestamps = []
intervals = []

with open(file_name, "r") as f:
    for line in f:
        parts = line.strip().split()

        if len(parts) < 3:
            continue

        try:
            t = float(parts[0])
            interval = int(parts[2])
        except:
            continue

        timestamps.append(t)
        intervals.append(interval)

if len(timestamps) == 0:
    print("khong doc duoc du lieu")
    exit()

t0 = timestamps[0]
timestamps = [(t - t0) * 1_000_000_000 for t in timestamps]

data_by_phase = [[] for _ in range(5)]

for t, interval in zip(timestamps, intervals):
    phase = int(t // phase_duration)
    if phase < 5:
        data_by_phase[phase].append(interval)

fig, axes = plt.subplots(5, 1, figsize=(10, 12))

for i in range(5):
    axes[i].plot(data_by_phase[i])
    axes[i].set_title(f"x = {periods[i]} ns")
    axes[i].set_ylabel("interval (ns)")
    axes[i].grid(True)

axes[-1].set_xlabel("sample index")

plt.tight_layout()
plt.show()
