#testing algorithm soudness with 1D example
from pylab import *
length = 11
x = [0]*length + [1]*length
y = [0]*length + [1]*length
e = [0]*len(x)
d1 = [32768]*len(x)
d2 = [32768]*len(x)
mD1 = [0]*len(x)
mD2 = [0]*len(x)

e[0] = 1
e[-1] = 1
d1[0] = 1
d1[-1] = 1
d2[0] = 1
d2[-1] = 1
mD1[0] = 1
mD1[-1] = 1
mD2[0] = 1
mD2[-1] = 1

for i in range(1,len(x)-1):
	if x[i-1]-x[i] >= 1 or x[i+1]-x[i] >= 1:
		e[i] |= 1
		d1[i] = 1
		mD1[i] = 1
	if x[i-1]-x[i] <= -1 or x[i+1]-x[i] <= -1:
		e[i] |= 2
		d2[i] = 1
		mD2[i] = 1

for i in range(1,len(x)):
	d1[i] = min(d1[i],d1[i-1]+1)
	d2[i] = min(d2[i],d2[i-1]+1)
for i in reversed(range(0,len(x)-1)):
	d1[i] = min(d1[i],d1[i+1]+1)
	d2[i] = min(d2[i],d2[i+1]+1)

#needs iterations to deal with non-convex regions
for iters in range(5):
	for i in range(1,len(x)):
		mD1[i] = 1 if d1[i] == 1 else max(d1[i],max(mD1[i],mD1[i-1]))
		mD2[i] = 1 if d2[i] == 1 else max(d2[i],max(mD2[i],mD2[i-1]))
	for i in reversed(range(0,len(x)-1)):
		mD1[i] = 1 if d1[i] == 1 else max(d1[i],max(mD1[i],mD1[i+1]))
		mD2[i] = 1 if d2[i] == 1 else max(d2[i],max(mD2[i],mD2[i+1]))

for i in range(len(x)):
	sw = min(d1[i],d2[i])
	lw = max(mD1[i],mD2[i])
	sgn = 1 if sw == d1[i] else -1
	sgn2 = '+' if sgn == 1 else '-'
	#print d1[i],d2[i],sgn2,lw,mD1[i],mD2[i]
	shift = float(d2[i]+d1[i]-2)/float(2*lw)
	y[i] = x[i] + sgn*(0.5-0.5*shift)

close('all')
plot(x)
plot(y)
show()