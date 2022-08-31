import pandas as pd
from sklearn import cluster
from sklearn.manifold import TSNE
from sklearn.decomposition import PCA
from sklearn.preprocessing import MinMaxScaler
import numpy as np
import matplotlib.pyplot as plt
import sys
import warnings

NCLASS = 3
NDIM = 2
category_colors = plt.get_cmap('tab10')(np.linspace(0., 1., NCLASS))

metrics = [
    'Max.Length', 'Max.Width', 'Max.Height', 'Act.Length.repr', 'Act.Width', 
    'Act.Height', 'Avg.Refl', 'Min.Refl', 'Max.Refl', 'Std.Dev.Refl'
]

def dimreduction(X, method, n_components):
    meth = method(n_components = n_components)
    X = meth.fit_transform(X)
    if n_components > 2:
        X = X[:,0:2]
    X = MinMaxScaler().fit_transform(X)
    return X

def clustering(X, n_clusters):
    kmeans = cluster.KMeans(n_clusters=n_clusters, random_state=0).fit(X)
    return kmeans.labels_.astype(int)

def plotClusters(X, predicted, title, save):
    plt.figure(figsize=(6, 6), dpi=320)
    for (point, pred) in zip(X, predicted):
        plt.scatter(*point, color=category_colors[pred])
    plt.xticks([])
    plt.yticks([])
    plt.title(title)
    plt.tight_layout()
    plt.savefig(save)

if __name__ == '__main__':
    warnings.filterwarnings('ignore')
    if len(sys.argv) <= 1:
        print('Error: Wrong argument format!')
        print('Usage: python3 cluster.py <filenm>')
        exit()
    filenm = sys.argv[1]
    df = pd.read_csv(filenm)
    df = df[metrics]
    arr = df.to_numpy()
    arr_pca2 = dimreduction(arr, PCA, NDIM)
    predicted_pca2 = clustering(arr_pca2, NCLASS)
    plotClusters(arr_pca2, predicted_pca2, 'Cluster Result', 'PCA_2.pdf')
    arr_tsne2 = dimreduction(arr, TSNE, NDIM)
    predicted_tsne2 = clustering(arr_tsne2, NCLASS)
    plotClusters(arr_tsne2, predicted_tsne2, 'Cluster Result', 'TSNE_2.pdf')