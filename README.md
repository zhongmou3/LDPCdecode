# LDPCdecode
基本上就是学长的代码，自己改点东西

目前使用了打孔算法，3.7db误帧率达到7.7276e-04，和老师的要求基本上接近

# 分层算法

![image](https://user-images.githubusercontent.com/69625023/140442698-309cedd4-0353-4e24-af18-b6ac67a51e2f.png)

上面的算法是水平分层，是一种按行分层的算法，但老师说我们的矩阵适合的是垂直分层算法

![image](https://user-images.githubusercontent.com/69625023/140531619-c8730ca2-2c7d-48b7-84d2-54ca96afff4a.png)

然后这个是垂直分层，可以看出和普通的MS算法相比，每一层是直接计算，不需要等某一个节点全部更新完毕,并且这个分层是按列分层
