import numpy as np
from matplotlib import pyplot as plt
from matplotlib import cm

def _poles(n):
  poly = np.zeros(1+n)
  poly[0] = 1
  poly[n] = -1
  roots = np.roots(poly)
  return np.column_stack((roots.real, roots.imag))

def _conv_color(sums):
  return sums

def _norm_sizes(sizes):
  return np.interp(sizes,[min(sizes),max(sizes)],[20,2000])

# returns x,y arrays, color array, size array
def _build_px(df):
  df = df.drop('pc', axis=1)
  wts = df['wt']
  pf_df = df.drop('wt', axis=1)
  sums = pf_df.sum(axis=1)

  pf_df = pf_df.divide(sums, axis=0)

  remove_rows = pf_df.isin([np.nan]).any(1)

  wts = wts[~remove_rows]
  sums = sums[~remove_rows]
  pf_df = pf_df[~remove_rows]

  cols = list(pf_df.columns)
  poles = _poles(len(pf_df.columns))
  pf_df = pf_df.dot(_poles(len(pf_df.columns)))

  return pf_df[0], pf_df[1], _conv_color(sums), _norm_sizes(wts), cols, poles

def graph(df):
  _build_px(df)
  xs, ys, colors, sizes, names, poles = _build_px(df)
  plt.scatter(xs, ys, c=colors, s=sizes, alpha=0.5, cmap='plasma')
  plt.colorbar()
  plt.xlim(-1.5,1.5)
  plt.ylim(-1.5,1.5)

  for name, pole in zip(names, poles):
    plt.annotate(name, pole*1.25)
    
  plt.savefig('meme.jpg')
