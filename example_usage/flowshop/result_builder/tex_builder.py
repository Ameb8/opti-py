#!/usr/bin/env python3
"""
Generate professional LaTeX tables for algorithm comparison.
Creates separate, includable table files for each configuration.
Automatically generates main.tex with proper packages for cleveref, float, etc.

Usage:
    from generate_latex_tables_updated import generate_all_tables
    import pandas as pd
    
    df = pd.read_pickle('tmp_results')
    generate_all_tables(df, output_dir='tables')
    
    # This creates:
    # - tables/summary.tex
    # - tables/makespan_noblocking.tex
    # - tables/makespan_blocking.tex
    # - tables/tardiness_noblocking.tex
    # - tables/tardiness_blocking.tex
    # - main.tex (auto-generated)
    
Then compile with:
    pdflatex main.tex
    
Reference tables with \cref:
    \cref{tab:summary_best}
    \cref{tab:makespan_noblocking}
    etc.
"""

import pandas as pd
import numpy as np
from pathlib import Path
from typing import Optional


def format_algorithm_name(mutation: str, crossover: str) -> str:
    """Format algorithm name (mutation + crossover)."""
    return f"{mutation.upper()}/{crossover.upper()}"


def generate_comparison_table(
    df: pd.DataFrame,
    optimize_tardiness: bool,
    blocking: bool
) -> str:
    """
    Generate a single LaTeX table for given configuration (no document wrapper).
    
    Args:
        df: Full dataframe with all results
        optimize_tardiness: Whether we're optimizing for tardiness
        blocking: Whether blocking is enabled
    
    Returns:
        LaTeX table code as string (just the table, no document preamble)
    """
    
    # Filter data for this configuration
    subset = df[
        (df['optimize_tardiness'] == optimize_tardiness) &
        (df['blocking'] == blocking)
    ].copy()
    
    # Sort by the primary objective
    if optimize_tardiness:
        subset = subset.sort_values('tardiness', ascending=True)
    else:
        subset = subset.sort_values('makespan', ascending=True)
    
    # Reset rank based on sort
    subset['rank'] = range(1, len(subset) + 1)
    
    # Create display dataframe
    display = subset[['rank', 'mutation', 'crossover', 'makespan', 'makespan_std', 
                      'tardiness', 'exec_time']].copy()
    
    # Format values
    display['Algorithm'] = display.apply(
        lambda row: format_algorithm_name(row['mutation'], row['crossover']),
        axis=1
    )
    display['Makespan'] = display['makespan'].apply(lambda x: f"{x:,.0f}")
    display['MK±Std'] = display['makespan_std'].apply(lambda x: f"±{x:,.0f}" if x > 0 else "±0")
    display['Tardiness'] = display['tardiness'].apply(lambda x: f"{x:,.0f}")
    display['Time (s)'] = display['exec_time'].apply(lambda x: f"{x:.3f}")
    
    # Select columns for display
    table_data = display[['rank', 'Algorithm', 'Makespan', 'MK±Std', 'Tardiness', 'Time (s)']].reset_index(drop=True)
    
    # Create configuration label
    obj = "Tardiness" if optimize_tardiness else "Makespan"
    block = "with Blocking" if blocking else "without Blocking"
    caption = f"Algorithm Comparison: {obj} Optimization {block}"
    label = f"tab:{obj.lower()}_{('blocking' if blocking else 'noblocking').lower()}"
    
    # Build LaTeX table (standalone, includable)
    latex = f"""\\begin{{table}}[H]
\\centering
\\caption{{{caption}}}
\\label{{{label}}}
\\small
\\begin{{tblr}}{{
    colspec = {{r l r r r r}},
    row{{1}} = {{bg=headercolor, font=\\bfseries}},
    hlines,
    vlines,
}}
Rank & Algorithm & Makespan & ±Std & Tardiness & Time (s) \\\\
"""
    
    # Add data rows
    for idx, row in table_data.iterrows():
        latex += f"{row['rank']} & {row['Algorithm']} & {row['Makespan']} & {row['MK±Std']} & {row['Tardiness']} & {row['Time (s)']} \\\\\n"
    
    latex += """\\end{tblr}
\\end{table}
"""
    
    return latex


