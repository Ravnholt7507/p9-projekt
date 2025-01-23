import pandas as pd
import matplotlib.pyplot as plt

def load_and_prepare_data(csv_file: str) -> pd.DataFrame:
    df = pd.read_csv(csv_file)

    # Map aggregator_type
    aggregator_map = {0: 'Normal', 1: 'TEC', 2: 'DFO'}
    df['aggregator_label'] = df['aggregator_type'].map(aggregator_map)

    # Map alignment
    alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
    df['alignment_label'] = df['alignment'].map(alignment_map)

    df['savings_percent'] = (df['savings'] / df['baseline_cost']) * 100
    # Combine threshold into a short label
    df['threshold_label'] = "th=" + df['est_threshold'].astype(str)

    # Combine group size
    df['group_size_label'] = "G_size=" + df['max_group_size'].astype(str)

    return df

def _add_bar_value_labels(ax, fmt="%.1f%%"):
    """
    Manually label each bar with its height value. 
    This works in older Matplotlib versions without ax.bar_label.
    """
    for container in ax.containers:
        for bar in container:
            height = bar.get_height()
            if height > 0:
                x = bar.get_x() + bar.get_width() / 2
                y = height
                label = fmt % height
                ax.text(x, y, label, ha='center', va='bottom', fontsize=8)

def plot_savings_grid(df: pd.DataFrame):
    group_sizes = df['group_size_label'].unique()
    aggregators = df['aggregator_label'].unique()

    fig, axes = plt.subplots(
        nrows=len(group_sizes),
        ncols=len(aggregators),
        figsize=(5 * len(aggregators), 4 * len(group_sizes)),
        sharey=True
    )
    fig.suptitle("Figure 1: Savings – Subplots by (Group Size x Aggregator)", fontsize=12)

    # In case there's only one aggregator, reshape axes
    if len(aggregators) == 1:
        axes = [[ax] for ax in axes]

    for row_idx, gsize in enumerate(group_sizes):
        for col_idx, agg_lbl in enumerate(aggregators):
            ax = axes[row_idx][col_idx]

            # Filter for aggregator + group size
            subdf = df[
                (df['aggregator_label'] == agg_lbl) &
                (df['group_size_label'] == gsize)
            ]

            # Group by threshold_label and alignment_label, averaging the "savings_percent"
            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False)['savings_percent'].mean()

            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            # Pivot for the bar plot
            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='savings_percent')
            plot_obj = pivoted.plot(kind='bar', ax=ax, legend=False)

            # Add manual value labels on each bar
            _add_bar_value_labels(ax, fmt="%.1f%%")

            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            if col_idx == 0:
                ax.set_ylabel("Avg Savings (%)")
            ax.legend(fontsize=8)

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
    fig.suptitle("Figure 2: Scenario Time (s) – Subplots by (Group Size x Aggregator)", fontsize=14)

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
            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False)['scenario_time'].mean()

            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='scenario_time')
            plot_obj = pivoted.plot(kind='bar', ax=ax, legend=False)

            # Label each bar manually (in seconds with 2 decimals)
            for container in ax.containers:
                for bar in container:
                    height = bar.get_height()
                    if height > 0:
                        x = bar.get_x() + bar.get_width() / 2
                        y = height
                        label = f"{height:.2f}"
                        ax.text(x, y, label, ha='center', va='bottom', fontsize=8)

            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            if col_idx == 0:
                ax.set_ylabel("Scenario Time (s)")
            ax.legend(fontsize=8)

    plt.tight_layout()
    plt.show()

def main():
    csv_file = "../data/economic_savings.csv"  # Replace with your actual CSV
    df = load_and_prepare_data(csv_file)

    # Plot savings in percentages
    plot_savings_grid(df)

    # Plot scenario time
    plot_scenario_time_grid(df)

if __name__ == "__main__":
    main()
