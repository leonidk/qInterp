#testing algorithm soudness with 1D example
from pylab import *
length = 11
L1 = False

x = [0]*length + [1]*length
y = [0]*length + [1]*length
e = [0]*len(x)
d1 = [32768 if L1 else 32768]*len(x)
d2 = [32768 if L1 else 32768]*len(x)
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
if L1:
	for i in range(1,len(x)):
		d1[i] = min(d1[i],d1[i-1]+1)
		d2[i] = min(d2[i],d2[i-1]+1)
	for i in reversed(range(0,len(x)-1)):
		d1[i] = min(d1[i],d1[i+1]+1)
		d2[i] = min(d2[i],d2[i+1]+1)
else:
	v = [0.0]*(len(x)+1)
	z = [0.0]*(len(x)+1)
	k = 0
	v[0] = 0
	z[0] = -1000.0
	z[1] = 1000.0
	for q in range(1,len(x)):
		while True:
			dn = (2*q -2*v[k])
			s = ((d1[q] + q*q) - (d1[v[k]] + v[k]*v[k]))/(dn if dn != 0.0 else 1.0)
			if s > z[k]:
				k = k+1
				v[k] = q
				z[k] = s
				z[k+1] = 1000.0
				break
			k = k-1
	k = 0
	for q in range(0,len(x)):
		while z[k+1] < q:
			k = k+1
		d1[q] = (q-v[k])*(q-v[k]) + d1[v[k]]
	v = [0.0]*(len(x)+1)
	z = [0.0]*(len(x)+1)
	k = 0
	v[0] = 0
	z[0] = -1000.0
	z[1] = 1000.0
	for q in range(1,len(x)):
		while True:
			dn = (2*q -2*v[k])
			s = ((d2[q] + q*q) - (d2[v[k]] + v[k]*v[k]))/(dn if dn != 0.0 else 1.0)
			print s,dn,k,z[k]
			if s > z[k]:
				k = k+1
				v[k] = q
				z[k] = s
				z[k+1] = 1000.0
				break
			k = k-1
	k = 0
	for q in range(0,len(x)):
		while z[k+1] < q:
			k = k+1
		d2[q] = (q-v[k])*(q-v[k]) + d2[v[k]]
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
	den = 2*lw-2
	shift = float(d2[i]+d1[i]-2)/(den if den != 0 else 1)

	print i,shift,sw,lw,d1[i],d2[i],float(d2[i]+d1[i]-2),den
	y[i] = x[i] + sgn*(0.5-0.5*shift)

close('all')
plot(x)
plot(y)
show()