# LDPCdecode
基本上就是学长的代码，自己改点东西

目前使用了打孔算法，3.7db误帧率达到7.7276e-04，和老师的要求基本上接近

# 分层算法

老师说我们的矩阵适合的是垂直分层算法

![image](https://user-images.githubusercontent.com/69625023/140531619-c8730ca2-2c7d-48b7-84d2-54ca96afff4a.png)

这个是垂直分层，可以看出和普通的MS算法相比，每一层是直接计算，不需要等某一个节点全部更新完毕,并且这个分层是按列分层

不过我不太理解的是，根据下图

![image](https://user-images.githubusercontent.com/69625023/140614468-e02d4710-ec48-4858-948d-bfde81720476.png)

1、Horizontal step和Vertical step应该同时处理，那仿真怎么体现同时处理？

     问题解决，仿真就不要同时处理了，一个先一个后，谁先谁后都可以。

2、列分层，每一层的校验节点对应的变量节点也就1到2个，这样找最小值岂不是大部分层都无法找到？

     理解上的错误，CTV需要算所有的变量节点
     
同时有个要注意的地方，第一次迭代不要用分层
     
![image](https://user-images.githubusercontent.com/69625023/140442698-309cedd4-0353-4e24-af18-b6ac67a51e2f.png)

![image](https://user-images.githubusercontent.com/69625023/140650615-0af6582d-a775-4684-817f-4d15b2ad8a3f.png)

![image](https://user-images.githubusercontent.com/69625023/140650619-1b841760-a6d1-4597-af2d-db86a60ec4f3.png)

上面的算法是水平分层，是一种按行分层的算法，当然这个图用的是bp，我们用最小和，本次也编写了这个算法

这两种算法本质相同，比如在算CTV时，里面的变量节点在一直更新，这样收敛速度会加快，但是水平分层好实现一些
