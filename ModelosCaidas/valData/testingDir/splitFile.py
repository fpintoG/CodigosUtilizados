from itertools import zip_longest
import glob

def grouper(n, iterable, fillvalue=None):
    "Collect data into fixed-length chunks or blocks"
    # grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx
    args = [iter(iterable)] * n
    return zip_longest(fillvalue=fillvalue, *args)

n = 61

read_files = glob.glob("*.txt")
for r in read_files:
	with open(r) as f:
		for i, g in enumerate(grouper(n, f, fillvalue=''), 1):
			with open(r.format(i * n) + '.txt', 'w') as fout:
				fout.writelines(g)