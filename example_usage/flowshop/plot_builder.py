import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from pathlib import Path
from itertools import product


def compare_algorithms(
    df: pd.DataFrame,
    plot_dir: Path,
    blocking: bool,
    optimize_tardiness: bool,
    metric: str = 'auto',
    figsize: tuple = (12, 6)
) -> None:
    # Filter data for this combo
    df_filtered = df[
        (df['blocking'] == blocking) & 
        (df['optimize_tardiness'] == optimize_tardiness)
    ].copy()
    
    if df_filtered.empty:
        print(f"  No data for blocking={blocking}, optimize_tardiness={optimize_tardiness}")
        return
    
    # Determine which metric to use
    if metric == 'auto':
        if optimize_tardiness:
            metric = 'tardiness'
            metric_display = 'Tardiness'
        else:
            metric = 'makespan'
            metric_display = 'Makespan'
    elif metric == 'exec_time':
        metric_display = 'Execution Time (s)'
    else:
        metric_display = metric.replace('_', ' ').title()
    
    # Validate metric exists
    if metric not in df_filtered.columns:
        raise ValueError(f"Column '{metric}' not found in DataFrame")
    
    # Create combination column for x-axis
    df_filtered['algorithm'] = df_filtered['mutation'] + ' / ' + df_filtered['crossover']
    
    # Get std column name
    std_col = f'{metric}_std'
    
    # Setup plot
    sns.set_theme(style="whitegrid", context="talk", palette="deep")
    fig, ax = plt.subplots(figsize=figsize)
    
    # Create bar plot
    x_positions = range(len(df_filtered))
    bars = ax.bar(x_positions, df_filtered[metric], color='steelblue', edgecolor='black', linewidth=1.5)
    
    # Add error bars if std exists and is non-zero
    if std_col in df_filtered.columns:
        yerr = df_filtered[std_col]
        # Only add error bars where std > 0
        if (yerr > 0).any():
            ax.errorbar(
                x_positions,
                df_filtered[metric],
                yerr=yerr,
                fmt='none',
                color='black',
                capsize=5,
                linewidth=2,
                capthick=2
            )
    
    # Set y-axis limits to zoom in on data range
    min_val = df_filtered[metric].min()
    max_val = df_filtered[metric].max()
    margin = (max_val - min_val) * 0.1  # 10% margin above/below
    
    ax.set_ylim(min_val - margin, max_val + margin)
    
    # Labels and title
    blocking_str = "blocking" if blocking else "no blocking"
    tardiness_str = "optimize tardiness" if optimize_tardiness else "no tardiness opt"
    title = f'Algorithm Comparison - {metric_display} ({blocking_str}, {tardiness_str})'
    
    ax.set_title(title, fontsize=14, fontweight='bold')
    ax.set_xlabel('Mutation / Crossover', fontsize=12)
    ax.set_ylabel(metric_display, fontsize=12)
    ax.set_xticks(x_positions)
    ax.set_xticklabels(df_filtered['algorithm'], rotation=45, ha='right')
    
    plt.tight_layout()
    
    # Save with descriptive filename
    blocking_label = "blocked" if blocking else "unblocked"
    tardiness_label = "tardiness" if optimize_tardiness else "no_tardiness"
    filename = f'algorithm_comparison_{blocking_label}_{tardiness_label}_{metric}.png'
    
    plt.savefig(plot_dir / filename, dpi=300, bbox_inches="tight")
    plt.close()
    
    print(f"  Saved: {filename}")


def build_plots(data: pd.DataFrame, plot_dir: Path) -> None:
    plot_dir.mkdir(parents=True, exist_ok=True)
    
    print("Building algorithm comparison plots...")
    
    # Generate plots for all 4 combinations (FF, FT, TF, TT)
    for blocking, optimize_tardiness in product([False, True], repeat=2):
        config_str = f"blocking={blocking}, optimize_tardiness={optimize_tardiness}"
        
        # Plot 1: auto-selected metric (tardiness if optimize_tardiness, else makespan)
        print(f"  {config_str} → makespan/tardiness")
        compare_algorithms(data, plot_dir, blocking, optimize_tardiness, metric='auto')
        
        # Plot 2: execution time
        print(f"  {config_str} → exec_time")
        compare_algorithms(data, plot_dir, blocking, optimize_tardiness, metric='exec_time')
    
    print(f"✓ All plots saved to {plot_dir}")

