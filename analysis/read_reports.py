
import pandas as pd
import numpy as np

def _get_file_path(name):
  return '../reports/' + name + '_report'

def _metric(name, ty):
  return '%s_%s'%(name, ty)

def _prepend(pp, names):
  return list(map(lambda rep : '%s_%s'%(pp, rep), names))

def _build_single_df(name):
    return pd.read_csv(_get_file_path(name), header=0, names=[
      'pc', _metric('acc', name), _metric('acc_wt', name), _metric('miss', name), _metric('cov_wt', name)
    ])

def _consolidate(header, rep_names, func, df):
  type_names = _prepend(header, rep_names)
  df[header] = func(df[type_names], axis=1)
  df.drop(type_names, axis=1, inplace=True)

def build_df(rep_names):
  # join on PCs
  iter_df = _build_single_df(rep_names[0])
  for i in range(1, len(rep_names)):
    iter_df = iter_df.merge(_build_single_df(rep_names[i]), on='pc')

  # consolidate weights
  _consolidate('acc_wt', rep_names, pd.DataFrame.mean, iter_df)
  _consolidate('cov_wt', rep_names, pd.DataFrame.mean, iter_df)

  nn_reps = rep_names.copy()
  nn_reps.remove('no')

  miss_names = _prepend('miss', nn_reps)
  cov_names = _prepend('cov', nn_reps)

  # build coverage
  for miss_name, cov_name in zip(miss_names, cov_names):
    iter_df[cov_name] = iter_df[miss_name]/iter_df['miss_no']
    iter_df[cov_name] = 1-iter_df[cov_name]
  
  # drop miss names
  iter_df.drop(miss_names, axis=1, inplace=True)
  iter_df.drop(list(iter_df.filter(regex=r'.*_no')), axis=1, inplace=True)

  iter_df = iter_df[~iter_df.isin([np.nan, np.inf, -np.inf, 0]).any(1)]
  iter_df[cov_names] = iter_df[cov_names].clip(0,1)

  acc_df = iter_df.filter(regex='(acc.*)|(pc)').rename(columns=lambda x: x.replace('acc_',''))
  cov_df = iter_df.filter(regex='(cov.*)|(pc)').rename(columns=lambda x: x.replace('cov_',''))
  
  return acc_df, cov_df

def _print_full(df):
  with pd.option_context('display.max_rows', None, 'display.max_columns', None):
    print(df)
