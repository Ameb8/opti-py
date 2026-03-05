import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from pathlib import Path


def makespan_heatmap(data: pd.DataFrame, plot_dir: Path) -> None:
    for blocking in [True, False]:
        subset = data[data["blocking"] == blocking]

        pivot = subset.pivot_table(
            values="makespan",
            index="num_machines",
            columns="num_jobs",
            aggfunc="mean"
        )

        plt.figure(figsize=(8,6))

        sns.heatmap(
            pivot,
            cmap="viridis",
            annot=True,
            fmt=".1f",
            cbar=True
        )

        plt.title(f"Makespan Heatmap (blocking={blocking})")
        plt.xlabel("Number of Jobs")
        plt.ylabel("Number of Machines")

        plt.tight_layout()

        filename = f"makespan_heatmap_blocking_{blocking}.png"
        plt.savefig(plot_dir / filename, dpi=300, bbox_inches="tight")
        plt.close()


def bar_plot(
    data: pd.DataFrame,
    plot_dir: Path,
    x: str,
    y: str,
    hue: str,
    title: str,
    xlabel: str,
    ylabel: str,
    filename: str,
    errorbar="sd",
    figsize=(8,5),
    ylim_bottom=0
) -> None:
    sns.set_theme(
        style="whitegrid",
        context="talk",
        palette="deep" 
    )
    
    plt.figure(figsize=figsize)

    sns.barplot(
        data=data,
        x=x,
        y=y,
        hue=hue,
        estimator="mean",
        errorbar=errorbar, 
        edgecolor="black",
        capsize=0.1
    )

    if ylim_bottom is not None:
        plt.ylim(bottom=ylim_bottom)

    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

    plt.tight_layout()
    plt.savefig(plot_dir / filename, dpi=300, bbox_inches="tight")
    plt.close()


def build_plots(data: pd.DataFrame, plot_dir: Path) -> None:
    plot_dir.mkdir(parents=True, exist_ok=True)
    makespan_heatmap(data, plot_dir)

     # Execution time by num machines plot
    bar_plot(
        data=data,
        plot_dir=plot_dir,
        x="num_machines",
        y="exec_time",
        hue="blocking",
        title="Execution Time vs Number of Machines",
        xlabel="Number of Machines",
        ylabel="Mean Execution Time",
        filename="machines_vs_exec_time.png"
    )

    # Execution time by num jobs
    bar_plot(
        data=data,
        plot_dir=plot_dir,
        x="num_jobs",
        y="exec_time",
        hue="blocking",
        title="Execution Time vs Number of Jobs",
        xlabel="Number of Jobs",
        ylabel="Mean Execution Time",
        filename="jobs_vs_exec_time.png"
    )