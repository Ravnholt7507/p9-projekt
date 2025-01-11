# import pandas as pd
# import matplotlib.pyplot as plt

# def load_and_structure(csv_file: str) -> pd.DataFrame:
#     df = pd.read_csv(csv_file)

#     aggregator_map = {0: 'Normal', 1: 'TEC', 2: 'Other?'}
#     if 'aggregator_type' in df.columns:
#         df['aggregator_label'] = df['aggregator_type'].map(aggregator_map).fillna('Unknown')


#     alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
#     if 'alignment' in df.columns:
#         df['alignment_label'] = df['alignment'].map(alignment_map).fillna('Unknown')

#     return df


# def plot_time_vs_flexoffers(csv_file: str):
#     df = load_and_structure(csv_file)

#     grouped = df.groupby(['NrOfFlexOffers','aggregator_label'])['scenario_time'].mean().reset_index()

#     pivoted = grouped.pivot(index='NrOfFlexOffers', columns=['aggregator_label'], values='scenario_time')
#     pivoted.plot(kind='bar', width=0.75, figsize=(7,5), color=['skyblue','orange','green'])

#     plt.xlabel("Number of FlexOffers")
#     plt.ylabel("Average Scenario Time (s)")
#     plt.title("Scenario Time vs. Number of FlexOffers\n(grouped by Aggregator Type)")
#     plt.legend(title='Aggregator Type')
#     plt.tight_layout()
#     plt.show()


# def plot_alignment_savings(csv_file: str):
#     df = load_and_structure(csv_file)

#     grouped = df.groupby(['alignment_label','aggregator_label'])['savings'].mean().reset_index()

#     pivoted = grouped.pivot(index='alignment_label', columns='aggregator_label', values='savings')
#     pivoted.plot(kind='bar', figsize=(7,5))

#     plt.xlabel("Alignment Method")
#     plt.ylabel("Average Savings")
#     plt.title("Savings by Alignment, grouped by Aggregator Type")
#     plt.legend(title='Aggregator')
#     plt.tight_layout()
#     plt.show()

# def plot_baseline_vs_aggregated(csv_file: str):
#     df = load_and_structure(csv_file)

#     grouped = df.groupby(['aggregator_label','alignment_label'])[['baseline_cost','aggregated_cost']].mean().reset_index()

#     pivoted = grouped.pivot(index=['alignment_label'], columns=['aggregator_label',], values=['baseline_cost','aggregated_cost'])

#     pivoted.plot(kind='bar', figsize=(8,5))
#     plt.title("Baseline vs. Aggregated Cost\n(averaged by aggregator & alignment)")
#     plt.ylabel("Cost")
#     plt.tight_layout()
#     plt.show()


# def plot_scenario_time_vs_flexoffers(csv_file: str):
#     """
#     Plot how scenario_time changes as a function of NrOfFlexOffers.
#     Each bar is a scenario, grouped or colored by aggregator_type or alignment.
#     """
#     df = pd.read_csv(csv_file)

#     # Example approach: group by NrOfFlexOffers, aggregator_type, alignment
#     # Then average or sum the scenario_time
#     grouped = df.groupby(['NrOfFlexOffers', 'aggregator_type', 'alignment'], as_index=False)['scenario_time'].mean()

#     # We create a pivot table so each aggregator_type or alignment becomes a column
#     pivoted = grouped.pivot_table(index='NrOfFlexOffers',
#                                   columns=['aggregator_type', 'alignment'],
#                                   values='scenario_time',
#                                   aggfunc='mean')

#     fig, ax = plt.subplots(figsize=(8, 5))
#     pivoted.plot(kind='bar', ax=ax)
#     ax.set_xlabel("Number of FlexOffers")
#     ax.set_ylabel("Average Scenario Time (s)")
#     ax.set_title("Scenario Time vs. Number of FlexOffers (by aggregator_type & alignment)")
#     ax.legend(title="(aggregator_type, alignment)", bbox_to_anchor=(1.02, 1), loc='upper left')

#     plt.tight_layout()
#     plt.show()


