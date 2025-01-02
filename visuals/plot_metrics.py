#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt

def main():
    # 1) Read CSV
    df = pd.read_csv('../data/metrics.csv')  
    # Expect columns: Hour,SpotPrice,WithFlexPower,WithFlexCost

    hours = df['Hour']
    spot_price = df['SpotPrice']
    with_flex_power = df['WithFlexPower']
    with_flex_cost = df['WithFlexCost']

    # 2) Plot Spot Price + withFlexPower on dual axis
    fig, ax1 = plt.subplots(figsize=(10,5))

    color1 = 'tab:blue'
    ax1.set_xlabel('Hour')
    ax1.set_ylabel('Spot Price [EUR/MWh]', color=color1)
    ax1.plot(hours, spot_price, color=color1, marker='o', label='Spot Price')
    ax1.tick_params(axis='y', labelcolor=color1)

    ax2 = ax1.twinx()
    color2 = 'tab:red'
    ax2.set_ylabel('With Flex Power [kW]', color=color2)
    ax2.plot(hours, with_flex_power, color=color2, marker='s', label='With Flex')
    ax2.tick_params(axis='y', labelcolor=color2)

    # Combine legends
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines1 + lines2, labels1 + labels2, loc='upper right')

    plt.title('Spot Price & Scheduled Power (With FlexOffers)')
    plt.tight_layout()
    plt.show()

    # 3) Cumulative Cost Plot
    df['CumulativeWithFlex'] = df['WithFlexCost'].cumsum()
    
    plt.figure(figsize=(8,4))
    plt.plot(df['Hour'], df['CumulativeWithFlex'], marker='o', label='With Flex (Cumulative)')
    plt.xlabel('Hour')
    plt.ylabel('Cumulative Cost [EUR]')
    plt.title('Cumulative Cost Throughout the Day (With Flex)')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
