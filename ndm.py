import numpy as np
import matplotlib.pyplot as plt

def apply(a,b,X,diff) : return a*X+b+diff
def NormalDataManufacture(a,b,min_x,max_x,sigma,n,group):
    X = np.random.randint(min_x,max_x,n)
    diff = np.random.normal(0,sigma,n)
    #print(min_x,max_x)
    Y = apply(a,b,X,diff)
    Y = (Y+0.5).astype('int')
    for i in range(len(Y)):
        if Y[i] < 0 : Y[i] = 0
        print(str(group)+' '+str(X[i])+' '+str(Y[i])+' ')
     
    return X,Y
    

np.random.seed(100) #シード設定

_plot = False

point=5
g_column=2
g_row=3
itr = g_column*g_row
#作成される点数は，point*g_column*g_row itrはグラフの個数

group = 100
ItemNum,GroupNum,MaxWeight = itr*point*group,group,5000
print(str(ItemNum)+' '+str(GroupNum)+' '+str(MaxWeight))

sep_base=10 #区間幅の平均
sep_sigma=3 #区間幅のばらつき

for t in range(group):
    #視覚に優しく
    if _plot :
        fig = plt.figure(figsize=(12,7))
        t_fig = plt.figure()
        
        
        t_ax = t_fig.add_subplot(111)
        t_ax.scatter(0,0,color='w',edgecolor='w')
    
    foundation=5 #min_xに相当，
    slopes = np.sort(np.abs(np.random.normal(1,2,itr))) #正の傾きを生成 ソート
    for i in range(itr) :
        if slopes[i] < 0.3 : slopes[i] = 0.3
    sections = np.sort(np.abs(np.random.normal(10,3,itr))) #正の切片を生成 ソート
    sigmas = np.array([max(1/slopes[k] , sections[k]/2)  for k in range(itr)]) #標準偏差の生成
    colors = ['red','blue','green','yellow','cyan','orange']


    for i in range(itr) :
        
        separation = np.random.normal(sep_base,sep_sigma,1).astype('int')
        sepa_array = np.array( [foundation , foundation + separation] )
        foundation += separation
        #print(slopes[i],sections[i],sigmas[i])
        X,Y = NormalDataManufacture(slopes[i],sections[i],\
                                    sepa_array[0],sepa_array[1],sigmas[i],point,t)
        if _plot :
            ax = fig.add_subplot(g_row,g_column,i+1)
            ax.scatter(X,Y,color=colors[i%6])
            
            ax.plot(sepa_array,apply(slopes[i],sections[i],sepa_array,\
                        np.array([0,0]) ),color=colors[i%6],linestyle='--')
            t_ax.scatter(X,Y,color=colors[i%6])
        
    if _plot :
        fig.tight_layout()
        t_fig.tight_layout()
        plt.show()
        _plot=False