# def plot_savings_vs_threshold(csv_file: str):
#     """
#     Plot how savings changes with est_threshold/lst_threshold.
#     We'll assume they move together (like your data),
#     then show alignment or aggregator_type on separate bars.
#     """
#     df = pd.read_csv(csv_file)

#     # Group by (est_threshold, aggregator_type) and get mean savings
#     grouped = df.groupby(['est_threshold', 'aggregator_type'], as_index=False)['savings'].mean()

#     fig, ax = plt.subplots(figsize=(8, 5))

#     # If your data always has matching est_threshold=lst_threshold, we can label it as 'threshold'
#     # or you can do multiple bars side by side if you want
#     for agg_type in sorted(grouped['aggregator_type'].unique()):
#         subset = grouped[grouped['aggregator_type'] == agg_type]
#         ax.bar(subset['est_threshold'] + 0.1*agg_type,  # slight offset for aggregator type
#                subset['savings'],
#                width=0.1,
#                label=f"aggregator_type={agg_type}")

#     ax.set_xlabel("Threshold (est_threshold)")
#     ax.set_ylabel("Avg Savings")
#     ax.set_title("Savings vs. est_threshold by aggregator_type")
#     ax.legend()
#     plt.xticks(sorted(grouped['est_threshold'].unique()))
#     plt.tight_layout()
#     plt.show()


# def plot_subplots_side_by_side(csv_file: str):
#     df = pd.read_csv(csv_file)

#     fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(12, 5), sharey=False)

#     alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
#     df['alignment_label'] = df['alignment'].map(alignment_map)

#     grouped_time = df.groupby('alignment_label')['scenario_time'].mean().reset_index()

#     ax1.bar(grouped_time['alignment_label'], grouped_time['scenario_time'], color='skyblue')
#     ax1.set_title("Avg Scenario Time by Alignment")
#     ax1.set_xlabel("Alignment")
#     ax1.set_ylabel("Scenario Time (s)")

#     # RIGHT SUBPLOT
#     # Average savings by alignment_label
#     grouped_savings = df.groupby('alignment_label')['savings'].mean().reset_index()
#     ax2.bar(grouped_savings['alignment_label'], grouped_savings['savings'], color='orange')
#     ax2.set_title("Avg Savings by Alignment")
#     ax2.set_xlabel("Alignment")
#     ax2.set_ylabel("Savings")

#     plt.tight_layout()
#     plt.show()


# def plot_scenario_time_and_savings(csv_file: str):
#     """
#     Example of side-by-side plots focusing on scenario_time and savings for
#     each scenario, possibly colored by aggregator_type or alignment.
#     """
#     df = pd.read_csv(csv_file)

#     # Sort by scenario_id just to have consistent ordering
#     df = df.sort_values(by='scenario_id')

#     fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(12, 5))
#     # Left: scenario_time
#     axs[0].bar(df['scenario_id'], df['scenario_time'], color='green')
#     axs[0].set_title("Scenario Time by Scenario ID")
#     axs[0].set_xlabel("Scenario ID")
#     axs[0].set_ylabel("Scenario Time (s)")

#     # Right: savings
#     axs[1].bar(df['scenario_id'], df['savings'], color='purple')
#     axs[1].set_title("Savings by Scenario ID")
#     axs[1].set_xlabel("Scenario ID")
#     axs[1].set_ylabel("Savings")

#     plt.tight_layout()
#     plt.show()


# def plot_savings_by_aggregator_type(csv_file: str):

#     df = load_and_structure(csv_file)

#     grouped = df.groupby(['aggregator_label','NrOfFlexOffers'])['savings'].mean().reset_index()
#     pivoted = grouped.pivot(index='NrOfFlexOffers', columns='aggregator_label', values='savings')

#     pivoted.plot(kind='bar', figsize=(7,5))
#     plt.xlabel("Number of FlexOffers")
#     plt.ylabel("Average Savings")
#     plt.title("Savings by Aggregator Type\n(varying NrOfFlexOffers)")
#     plt.legend(title='Aggregator Type')
#     plt.tight_layout()
#     plt.show()


# if __name__ == "__main__":
#     csv_path = "../data/stable_test_data.csv"

