import pandas as pd
import matplotlib.pyplot as plt

# -- 1. Read the CSV file --
# We'll tell pandas to ignore lines starting with "TotalRevenue"
df = pd.read_csv("../data/FCR_Solution.csv", comment='T')

# Convert columns to numeric as needed
# If the last line "TotalRevenue" snuck in, you can also drop NaN rows:
df = df.dropna(subset=["AFO", "Hour"])  # ensures we drop any malformed row

# Convert "AFO" and "Hour" to integer if needed
df["AFO"] = df["AFO"].astype(int)
df["Hour"] = df["Hour"].astype(int)

# -- 2. Unique AFOs in the data --
afos = df["AFO"].unique()

# -- 3. Create a figure with subplots for each AFO --
num_afos = len(afos)
fig, axes = plt.subplots(nrows=num_afos, ncols=1, figsize=(8, 3*num_afos))

if num_afos == 1:
    axes = [axes]  # Make it iterable even with one subplot

for i, afo in enumerate(afos):
    ax = axes[i]
    # Filter data for this AFO
    afo_data = df[df["AFO"] == afo]

    ax.plot(afo_data["Hour"], afo_data["Power"],   label="Power",   marker='o')
    ax.plot(afo_data["Hour"], afo_data["UpReg"],   label="UpReg",   marker='^')
    ax.plot(afo_data["Hour"], afo_data["DownReg"], label="DownReg", marker='v')

    ax.set_title(f"AFO {afo} Schedules")
    ax.set_xlabel("Hour")
    ax.set_ylabel("kW (or MW)")
    ax.legend()
    ax.grid(True)

plt.tight_layout()
plt.show()

# Read the CSV, ignore the last line with "TotalRevenue"
df = pd.read_csv("../data/FCR_Solution.csv", comment='T').dropna()

df["AFO"]  = df["AFO"].astype(int)
df["Hour"] = df["Hour"].astype(int)

plt.figure(figsize=(10,6))

# Plot each AFO's Power in a loop
for afo in df["AFO"].unique():
    afo_data = df[df["AFO"] == afo]
    plt.plot(afo_data["Hour"], afo_data["Power"], label=f"AFO {afo} Power")

plt.title("All AFO Power Schedules")
plt.xlabel("Hour")
plt.ylabel("Power (kW)")
plt.legend()
plt.grid(True)
plt.show()


df = pd.read_csv("../data/FCR_Solution.csv", comment='T').dropna()
df["Hour"] = df["Hour"].astype(int)

# Group by hour, summing Power, UpReg, DownReg
agg_by_hour = df.groupby("Hour", as_index=False)[["Power", "UpReg", "DownReg"]].sum()

plt.figure(figsize=(10,5))
plt.plot(agg_by_hour["Hour"], agg_by_hour["Power"],   label="Total Power",   marker='o')
plt.plot(agg_by_hour["Hour"], agg_by_hour["UpReg"],   label="Total UpReg",   marker='^')
plt.plot(agg_by_hour["Hour"], agg_by_hour["DownReg"], label="Total DownReg", marker='v')

plt.title("Aggregated Schedules by Hour (Summed Across All AFOs)")
plt.xlabel("Hour")
plt.ylabel("Sum of Power (kW)")
plt.legend()
plt.grid(True)
plt.show()


df = pd.read_csv("../data/FCR_Solution.csv", comment='T').dropna()

# Convert to numeric
df["AFO"] = df["AFO"].astype(int)

# Sum "Power" for each AFO across all hours
afo_sums = df.groupby("AFO")["Power"].sum().reset_index()

plt.figure(figsize=(8,5))
plt.bar(afo_sums["AFO"], afo_sums["Power"])
plt.xlabel("AFO")
plt.ylabel("Total Power (kWh or kW * hours)")
plt.title("Total Power Used by Each AFO")
plt.grid(True, axis='y')
plt.show()
