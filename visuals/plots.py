import pandas as pd
import matplotlib.pyplot as plt

def load_and_prepare_data(csv_file: str) -> pd.DataFrame:
    """
    Load CSV data and create some helpful labels for aggregator type,
    alignment, thresholds, group size, etc.
    """
    df = pd.read_csv(csv_file)

    # Map aggregator_type
    aggregator_map = {0: 'Normal', 1: 'TEC', 2: 'DFO'}
    df['aggregator_label'] = df['aggregator_type'].map(aggregator_map)
    # Map alignment
    alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
    df['alignment_label'] = df['alignment'].map(alignment_map)
    # Combine thresholds into a short string label, e.g. "est=2|lst=2"
    df['threshold_label'] = ("th=" + df['est_threshold'].astype(str))

    # Combine group size for convenience
    df['group_size_label'] = "G_size=" + df['max_group_size'].astype(str)

    return df
def plot_savings_grid(df: pd.DataFrame):
    """
    Plots savings as a percentage of baseline_cost. Then labels each bar with the % value.
    """
    group_sizes = df['group_size_label'].unique()
    aggregators = df['aggregator_label'].unique()

    fig, axes = plt.subplots(
        nrows=len(group_sizes),
        ncols=len(aggregators),
        figsize=(5 * len(aggregators), 4 * len(group_sizes)),
        sharey=True
    )
    fig.suptitle("Figure 1: Savings as % of Baseline – Subplots by (Group Size x Aggregator)", fontsize=12)

    # Handle subplots shape if there's only one aggregator or group size
    if len(aggregators) == 1 and len(group_sizes) == 1:
        axes = [[axes]]
    elif len(aggregators) == 1:
        axes = [axes]
    elif len(group_sizes) == 1:
        axes = [axes]

    for row_idx, gsize in enumerate(group_sizes):
        for col_idx, agg_lbl in enumerate(aggregators):
            ax = axes[row_idx][col_idx]

            # Filter data to this aggregator & group size
            subdf = df[(df['aggregator_label'] == agg_lbl) &
                       (df['group_size_label'] == gsize)]
            # Group by threshold & alignment, then compute mean of 'savings' and 'baseline_cost'
            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False).agg({
                'savings': 'mean',
                'baseline_cost': 'mean'
            })
            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            # Compute savings% = (savings / baseline_cost)*100
            grouped['pct_savings'] = (grouped['savings'] / grouped['baseline_cost']) * 100.0

            # pivot so threshold_label => rows, alignment_label => columns
            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='pct_savings')

            # plot as bar chart
            bars = pivoted.plot(kind='bar', ax=ax)

            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            if col_idx == 0:
                ax.set_ylabel("Savings (% of Baseline)")
            ax.legend(fontsize=8)

            # Label each bar with its numeric value
            for container in ax.containers:
                for bar in container:
                    height = bar.get_height()
                    ax.annotate(
                        f"{height:.1f}%",
                        xy=(bar.get_x() + bar.get_width()/2, height),
                        xytext=(0, 3),  # vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom'
                    )

    plt.tight_layout()
    plt.show()
def plot_scenario_time_grid(df: pd.DataFrame):
    group_sizes = df['group_size_label'].unique()
    aggregators = df['aggregator_label'].unique()

    fig, axes = plt.subplots(
        nrows=len(group_sizes),
        ncols=len(aggregators),
        figsize=(5 * len(aggregators), 4 * len(group_sizes)),
        sharey=True
    )
    fig.suptitle("Figure 2: Scenario Time (seconds) – Subplots by (Group Size x Aggregator)", fontsize=14)

    # Handle subplots shape for 1D cases
    if len(group_sizes) == 1 and len(aggregators) == 1:
        axes = [[axes]]
    elif len(group_sizes) == 1:
        axes = [axes]
    elif len(aggregators) == 1:
        axes = [[ax] for ax in axes]

    for row_idx, gsize in enumerate(group_sizes):
        for col_idx, agg_lbl in enumerate(aggregators):
            ax = axes[row_idx][col_idx]

            # Filter data
            subdf = df[
                (df['aggregator_label'] == agg_lbl) &
                (df['group_size_label'] == gsize)
            ]

            # Group by threshold_label and alignment_label
            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False)['scenario_time'].mean()

            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            # Pivot => threshold_label on x axis, alignment_label => columns
            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='scenario_time')

            bars = pivoted.plot(kind='bar', ax=ax)
            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            
            # Label only the leftmost column's y-axis
            if col_idx == 0:
                ax.set_ylabel("Scenario Time (s)")

            ax.legend(fontsize=8)

            # ---- Label each bar with numeric value in seconds ----
            for container in ax.containers:
                for bar in container:
                    height = bar.get_height()  # scenario time in seconds
                    ax.annotate(
                        f"{height:.2f}s",                 # e.g. "12.34s"
                        xy=(bar.get_x() + bar.get_width()/2, height),
                        xytext=(0, 3),                    # vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom'
                    )

    plt.tight_layout()
    plt.show()