#     plot_time_vs_flexoffers(csv_path)
#     plot_alignment_savings(csv_path)
#     plot_baseline_vs_aggregated(csv_path)
#     plot_savings_by_aggregator_type(csv_path)
#     plot_scenario_time_vs_flexoffers(csv_path)
#     plot_savings_vs_threshold(csv_path)
#     plot_subplots_side_by_side(csv_path)
#     plot_scenario_time_and_savings(csv_path)




# import pandas as pd
# import matplotlib.pyplot as plt
# import numpy as np

# def load_data(csv_file: str) -> pd.DataFrame:
#     """
#     Loads the CSV into a DataFrame, does minimal mapping for aggregator/align numeric -> label,
#     sets up color mappings for aggregator_label, etc.
#     """
#     df = pd.read_csv(csv_file)

#     # Map aggregator_type numeric -> short label
#     agg_map = {0: 'Normal', 1: 'TEC', 2: 'Other?'}
#     df['aggregator_label'] = df['aggregator_type'].map(agg_map).fillna('Unknown')

#     # Map alignment numeric -> short label
#     align_map = {0: 'start', 1: 'balance', 2: 'price'}
#     df['alignment_label'] = df['alignment'].map(align_map).fillna('Other')

#     return df

# AGGREGATOR_COLORS = {
#     'SFO': '#1f77b4',  # skyblue-ish
#     'TECFO': '#ff7f0e',     # orange-ish
#     'DFO': '#2ca02c',  # green-ish
#     'Unknown': '#d62728'  # or any fallback
# }


# def plot_pivoted_bar(
#     ax, pivot_df: pd.DataFrame, x_label: str, y_label: str, title: str,
#     color_map: dict, legend_title: str = 'Aggregator'
# ):
#     col_labels = pivot_df.columns.tolist()
#     color_sequence = []
#     for col in col_labels:
#         if col in color_map:
#             color_sequence.append(color_map[col])
#         else:
#             color_sequence.append('#777777')  # fallback gray

#     pivot_df.plot(
#         kind='bar',
#         ax=ax,
#         color=color_sequence,
#         width=0.8
#     )

#     ax.set_xlabel(x_label)
#     ax.set_ylabel(y_label)
#     ax.set_title(title)
#     ax.legend(title=legend_title, fontsize=8)

# # 3) Figures
# def figure_one(df: pd.DataFrame):
#     """
#     2x2 figure focusing on scenario_time & savings wrt:
#     - row0 => NrOfFlexOffers
#     - row1 => max_group_size
#     - col0 => scenario_time
#     - col1 => savings
#     each pivoted by aggregator_label with consistent colors
#     """
#     fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(12, 8))
#     fig.suptitle("Figure 1: Scenario Time & Savings vs. #FlexOffers & max_group_size", fontsize=14, y=1.02)

#     # (0,0) scenario_time vs NrOfFlexOffers => pivot aggregator_label
#     ax = axes[0,0]
#     grouped = df.groupby(['NrOfFlexOffers','aggregator_label'], as_index=False)['scenario_time'].mean()
#     pivoted = grouped.pivot(index='NrOfFlexOffers', columns='aggregator_label', values='scenario_time')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='NrOfFlexOffers',
#                      y_label='Avg Scenario Time (s)',
#                      title='Scenario Time by #FlexOffers',
#                      color_map=AGGREGATOR_COLORS)

#     # (0,1) savings vs NrOfFlexOffers => pivot aggregator_label
#     ax = axes[0,1]
#     grouped = df.groupby(['NrOfFlexOffers','aggregator_label'], as_index=False)['savings'].mean()
#     pivoted = grouped.pivot(index='NrOfFlexOffers', columns='aggregator_label', values='savings')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='NrOfFlexOffers',
#                      y_label='Avg Savings',
#                      title='Savings by #FlexOffers',
#                      color_map=AGGREGATOR_COLORS)

