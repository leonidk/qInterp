import numpy as np
import matplotlib.pyplot as plt

length = 10
#x = [0]*length + [1]*length
x = [0]*length + [1]*length + [2]*length + [1]*length + [0]*length
dd = [0]*len(x)
dp = [0]*len(x)
dn = [0]*len(x)

en = [0]*len(x)
ed = [0]*len(x)
ep = [0]*len(x)



for i in range(1,len(x)):
	if x[i-1] < x[i]:
		en[i] = 1
		#en[i-1] = 1
	if x[i-1] > x[i]:
		ep[i] = 1
		#ep[i-1] = 1
for i in range(len(x)-2,-1,-1):
	if x[i+1] < x[i]:
		en[i] = 1
		#en[i+1] = 1
	if x[i+1] > x[i]:
		ep[i] = 1
		#ep[i+1] = 1
def dt(e,e2,e3):
	d = [1000]*len(e)
	for i in range(len(e)):
		if e[i] > 0:
			d[i] = 1
	for i in range(1,len(e)):
		if e[i] == 0:
			d[i] = min(d[i],d[i-1]+2)
	for i in range(len(e)-2,-1,-1):
		if  e[i] == 0:
			d[i] = min(d[i],d[i+1]+2)
	return d
def dt(e,e2,e3,check=True):
	d = [2000]*len(e)
	for i in range(len(e)):
		if e[i] > 0:
			d[i] = 1
	i = 0
	while i < len(e):
		if (not check) or e2[i] ==0 and e3[i]==0:
			d[i] = min(d[i],d[i-1]+2)
		i+= 1
	i = len(e)-2
	while i >= 0:
		if (not check) or e2[i] ==0 and e3[i]==0:
			d[i] = min(d[i],d[i+1]+2)
		i -= 1
	return d
dp = dt(ep,ed,en)
dn = dt(en,ed,ep)
ed[0] = 1
ed[-1] = 1
dd = dt(ed,ep,en,False)

y = [-1]*len(x)
for i in range(len(x)):
	m = -123123
	if dn[i] < dp[i] and dd[i] < dp[i]:
		m = 1
		y[i] = x[i] - 0.5*(dd[i])/float(dd[i]+dn[i])
	elif dp[i] < dn[i] and dd[i] < dn[i]:
		m = 2
		y[i] = x[i] + 0.5*(dd[i])/float(dd[i]+dp[i])
	else:
		m = 3
		y[i] = x[i] + 0.5*(dn[i]-dp[i])/float(dn[i]+dp[i])
		#y[i] = x[i] + 0.5*((dn[i]-dp[i])/float(dn[i]+dp[i])) 
	print(i,en[i],ep[i],ed[i],dn[i],dp[i],dd[i],x[i],y[i],m)

plt.subplot(3,1,1)
plt.plot(x)
plt.plot(y)
plt.subplot(3,1,2)
plt.plot(en,ls='--')
plt.plot(ep,ls='-.')
plt.plot(ed,ls=':')
plt.subplot(3,1,3)
plt.plot(dn,ls='--')
plt.plot(dp,ls='-.')
plt.plot(dd,ls=':')
plt.show()