import pandas as pd
import matplotlib.pyplot as plt

def plot_savings_vs_noffers_by_alignment(df: pd.DataFrame):
    """
    Plots 'scenario_time' (performance in seconds) vs. the number of FlexOffers (NrOfFlexOffers).
    Generates separate plots for each alignment type.
    Subplots within each plot are arranged by (group_size_label x aggregator_label).
    """
    
    # Create a string label for number of FlexOffers
    df['n_offers_label'] = "N=" + df['NrOfFlexOffers'].astype(str)
    
    group_sizes = df['group_size_label'].unique()
    aggregators = df['aggregator_label'].unique()
    alignments = df['alignment_label'].unique()
    
    # Iterate over each alignment type
    for alignment in alignments:
        # Filter data for the current alignment
        alignment_df = df[df['alignment_label'] == alignment]
        
        # Create a new figure for the current alignment
        fig, axes = plt.subplots(
            nrows=len(group_sizes),
            ncols=len(aggregators),
            figsize=(5 * len(aggregators), 4 * len(group_sizes)),
            sharey=True
        )
        fig.suptitle(f"Performance (s) vs. Number of FlexOffers\nAlignment: {alignment}", fontsize=14)
        
        # Adjust axes array shape for single row/column scenarios
        if len(group_sizes) == 1 and len(aggregators) == 1:
            axes = [[axes]]
        elif len(group_sizes) == 1:
            axes = [axes]
        elif len(aggregators) == 1:
            axes = [axes]
        
        for row_idx, gsize in enumerate(group_sizes):
            for col_idx, agg_lbl in enumerate(aggregators):
                ax = axes[row_idx][col_idx]
                
                # Filter data for this aggregator & group size within the current alignment
                subdf = alignment_df[
                    (alignment_df['group_size_label'] == gsize) &
                    (alignment_df['aggregator_label'] == agg_lbl)
                ]
                
                # Compute average scenario_time by n_offers_label
                grouped = subdf.groupby('n_offers_label', as_index=False)['scenario_time'].mean()
                
                if grouped.empty:
                    ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                    ax.set_xticks([])
                    ax.set_yticks([])
                    continue
                
                # Plot as bar
                plot_bars = ax.bar(grouped['n_offers_label'], grouped['scenario_time'], color='skyblue')
                
                ax.set_title(f"{agg_lbl}, {gsize}")
                ax.set_xlabel("Number of FlexOffers")
                if col_idx == 0:
                    ax.set_ylabel("Performance (s)")
                
                # Label each bar with numeric value
                for bar in plot_bars:
                    height = bar.get_height()
                    ax.annotate(f"{height:.2f}s",
                                xy=(bar.get_x() + bar.get_width() / 2, height),
                                xytext=(0, 3),  # 3 points vertical offset
                                textcoords="offset points",
                                ha='center', va='bottom',
                                fontsize=8)
        
        plt.tight_layout(rect=[0, 0.03, 1, 0.95])  # Adjust layout to accommodate the suptitle
        plt.show()



def main():
    csv_file = "../data/economic_savings.csv"  # Replace with your actual CSV
    df = load_and_prepare_data(csv_file)

    # Figure 1: savings
    plot_savings_grid(df)

    plot_savings_vs_noffers_by_alignment(df)
    # Figure 2: scenario_time
    plot_scenario_time_grid(df)

if __name__ == "__main__":
    main()