#     # (1,0) scenario_time vs max_group_size => pivot aggregator_label
#     ax = axes[1,0]
#     grouped = df.groupby(['max_group_size','aggregator_label'], as_index=False)['scenario_time'].mean()
#     pivoted = grouped.pivot(index='max_group_size', columns='aggregator_label', values='scenario_time')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='max_group_size',
#                      y_label='Avg Scenario Time (s)',
#                      title='Scenario Time by max_group_size',
#                      color_map=AGGREGATOR_COLORS)

#     # (1,1) savings vs max_group_size => pivot aggregator_label
#     ax = axes[1,1]
#     grouped = df.groupby(['max_group_size','aggregator_label'], as_index=False)['savings'].mean()
#     pivoted = grouped.pivot(index='max_group_size', columns='aggregator_label', values='savings')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='max_group_size',
#                      y_label='Avg Savings',
#                      title='Savings by max_group_size',
#                      color_map=AGGREGATOR_COLORS)

#     plt.tight_layout()
#     plt.show()

# def figure_two(df: pd.DataFrame):
#     """
#     1 row, 2 subplots:
#     - Left: baseline_cost vs aggregator_label pivoted by alignment_label
#     - Right: aggregated_cost vs aggregator_label pivoted by alignment_label
#     """
#     fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(12,5))
#     fig.suptitle("Figure 2: Baseline & Aggregated Costs by Aggregator & Alignment", fontsize=14, y=1.02)

#     # Left subplot => baseline_cost vs aggregator_label pivoted by alignment_label
#     ax = axes[0]
#     grouped = df.groupby(['aggregator_label','alignment_label'], as_index=False)['baseline_cost'].mean()
#     pivoted = grouped.pivot(index='aggregator_label', columns='alignment_label', values='baseline_cost')
#     pivoted.plot(kind='bar', ax=ax, width=0.8)
#     ax.set_title("Mean Baseline Cost")
#     ax.set_xlabel("Aggregator Label")
#     ax.set_ylabel("Baseline Cost")
#     ax.legend(title='Alignment', fontsize=8)

#     # Right subplot => aggregated_cost vs aggregator_label pivoted by alignment_label
#     ax = axes[1]
#     grouped2 = df.groupby(['aggregator_label','alignment_label'], as_index=False)['aggregated_cost'].mean()
#     pivoted2 = grouped2.pivot(index='aggregator_label', columns='alignment_label', values='aggregated_cost')
#     pivoted2.plot(kind='bar', ax=ax, width=0.8, color=['#ff7f0e','#2ca02c','#1f77b4','#d62728'])
#     ax.set_title("Mean Aggregated Cost")
#     ax.set_xlabel("Aggregator Label")
#     ax.set_ylabel("Aggregated Cost")
#     ax.legend(title='Alignment', fontsize=8)

#     plt.tight_layout()
#     plt.show()

# def figure_three(df: pd.DataFrame):
#     """
#     1 row, 2 subplots:
#     - (0) scenario_time vs est_threshold pivot aggregator_label
#     - (1) savings vs est_threshold pivot aggregator_label
#     """
#     fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(12,5))
#     fig.suptitle("Figure 3: Impact of est_threshold on Time & Savings", fontsize=14, y=1.02)

#     # scenario_time vs est_threshold
#     ax = axes[0]
#     grouped = df.groupby(['est_threshold','aggregator_label'], as_index=False)['scenario_time'].mean()
#     pivoted = grouped.pivot(index='est_threshold', columns='aggregator_label', values='scenario_time')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='est_threshold',
#                      y_label='Avg Scenario Time (s)',
#                      title='Scenario Time by est_threshold',
#                      color_map=AGGREGATOR_COLORS)

#     # savings vs est_threshold
#     ax = axes[1]
#     grouped2 = df.groupby(['est_threshold','aggregator_label'], as_index=False)['savings'].mean()
#     pivoted2 = grouped2.pivot(index='est_threshold', columns='aggregator_label', values='savings')
#     plot_pivoted_bar(ax, pivoted2,
#                      x_label='est_threshold',
#                      y_label='Avg Savings',
#                      title='Savings by est_threshold',
#                      color_map=AGGREGATOR_COLORS)

#     plt.tight_layout()
#     plt.show()

