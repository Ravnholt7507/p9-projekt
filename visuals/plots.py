import pandas as pd
import matplotlib.pyplot as plt

def load_and_structure(csv_file: str) -> pd.DataFrame:
    df = pd.read_csv(csv_file)

    aggregator_map = {0: 'Normal', 1: 'TEC', 2: 'DFO'}
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

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def plot_threshold_effects_side_by_side(
    csv_path: str = None,
    df: pd.DataFrame = None,
    show_plots: bool = True
):
    """
    Creates a single figure with bar charts showing how est_threshold,
    lst_threshold, and max_group_size affect scenario_time and savings.

    Plots are arranged in a 2x3 grid:
    - Top row: scenario_time by (est_threshold, lst_threshold, max_group_size)
    - Bottom row: savings by (est_threshold, lst_threshold, max_group_size)

    Parameters
    ----------
    csv_path : str, optional
        Path to the CSV file containing the data. If provided, it will be
        used to read the data into a DataFrame.
    df : pd.DataFrame, optional
        A DataFrame containing the data. If provided, csv_path is ignored.
    show_plots : bool, optional
        If True, display the plot on screen (default True).
    """

    # 1) Read the data if df is not already provided
    if df is None:
        if csv_path is None:
            raise ValueError("Either 'csv_path' or 'df' must be provided.")
        df = pd.read_csv(csv_path)

    # (Optional) Convert columns to numeric if needed
    numeric_cols = [
        "scenario_id", "aggregator_type", "alignment", "est_threshold",
        "lst_threshold", "max_group_size", "baseline_cost", "aggregated_cost",
        "savings", "scenario_time", "NrOfFlexOffers"
    ]
    for col in numeric_cols:
        df[col] = pd.to_numeric(df[col], errors='coerce')

    # Set a simple Seaborn style
    sns.set(style="whitegrid")

    # -------------------------------------------------------------------------
    # Group the data by each threshold and get mean scenario_time / savings
    # -------------------------------------------------------------------------
    df_est = df.groupby("est_threshold", as_index=False)[["scenario_time", "savings"]].mean()
    df_lst = df.groupby("lst_threshold", as_index=False)[["scenario_time", "savings"]].mean()
    df_mgs = df.groupby("max_group_size", as_index=False)[["scenario_time", "savings"]].mean()

    # -------------------------------------------------------------------------
    # Create one figure with 6 subplots (2 rows x 3 columns)
    # -------------------------------------------------------------------------
    fig, axes = plt.subplots(nrows=2, ncols=3, figsize=(15, 8), sharey=False)

    # Row 0: scenario_time
    sns.barplot(data=df_est, x="est_threshold", y="scenario_time", ax=axes[0,0], color="skyblue")
    axes[0,0].set_title("Scenario Time by est_threshold")
    axes[0,0].set_ylabel("Mean Scenario Time")

    sns.barplot(data=df_lst, x="lst_threshold", y="scenario_time", ax=axes[0,1], color="skyblue")
    axes[0,1].set_title("Scenario Time by lst_threshold")
    axes[0,1].set_ylabel("Mean Scenario Time")

    sns.barplot(data=df_mgs, x="max_group_size", y="scenario_time", ax=axes[0,2], color="skyblue")
    axes[0,2].set_title("Scenario Time by max_group_size")
    axes[0,2].set_ylabel("Mean Scenario Time")

    # Row 1: savings
    sns.barplot(data=df_est, x="est_threshold", y="savings", ax=axes[1,0], color="lightgreen")
    axes[1,0].set_title("Savings by est_threshold")
    axes[1,0].set_ylabel("Mean Savings")

    sns.barplot(data=df_lst, x="lst_threshold", y="savings", ax=axes[1,1], color="lightgreen")
    axes[1,1].set_title("Savings by lst_threshold")
    axes[1,1].set_ylabel("Mean Savings")

    sns.barplot(data=df_mgs, x="max_group_size", y="savings", ax=axes[1,2], color="lightgreen")
    axes[1,2].set_title("Savings by max_group_size")
    axes[1,2].set_ylabel("Mean Savings")

    plt.tight_layout()

    # Show the plots
    if show_plots:
        plt.show()

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def plot_savings_by_alignment(
    csv_path: str = None,
    df: pd.DataFrame = None,
    show_plots: bool = True
):
    # 1) Read the data if df not provided
    df = load_and_structure(csv_path)
    if df is None:
        if csv_path is None:
            raise ValueError("Either 'csv_path' or 'df' must be provided.")
        df = pd.read_csv(csv_path)
        

    # (Optional) Convert columns to numeric if needed
    # numeric_cols = [
    #     "scenario_id", "aggregator_type", "alignment", "est_threshold",
    #     "lst_threshold", "max_group_size", "baseline_cost", "aggregated_cost",
    #     "savings", "scenario_time", "NrOfFlexOffers"
    # ]
    # for col in numeric_cols:
    #     df[col] = pd.to_numeric(df[col], errors="coerce")

    # Group data by alignment and compute mean savings
    df_align = df.groupby("alignment_label", as_index=False)["savings"].mean()

    # Set a simple style
    sns.set(style="whitegrid")

    # 2) Create a single bar chart (alignment vs. mean savings)
    plt.figure(figsize=(6, 4))
    sns.barplot(data=df_align, x="alignment_label", y="savings", color="skyblue")
    plt.title("Mean Savings by Alignment")
    plt.xlabel("Alignment")
    plt.ylabel("Mean Savings")

    # 3) Show the plot
    if show_plots:
        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
    csv_path = "../data/economic_savings.csv"

    # plot_time_vs_flexoffers(csv_path)
    # plot_alignment_savings(csv_path)
    # plot_baseline_vs_aggregated(csv_path)
    # plot_savings_by_aggregator_type(csv_path)
    plot_savings_by_alignment(csv_path)