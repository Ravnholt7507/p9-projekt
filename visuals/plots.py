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
