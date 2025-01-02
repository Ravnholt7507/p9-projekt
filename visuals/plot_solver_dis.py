import pandas as pd
import matplotlib.pyplot as plt

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

if __name__ == "__main__":
    main()