# def figure_four(df: pd.DataFrame):
#     """
#     1 row, 2 subplots:
#     - (0) scenario_time vs alignment_label (group aggregator_label)
#     - (1) savings vs alignment_label (group aggregator_label)
#     """
#     fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(10,5))
#     fig.suptitle("Figure 4: Scenario Time & Savings by Alignment (colored aggregator)", fontsize=14, y=1.02)

#     # (0) scenario_time
#     ax = axes[0]
#     grouped = df.groupby(['alignment_label','aggregator_label'], as_index=False)['scenario_time'].mean()
#     pivoted = grouped.pivot(index='alignment_label', columns='aggregator_label', values='scenario_time')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='Alignment',
#                      y_label='Scenario Time (s)',
#                      title='Avg Scenario Time by Alignment',
#                      color_map=AGGREGATOR_COLORS)

#     # (1) savings
#     ax = axes[1]
#     grouped = df.groupby(['alignment_label','aggregator_label'], as_index=False)['savings'].mean()
#     pivoted = grouped.pivot(index='alignment_label', columns='aggregator_label', values='savings')
#     plot_pivoted_bar(ax, pivoted,
#                      x_label='Alignment',
#                      y_label='Savings',
#                      title='Avg Savings by Alignment',
#                      color_map=AGGREGATOR_COLORS)

#     plt.tight_layout()
#     plt.show()


# def figure_five(df: pd.DataFrame):
#     """
#     2x2 subplots: focusing on baseline_cost vs aggregator_label, aggregated_cost vs aggregator_label
#     scenario_time vs baseline_cost, scenario_time vs aggregated_cost in scatter form
#     """
#     fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(12,8))
#     fig.suptitle("Figure 5: Cost vs. Scenario Time Comparisons", fontsize=14, y=1.02)

#     ###################
#     # (0,0) baseline_cost vs aggregator_label => bar
#     ###################
#     ax = axes[0,0]
#     grouped = df.groupby('aggregator_label', as_index=False)['baseline_cost'].mean()
#     # We'll do a simple bar chart with consistent aggregator colors
#     aggregator_labels = grouped['aggregator_label'].tolist()
#     baseline_vals = grouped['baseline_cost'].tolist()
#     bar_colors = [AGGREGATOR_COLORS.get(lbl, '#777777') for lbl in aggregator_labels]
#     ax.bar(aggregator_labels, baseline_vals, color=bar_colors)
#     ax.set_title("Avg Baseline Cost by Aggregator")
#     ax.set_xlabel("Aggregator Label")
#     ax.set_ylabel("Baseline Cost")

#     ###################
#     # (0,1) aggregated_cost vs aggregator_label => bar
#     ###################
#     ax = axes[0,1]
#     grouped2 = df.groupby('aggregator_label', as_index=False)['aggregated_cost'].mean()
#     aggregator_labels2 = grouped2['aggregator_label'].tolist()
#     agg_vals = grouped2['aggregated_cost'].tolist()
#     bar_colors2 = [AGGREGATOR_COLORS.get(lbl, '#777777') for lbl in aggregator_labels2]
#     ax.bar(aggregator_labels2, agg_vals, color=bar_colors2)
#     ax.set_title("Avg Aggregated Cost by Aggregator")
#     ax.set_xlabel("Aggregator Label")
#     ax.set_ylabel("Aggregated Cost")

#     ###################
#     # (1,0) scenario_time vs baseline_cost => scatter
#     ###################
#     ax = axes[1,0]
#     # We can color each aggregator type differently:
#     for lbl, subdf in df.groupby('aggregator_label'):
#         ax.scatter(subdf['baseline_cost'], subdf['scenario_time'],
#                    alpha=0.7, label=lbl, color=AGGREGATOR_COLORS.get(lbl,'#777777'))
#     ax.set_title("Scenario Time vs. Baseline Cost")
#     ax.set_xlabel("Baseline Cost")
#     ax.set_ylabel("Scenario Time (s)")
#     ax.legend(title='Aggregator', fontsize=8)

