import pandas as pd
import matplotlib.pyplot as plt

def plot_time_vs_flexoffers(csv_file: str):

    df = pd.read_csv(csv_file)

    plt.figure(figsize=(6, 4))
    plt.bar(df['NrOfFlexOffers'].astype(str), df['scenario_time'], color='skyblue')
    plt.xlabel("Number of FlexOffers")
    plt.ylabel("Scenario Time (seconds?)")
    plt.title("Scenario Time vs. Number of FlexOffers")
    plt.tight_layout()
    plt.show()

def plot_alignment_savings(csv_file: str):

    df = pd.read_csv(csv_file)
    
    alignment_map = {0: 'start', 1: 'balance', 2: 'price'}
    df['alignment_label'] = df['alignment'].map(alignment_map)
    
    plt.figure(figsize=(6,4))
    plt.bar(df['alignment_label'].astype(str), df['savings'], color='orange')
    plt.xlabel("Alignment Method")
    plt.ylabel("Savings")
    plt.title("Savings by Alignment")
    plt.tight_layout()
    plt.show()

def plot_baseline_vs_aggregated(csv_file: str):

    df = pd.read_csv(csv_file)

    plt.figure(figsize=(6, 4))
    x_vals = range(len(df))
    width = 0.4
    
    plt.bar([x - width/2 for x in x_vals], df['baseline_cost'], 
            width=width, label='Baseline Cost', color='grey')
    plt.bar([x + width/2 for x in x_vals], df['aggregated_cost'], 
            width=width, label='Aggregated Cost', color='green')
    
    plt.xlabel("Scenario Index")
    plt.ylabel("Cost")
    plt.title("Baseline vs. Aggregated Cost")
    plt.legend()
    plt.tight_layout()
    plt.show()

def plot_savings_by_aggregator_type(csv_file: str):

    df = pd.read_csv(csv_file)

    grouped = df.groupby('aggregator_type')['savings'].mean().reset_index()
    
    plt.figure(figsize=(6, 4))
    plt.bar(grouped['aggregator_type'].astype(str), grouped['savings'], color='purple')
    plt.xlabel("Aggregator Type (0=Normal, 1=TEC)")
    plt.ylabel("Avg Savings")
    plt.title("Savings by Aggregator Type")
    plt.tight_layout()
    plt.show()

def plot_scenario_time_over_savings(csv_file: str):
    df = pd.read_csv(csv_file)
    
    plt.figure(figsize=(6, 4))
    plt.scatter(df['scenario_time'], df['savings'], color='red')
    plt.xlabel("Scenario Time")
    plt.ylabel("Savings")
    plt.title("Scenario Time vs. Savings")
    plt.tight_layout()
    plt.show()

# Example usage:
if __name__ == "__main__":
    csv_path = "../data/economic_savings.csv"
    
    plot_time_vs_flexoffers(csv_path)
    plot_alignment_savings(csv_path)
    plot_baseline_vs_aggregated(csv_path)
    plot_savings_by_aggregator_type(csv_path)
    plot_scenario_time_over_savings(csv_path)
