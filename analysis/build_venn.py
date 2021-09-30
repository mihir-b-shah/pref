
import pandas as pd

# really bad
def _int_to_bool_arr(v,l):
  ret = []
  while(v > 0):
    ret.append(True if v&1 == 1 else False)
    v >>= 1
  while(len(ret) < l):
    ret.append(False)
  return ret

def subset_analyze(df):
  for mask in range(1,16):
    cols = df.columns[_int_to_bool_arr(mask << 1, len(df.columns))]
    best = df[cols].idxmax(axis=1)
    best_with_weights = pd.concat([best, df['wt']], axis=1)
    
    print(list(cols))
    print(best_with_weights.groupby(0).sum())
    print('\n')
