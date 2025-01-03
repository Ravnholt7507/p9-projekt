import pandas as pd
import matplotlib.pyplot as plt

def plot_aggregation_performance(csv_filename="../data/aggregation_performance.csv"):

    df = pd.read_csv(csv_filename)
    plt.figure(figsize=(6, 4))
    plt.bar(df["num_flexOffers"], df["aggregation_time"], color="skyblue")
    plt.title("Aggregation Time vs. Number of FlexOffers")
    plt.xlabel("Number of FlexOffers")
    plt.ylabel("Aggregation Time (seconds)")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig("aggregation_performance.png", dpi=300, bbox_inches='tight')
    plt.show()


def main():
    agg_data_folder = "../data/aggregator_solutions.csv"
    dis_data_folder = "../data/disaggregated_flexoffers.csv"
    df_agg = pd.read_csv(agg_data_folder)
    df_dis = pd.read_csv(dis_data_folder)

    fig2, ax2 = plt.subplots(figsize=(8, 5))
    for agg_id, group in df_agg.groupby("AggregatorID"):
        ax2.plot(group["Hour"], group["Cost"], marker='s', label=f"Aggregator {agg_id} Cost")
    ax2.set_title("Aggregator-Level Cost over Time")
    ax2.set_xlabel("Hour")
    ax2.set_ylabel("Cost (EUR)")
    ax2.legend()
    ax2.grid(True)
    plt.savefig("aggregator_costs.png", dpi=300, bbox_inches="tight")
    plt.show()

    data = {
        "num_flexoffers": [10, 20, 30, 40, 50],
        "aggregation_time": [0.15, 0.35, 0.70, 1.10, 2.00]  # e.g. in seconds
    }
    df = pd.DataFrame(data)

    # Create a bar chart
    plt.figure(figsize=(6,4))
    plt.bar(df["num_flexoffers"], df["aggregation_time"], width=5.0, color='skyblue')

    # Labeling
    plt.title("Aggregation Time vs. Number of FlexOffers")
    plt.xlabel("Number of FlexOffers")
    plt.ylabel("Aggregation Time (seconds)")  # or minutes, etc.

    # Optionally, add grid or more styling
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Save the plot (optional)
    plt.savefig("aggregation_time_vs_flexoffers.png", dpi=300, bbox_inches='tight')

    # Show the plot
    plt.show()

if __name__ == "__main__":
    main()
