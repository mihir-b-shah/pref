
import sys

import read_reports
import build_venn

if(len(sys.argv) == 1):
  sys.exit('No reports found.')

acc_df, cov_df = read_reports.build_df(sys.argv[1:])
print(acc_df)
#build_venn.subset_analyze(acc_df)
