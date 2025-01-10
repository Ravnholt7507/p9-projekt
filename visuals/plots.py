import pandas as pd
import matplotlib.pyplot as plt

def load_and_structure(csv_file: str) -> pd.DataFrame:
    df = pd.read_csv(csv_file)

    aggregator_map = {0: 'Normal', 1: 'TEC', 2: 'Other?'}
    if 'aggregator_type' in df.columns:
        df['aggregator_label'] = df['aggregator_type'].map(aggregator_map).fillna('Unknown')


    alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
    if 'alignment' in df.columns:
        df['alignment_label'] = df['alignment'].map(alignment_map).fillna('Unknown')

    return df


def plot_time_vs_flexoffers(csv_file: str):
    df = load_and_structure(csv_file)

    grouped = df.groupby(['NrOfFlexOffers','aggregator_label'])['scenario_time'].mean().reset_index()

    pivoted = grouped.pivot(index='NrOfFlexOffers', columns=['aggregator_label'], values='scenario_time')
    pivoted.plot(kind='bar', width=0.75, figsize=(7,5), color=['skyblue','orange','green'])

    plt.xlabel("Number of FlexOffers")
    plt.ylabel("Average Scenario Time (s)")
    plt.title("Scenario Time vs. Number of FlexOffers\n(grouped by Aggregator Type)")
    plt.legend(title='Aggregator Type')
    plt.tight_layout()
    plt.show()


def plot_alignment_savings(csv_file: str):
    df = load_and_structure(csv_file)

    grouped = df.groupby(['alignment_label','aggregator_label'])['savings'].mean().reset_index()

    pivoted = grouped.pivot(index='alignment_label', columns='aggregator_label', values='savings')
    pivoted.plot(kind='bar', figsize=(7,5))

    plt.xlabel("Alignment Method")
    plt.ylabel("Average Savings")
    plt.title("Savings by Alignment, grouped by Aggregator Type")
    plt.legend(title='Aggregator')
    plt.tight_layout()
    plt.show()

def plot_baseline_vs_aggregated(csv_file: str):
    df = load_and_structure(csv_file)

    grouped = df.groupby(['aggregator_label','alignment_label'])[['baseline_cost','aggregated_cost']].mean().reset_index()

    pivoted = grouped.pivot(index=['alignment_label'], columns=['aggregator_label',], values=['baseline_cost','aggregated_cost'])

    pivoted.plot(kind='bar', figsize=(8,5))
    plt.title("Baseline vs. Aggregated Cost\n(averaged by aggregator & alignment)")
    plt.ylabel("Cost")
    plt.tight_layout()
    plt.show()


def plot_scenario_time_vs_flexoffers(csv_file: str):
    """
    Plot how scenario_time changes as a function of NrOfFlexOffers.
    Each bar is a scenario, grouped or colored by aggregator_type or alignment.
    """
    df = pd.read_csv(csv_file)

    # Example approach: group by NrOfFlexOffers, aggregator_type, alignment
    # Then average or sum the scenario_time
    grouped = df.groupby(['NrOfFlexOffers', 'aggregator_type', 'alignment'], as_index=False)['scenario_time'].mean()

    # We create a pivot table so each aggregator_type or alignment becomes a column
    pivoted = grouped.pivot_table(index='NrOfFlexOffers',
                                  columns=['aggregator_type', 'alignment'],
                                  values='scenario_time',
                                  aggfunc='mean')

    fig, ax = plt.subplots(figsize=(8, 5))
    pivoted.plot(kind='bar', ax=ax)
    ax.set_xlabel("Number of FlexOffers")
    ax.set_ylabel("Average Scenario Time (s)")
    ax.set_title("Scenario Time vs. Number of FlexOffers (by aggregator_type & alignment)")
    ax.legend(title="(aggregator_type, alignment)", bbox_to_anchor=(1.02, 1), loc='upper left')

    plt.tight_layout()
    plt.show()


