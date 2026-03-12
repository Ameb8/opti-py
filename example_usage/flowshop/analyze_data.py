import pandas as pd
import numpy as np


def average_lists(list_series):
    lists = list_series.tolist()

    # Not a list, return first value
    if not lists or not isinstance(lists[0], list):
        return lists[0]
    
    # Recursively average nested structures
    def recursive_average(nested_lists):
        if not isinstance(nested_lists[0], list):
            # Base case: numeric values
            return np.mean(nested_lists)
        else: # Recursive case: list of lists
            return [recursive_average([item[i] for item in nested_lists]) 
                    for i in range(len(nested_lists[0]))]
    
    return recursive_average(lists)



def average_seeds(df: pd.DataFrame, config_cols: list[str] = None) -> pd.DataFrame:
    # Auto-detect config columns if not provided
    if config_cols is None:
        # Result columns to exclude from grouping
        result_cols = {
            'sequence',
            'makespan',
            'completion_times',
            'completionTimes'
            'exec_time',
            'tardiness',
            'fitnesses', 
            'jobs',
            'due_dates'
            'seed'
        }
        config_cols = [col for col in df.columns if col not in result_cols]


    # Columns to compute standard deviation for
    std_cols = {'makespan', 'exec_time'}    

    # Build aggregation dict for result columns
    agg_dict = {}
    for col in df.columns:
        # Skip config and seed columns
        if col in config_cols or col == 'seed':
            continue
        
        # Check column data type
        sample_val = df[col].iloc[0]
        if isinstance(sample_val, list): # Average lists element-wise
            agg_dict[col] = average_lists
        elif df[col].dtype in ['float64', 'float32', 'int64', 'int32', 'int16', 'int8']:
            agg_dict[col] = 'mean' # Average numerical columns
        else: # Take first value for non-numeric columns
            agg_dict[col] = 'first'
    
    # Group by config columns only and aggregate
    averaged_df = df.groupby(config_cols, as_index=False).agg(agg_dict)
    
    # Add standard deviation columns
    for col in std_cols:
        if col in df.columns:
            # Group and compute std dev
            std_col_name = f'{col}_std'
            grouped = df.groupby(config_cols)[col].apply(np.std).reset_index()
            grouped.rename(columns={col: std_col_name}, inplace=True)
            averaged_df = averaged_df.merge(grouped, on=config_cols, how='left')
    

    return averaged_df

def sum_list_values(list_series):
    """Sum all numeric values within lists across the series."""
    lists = list_series.tolist()
    if not lists or not isinstance(lists[0], list):
        return 0
    
    # Recursively sum nested structures
    def recursive_sum(nested_lists):
        if not isinstance(nested_lists[0], list):
            # Base case: numeric values
            return np.sum(nested_lists)
        else:  # Recursive case: list of lists
            return sum([recursive_sum([item[i] for item in nested_lists]) 
                       for i in range(len(nested_lists[0]))])
    
    return recursive_sum(lists)


def aggregate_alg(df: pd.DataFrame) -> pd.DataFrame:
    # Columns to group by
    groupby_cols = ['mutation', 'crossover', 'optimize_tardiness', 'blocking']
    
    # Build aggregation dict
    agg_dict = {}
    
    if 'fitnesses' in df.columns:
        agg_dict['fitnesses'] = average_lists
    
    if 'exec_time' in df.columns:
        agg_dict['exec_time'] = 'mean'
    
    if 'makespan' in df.columns:
        agg_dict['makespan'] = 'mean'
    
    if 'tardiness' in df.columns:
        agg_dict['tardiness'] = sum_list_values
    
    # Also average any std dev columns that exist
    std_cols = [col for col in df.columns if col.endswith('_std')]
    for col in std_cols:
        agg_dict[col] = 'mean'
    
    # Group and aggregate
    algorithm_df = df.groupby(groupby_cols, as_index=False).agg(agg_dict)
    
    return algorithm_df