Redis

+ 单线程

快

+ C 语言编写
+ 单线程，少了上下文切换
+ IO 多路复用
+ 基于内存

+ 支持数据持久化
+ 支持主从集群（安全策略、读写分离），分片集群（水平扩展）



```
setnx
set key value nx


```

![image-20220531211402570](/Users/daydaylw3/Pictures/typora/image-20220531211402570.png)

Redis key 的层级结构

Proj:Type:name

Hash

`hset key field value`