def plot_savings_vs_threshold(csv_file: str):
    """
    Plot how savings changes with est_threshold/lst_threshold.
    We'll assume they move together (like your data),
    then show alignment or aggregator_type on separate bars.
    """
    df = pd.read_csv(csv_file)

    # Group by (est_threshold, aggregator_type) and get mean savings
    grouped = df.groupby(['est_threshold', 'aggregator_type'], as_index=False)['savings'].mean()

    fig, ax = plt.subplots(figsize=(8, 5))

    # If your data always has matching est_threshold=lst_threshold, we can label it as 'threshold'
    # or you can do multiple bars side by side if you want
    for agg_type in sorted(grouped['aggregator_type'].unique()):
        subset = grouped[grouped['aggregator_type'] == agg_type]
        ax.bar(subset['est_threshold'] + 0.1*agg_type,  # slight offset for aggregator type
               subset['savings'],
               width=0.1,
               label=f"aggregator_type={agg_type}")

    ax.set_xlabel("Threshold (est_threshold)")
    ax.set_ylabel("Avg Savings")
    ax.set_title("Savings vs. est_threshold by aggregator_type")
    ax.legend()
    plt.xticks(sorted(grouped['est_threshold'].unique()))
    plt.tight_layout()
    plt.show()


def plot_subplots_side_by_side(csv_file: str):
    """
    Create a single figure with multiple subplots, side by side or in a grid,
    so you can visually compare scenario_time, savings, and aggregator_type, etc.
    """
    df = pd.read_csv(csv_file)

    # Example: We'll do 1 row, 2 columns
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(12, 5), sharey=False)

    # Left subplot: scenario_time vs. alignment
    # Right subplot: savings vs. alignment

    # Convert numeric alignment codes to string labels
    alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
    df['alignment_label'] = df['alignment'].map(alignment_map)

    # LEFT SUBPLOT
    # Average scenario_time by alignment_label
    grouped_time = df.groupby('alignment_label')['scenario_time'].mean().reset_index()

    ax1.bar(grouped_time['alignment_label'], grouped_time['scenario_time'], color='skyblue')
    ax1.set_title("Avg Scenario Time by Alignment")
    ax1.set_xlabel("Alignment")
    ax1.set_ylabel("Scenario Time (s)")

    # RIGHT SUBPLOT
    # Average savings by alignment_label
    grouped_savings = df.groupby('alignment_label')['savings'].mean().reset_index()
    ax2.bar(grouped_savings['alignment_label'], grouped_savings['savings'], color='orange')
    ax2.set_title("Avg Savings by Alignment")
    ax2.set_xlabel("Alignment")
    ax2.set_ylabel("Savings")

    plt.tight_layout()
    plt.show()


def plot_scenario_time_and_savings(csv_file: str):
    """
    Example of side-by-side plots focusing on scenario_time and savings for
    each scenario, possibly colored by aggregator_type or alignment.
    """
    df = pd.read_csv(csv_file)

    # Sort by scenario_id just to have consistent ordering
    df = df.sort_values(by='scenario_id')

    fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(12, 5))
    # Left: scenario_time
    axs[0].bar(df['scenario_id'], df['scenario_time'], color='green')
    axs[0].set_title("Scenario Time by Scenario ID")
    axs[0].set_xlabel("Scenario ID")
    axs[0].set_ylabel("Scenario Time (s)")

    # Right: savings
    axs[1].bar(df['scenario_id'], df['savings'], color='purple')
    axs[1].set_title("Savings by Scenario ID")
    axs[1].set_xlabel("Scenario ID")
    axs[1].set_ylabel("Savings")

    plt.tight_layout()
    plt.show()


def plot_savings_by_aggregator_type(csv_file: str):

    df = load_and_structure(csv_file)

    grouped = df.groupby(['aggregator_label','NrOfFlexOffers'])['savings'].mean().reset_index()
    pivoted = grouped.pivot(index='NrOfFlexOffers', columns='aggregator_label', values='savings')

    pivoted.plot(kind='bar', figsize=(7,5))
    plt.xlabel("Number of FlexOffers")
    plt.ylabel("Average Savings")
    plt.title("Savings by Aggregator Type\n(varying NrOfFlexOffers)")
    plt.legend(title='Aggregator Type')
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    csv_path = "../data/economic_savings.csv"

    plot_time_vs_flexoffers(csv_path)
    plot_alignment_savings(csv_path)
    plot_baseline_vs_aggregated(csv_path)
    plot_savings_by_aggregator_type(csv_path)
    plot_scenario_time_vs_flexoffers(csv_path)
    plot_savings_vs_threshold(csv_path)
    plot_subplots_side_by_side(csv_path)
    plot_scenario_time_and_savings(csv_path)
