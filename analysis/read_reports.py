
import pandas as pd

def _get_file_path(name):
  return 'reports/' + name + '_report'

def _build_single_df(name):
    return pd.read_csv(_get_file_path(name), header=0, names=['pc', name, 'wt_%s'%(name)])

def build_df(rep_names):
  iter_df = _build_single_df(rep_names[0])
  for i in range(1, len(rep_names)):
    iter_df = iter_df.merge(_build_single_df(rep_names[i]), on='pc')
  wt_names = list(map(lambda rep : 'wt_' + rep, rep_names))
  iter_df['wt'] = iter_df[wt_names].mean(axis=1)
  iter_df.drop(wt_names, axis=1, inplace=True)

  return iter_df
