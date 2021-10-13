
import sys

import read_reports
import subsets
import pole_plot

if(len(sys.argv) == 1):
  sys.exit('No reports found.')

acc_df, cov_df = read_reports.build_df(sys.argv[1:])
#subsets.subset_analyze(cov_df)
pole_plot.graph(cov_df)