def generate_summary_table(df: pd.DataFrame) -> str:
    """
    Generate a summary table showing best algorithm for each configuration (no wrapper).
    """
    
    configs = [
        (False, False, "Makespan, No Blocking"),
        (False, True, "Makespan, Blocking"),
        (True, False, "Tardiness, No Blocking"),
        (True, True, "Tardiness, Blocking"),
    ]
    
    rows = []
    
    for opt_tard, block, config_name in configs:
        subset = df[
            (df['optimize_tardiness'] == opt_tard) &
            (df['blocking'] == block)
        ]
        
        # Find best by primary objective
        if opt_tard:
            best_idx = subset['tardiness'].idxmin()
            best_val = subset.loc[best_idx, 'tardiness']
        else:
            best_idx = subset['makespan'].idxmin()
            best_val = subset.loc[best_idx, 'makespan']
        
        best_row = subset.loc[best_idx]
        alg = format_algorithm_name(best_row['mutation'], best_row['crossover'])
        
        rows.append({
            'Configuration': config_name,
            'Best Algorithm': alg,
            'Objective Value': f"{best_val:,.0f}"
        })
    
    summary_df = pd.DataFrame(rows)
    
    latex = r"""\begin{table}[H]
\centering
\caption{Best Algorithm per Configuration}
\label{tab:summary_best}
\small
\begin{tblr}{
    colspec = {l l r},
    row{1} = {bg=headercolor, font=\bfseries},
    hlines,
    vlines,
}
Configuration & Best Algorithm & Objective Value \\
"""
    
    for _, row in summary_df.iterrows():
        latex += f"{row['Configuration']} & {row['Best Algorithm']} & {row['Objective Value']} \\\\\n"
    
    latex += """\end{tblr}
\end{table}
"""
    
    return latex


def generate_main_tex(output_dir: str = 'tables', main_tex_path: Optional[str] = None, verbose: bool = True) -> None:
    """
    Generate a main.tex file that includes all generated tables.
    
    Includes all required packages:
    - tabularray: Modern table formatting
    - xcolor: Color support for table headers
    - float: [H] float placement
    - booktabs: Professional table lines
    - hyperref: Hyperlinks and PDF features
    - cleveref: Smart cross-references (\cref)
    
    Args:
        output_dir: Directory where tables are stored (default: 'tables')
        main_tex_path: Path where main.tex will be created. If None, creates in parent of output_dir.
    """
    
    if main_tex_path is None:
        main_tex_path = Path(output_dir).parent / 'main.tex'
    else:
        main_tex_path = Path(main_tex_path)
    
    # LaTeX preamble with all required packages
    main_tex_content = r"""\documentclass{article}
\usepackage{geometry}
\geometry{margin=1in}

% Tables
\usepackage{tabularray}
\usepackage{xcolor}
\usepackage{float}
\usepackage{booktabs}
\UseTblrLibrary{booktabs}

% Cross-references and hyperlinks
\usepackage{hyperref}
\usepackage[capitalize]{cleveref}

% Define header color
\definecolor{headercolor}{gray}{0.9}

\title{Algorithm Comparison Results}
\author{}
\date{\today}

\begin{document}

\maketitle

\listoftables
\clearpage

\section{Summary}
\input{tables/summary.tex}

\section{Makespan Optimization}
\subsection{Without Blocking}
\input{tables/makespan_noblocking.tex}

\subsection{With Blocking}
\input{tables/makespan_blocking.tex}

\section{Tardiness Optimization}
\subsection{Without Blocking}
\input{tables/tardiness_noblocking.tex}

\subsection{With Blocking}
\input{tables/tardiness_blocking.tex}

\end{document}
"""
    
    with open(main_tex_path, 'w') as f:
        f.write(main_tex_content)
    
    if verbose:
        print(f"Generated main.tex at {main_tex_path}")


def generate_tex(
    df: pd.DataFrame,
    output_dir: str = 'tables',
    generate_main: bool = True,
    verbose: bool = True
) -> None:
    """
    Generate all LaTeX tables as individual includable files.
    
    Args:
        df: DataFrame with algorithm comparison results
        output_dir: Directory to save .tex files (created if doesn't exist)
        generate_main: If True, also generate main.tex file (default: True)
        verbose: If True, print status messages (default: True)
    """
    
    # Create output directory
    output_path = Path(output_dir)
    output_path.mkdir(exist_ok=True)
    
    if verbose:
        print(f"Generating tables in '{output_dir}/'...")
    
    # 1. Summary table
    summary_tex = generate_summary_table(df)
    summary_file = output_path / 'summary.tex'
    with open(summary_file, 'w') as f:
        f.write(summary_tex)
    if verbose:
        print(f"{summary_file} created")
    
    # 2-5. Configuration-specific tables
    configs = [
        (False, False, 'makespan_noblocking'),
        (False, True, 'makespan_blocking'),
        (True, False, 'tardiness_noblocking'),
        (True, True, 'tardiness_blocking'),
    ]
    
    for opt_tard, block, filename in configs:
        table_tex = generate_comparison_table(df, opt_tard, block)
        table_file = output_path / f'{filename}.tex'
        with open(table_file, 'w') as f:
            f.write(table_tex)
        if verbose:
            print(f"{table_file} created")
    
    # Generate main.tex if requested
    if generate_main:
        generate_main_tex(output_dir)
    


