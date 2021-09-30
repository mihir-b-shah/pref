
import sys

import read_reports
import build_venn

if(len(sys.argv) == 1):
  sys.exit('No reports found.')

df = read_reports.build_df(sys.argv[1:])
build_venn.subset_analyze(df)