#     ###################
#     # (1,1) scenario_time vs aggregated_cost => scatter
#     ###################
#     ax = axes[1,1]
#     for lbl, subdf in df.groupby('aggregator_label'):
#         ax.scatter(subdf['aggregated_cost'], subdf['scenario_time'],
#                    alpha=0.7, label=lbl, color=AGGREGATOR_COLORS.get(lbl,'#777777'))
#     ax.set_title("Scenario Time vs. Aggregated Cost")
#     ax.set_xlabel("Aggregated Cost")
#     ax.set_ylabel("Scenario Time (s)")

#     # We don’t create a new legend if we want the same aggregator legend => 
#     # or we can do ax.legend(...) again if we want a separate legend
#     ax.legend(title='Aggregator', fontsize=8)

#     plt.tight_layout()
#     plt.show()


# ###################################
# # 4) MAIN
# ###################################
# def main():
#     csv_path = "../data/stable_test_data.csv"  # Replace with your CSV path
#     df = load_data(csv_path)

#     # Possibly sort or filter
#     # df = df.sort_values(by='scenario_id')

#     figure_one(df)
#     figure_two(df)
#     figure_three(df)
#     figure_four(df)
#     figure_five(df)


# if __name__ == "__main__":
#     main()



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
    group_sizes = df['group_size_label'].unique()
    aggregators = df['aggregator_label'].unique()

    fig, axes = plt.subplots(
        nrows=len(group_sizes),
        ncols=len(aggregators),
        figsize=(5 * len(aggregators), 4 * len(group_sizes)),  # scale figure size
        sharey=True
    )
    fig.suptitle("Figure 1: Savings – Subplots by (Group Size x Aggregator)", fontsize=12)

    if len(aggregators) == 1:
        axes = [[ax] for ax in axes]

    # Now iterate over each row/col
    for row_idx, gsize in enumerate(group_sizes):
        for col_idx, agg_lbl in enumerate(aggregators):
            ax = axes[row_idx][col_idx]

            # Filter data for the current aggregator & group size
            subdf = df[(df['aggregator_label'] == agg_lbl) &
                       (df['group_size_label'] == gsize)]

            # Group by (threshold_label, alignment_label), then average 'savings'
            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False)['savings'].mean()

            # If there's no data for that combination, skip
            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            # Pivot so threshold_label -> x axis, alignment_label -> columns
            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='savings')

            pivoted.plot(kind='bar', ax=ax)

            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            if col_idx == 0:
                ax.set_ylabel("Avg Savings")
            ax.legend(fontsize=8)

    plt.tight_layout()
    plt.show()

def plot_scenario_time_grid(df: pd.DataFrame):

    group_sizes = sorted(df['group_size_label'].unique())
    aggregators = sorted(df['aggregator_label'].unique())

    fig, axes = plt.subplots(
        nrows=len(group_sizes),
        ncols=len(aggregators),
        figsize=(5 * len(aggregators), 4 * len(group_sizes)),
        sharey=True
    )
    fig.suptitle("Figure 2: Scenario Time Subplots by (Group Size x Aggregator)", fontsize=14)

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
            subdf = df[(df['aggregator_label'] == agg_lbl) &
                       (df['group_size_label'] == gsize)]

            grouped = subdf.groupby(['threshold_label','alignment_label'], as_index=False)['scenario_time'].mean()

            if grouped.empty:
                ax.set_title(f"{agg_lbl}, {gsize}\n(No data)")
                ax.set_xticks([])
                continue

            pivoted = grouped.pivot(index='threshold_label', columns='alignment_label', values='scenario_time')

            pivoted.plot(kind='bar', ax=ax)

            ax.set_title(f"{agg_lbl}, {gsize}")
            ax.set_xlabel("Threshold Label")
            if col_idx == 0:
                ax.set_ylabel("Scenario Time (s)")
            ax.legend(fontsize=8)

    plt.tight_layout()
    plt.show()

def main():
    csv_file = "../data/stable_test_data.csv"  # Replace with your actual CSV
    df = load_and_prepare_data(csv_file)

    # Figure 1: savings
    plot_savings_grid(df)

    # Figure 2: scenario_time
    plot_scenario_time_grid(df)

if __name__ == "__main__":
    main()
