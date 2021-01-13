import numpy as np
import matplotlib.pyplot as plt

def apply(a,b,X,diff) : return a*X+b+diff

def NormalDataManufacture(a,b,min_x,max_x,sigma,n) :
    X = np.random.randint(min_x,max_x,n)
    diff = np.random.normal(0,sigma,n)
    Y = apply(a,b,X,diff)
    Y = (Y+0.5).astype('int')
    for i in range(len(Y)) :
        if Y[i] < 0 : Y[i] = abs(Y[i])
    return X,Y
    
np.random.seed(6)
_plot = False
count = 0
while True:
    params = input()#'input: \'int point\',\'int min_x\',\'int max_x\',\'float slope\',\'int section\',\'float sigma\'\n')
    params = params.split(',')
    point = int(params[0])
    if point == 0 : break
    min_x = int(params[1])
    max_x = int(params[2])
    slope = float(params[3])
    section = int(params[4])
    sigma = float(params[5])
    
    X,Y = NormalDataManufacture(slope,section,min_x,max_x,sigma,point)
    
    print(point)
    for i in range(point) : print(str(Y[i])+','+str(X[i]))
    
    if _plot:
        fig = plt.figure(figsize=(12,7))
        ax = fig.add_subplot(111)
        
        px = np.array([min(X),max(X)])
        
        ax.scatter(X,Y,color='black',marker='o')
        ax.plot(px,slope*px+section,color='red',linestyle='--',linewidth=0.5,label='y='+str(slope)+'x') #+'+str(section))
        
        ax.set_title('number of item = '+str(point)+" $\sigma$ = "+str(sigma))
        ax.set_xlabel('x',fontsize=14)
        ax.set_ylabel('y',rotation=0,fontsize=14)
        ax.spines['right'].set_visible(False)
        ax.spines['top'].set_visible(False)
        
        #fig.text(0.4,0,'Fig1. make items',fontsize=18)
        plt.legend()
        plt.savefig('item_plot.png',bbox_inches='tight',pad_inches=1.0)
        plt.show()
    
    count+=1
