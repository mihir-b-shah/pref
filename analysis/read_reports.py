
import pandas as pd

def _get_file_path(name):
  return '../reports/' + name + '_report'

def _metric(name, ty):
  return '%s_%s'%(name, ty)

def _prepend(pp, names):
  return list(map(lambda rep : '%s_%s'%(pp, rep), names))

def _build_single_df(name):
    return pd.read_csv(_get_file_path(name), header=0, names=[
      'pc', _metric('acc', name), _metric('wt_acc', name), _metric('miss', name), _metric('wt_cov', name)
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
  _consolidate('wt_acc', rep_names, pd.DataFrame.mean, iter_df)
  _consolidate('wt_cov', rep_names, pd.DataFrame.mean, iter_df)

  return iter_df

df = build_df(['no', 'bo', 'sisb', 'sms', 'ip_stride'])
print('no: ' + str(sum(df['miss_no'])))
print('bo: ' + str(sum(df['miss_bo'])))
print('sms: ' + str(sum(df['miss_sms'])))
print('ip_stride: ' + str(sum(df['miss_ip_stride'])))
print('sisb: ' + str(sum(df['miss_sisb'])))
print(df)
