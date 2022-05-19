[toc]

------

## 17.2 InnoDB 的 Buffer Pool

### 17.2.1 啥是 Buffer Pool

一片连续的内存

大小在启动选项配置

```
[server]
innodb_buffer_pool_size = xxxxx(字节)
```

最小值为 5M

### 17.2.2 Buffer Pool 的内部组成

内部也被划分成页，大小 16KB

缓冲页

每个缓冲页都有一些控制信息：表空间编号、页号、缓冲页地址、链表节点信息

把每个页对应的控制信息占用的-块内存称为一个控制块.

控制块与缓冲页是一一对应的，都存放在 Buffer Pool

![image-20220519163025160](/Users/daydaylw3/Pictures/typora/image-20220519163025160.png)

> debug模式下，每个控制块大小大约为缓冲页的5%
>
> 缓冲页不在 innodb_buffer_pool_size 申请的范围内

### 17.2.3 free 链表的管理

把所有空闲的缓冲页对应的控制块作为一个节点放到一-个链表中，这个链表也可以称为 free 链表

为了管理好这个free 链衰，我们特意为这个链表定义了一个基节点，里面包含链表的头节点地址、尾节点地址，以及当前链表中节点的数量等信息

加载一个页到 Buffer Pool，就从 free 取一个空闲页，填上控制块信息，就把控制块从 free 中移除，表示该缓冲页已经被使用了

### 17.2.4 缓冲页的哈希处理

用表空间号+页号 作为key，缓冲页控制块的地址作为 value 创建哈希表。访问某个页，从哈希表中查看是否有对应的缓冲页；没有就从 free 链表中选一个空闲的

### 17.2.5 flush 链表的管理

脏页（dirty page）：修改过的缓冲页数据（和磁盘上的不一样了）

不立即刷盘

如何识别脏页？一个存储脏页的链表：flush 链表

![image-20220519164355674](/Users/daydaylw3/Pictures/typora/image-20220519164355674.png)

> flush 链表的节点和 free 链表的节点是互斥的

### 17.2.6 LRU 链接的管理

缓冲页 free 用完了要移除旧的，移哪些？

#### 2. 简单的 LRU 链表

Least Recently Used

维护一个链表，越接近表头就常用，表末端就少

#### 3. 划分区域的 LRU 链表

简单的 LRU 链表的问题

+ 预读
  + 线性预读：innodb_read_ahead_threshold，顺序访问某个区的页面超过该值，就异步读取下个区中全部的页面
  + 随机预读：某个区 13 个连续的页面都被加载到了 Buffer Pool，异步读取本区所有其他页面。innodb_random_read_ahead
+ 全表扫描时，会读取很多页，会严重影响其他查询对 Buffer Pool 的使用

为了提高 Buffer Pool 的命中率，按比例划分

+ 热数据，young 区域
+ 冷数据，old 区域

![image-20220519171355614](/Users/daydaylw3/Pictures/typora/image-20220519171355614.png)

比例确定：innodb_old_blocks_pct（全局变量）

可以在启动时设定

+ 预读优化：初次加载放到old头部，不影响young
+ 全表扫描优化：记录 old 区域缓冲页第一次访问的时间，后续再访问时比较间隔，innodb_old_blocks_time，大于该值 > 加入到 young 头部，小于就留在 old（全表扫描过程中，多次访问同一个页面的多条记录的时间不会超过 1s）

#### 4. 更进一步优化 LRU 链表

解决频繁移动 young 缓冲页的开销，只有被访问的缓冲页位于 young 区域 1/4 的后面，才会被移动到 LRU 链表头部

> 优化 LRU 的目的是为了高效提高 Buffer Pool 的命中率

> flush 中的节点肯定也是 LRU 链表中的节点

### 17.2.7 其他的一些链表

unzip LRU

zip clean

zip free

### 17.2.8 刷新脏页到磁盘

后台有专门的线程负责每隔一段时间就把脏页刷新到磁盘

+ 从 LRU 链表的冷数据中刷新一部分页面到磁盘

  innodb_lru_scan_depth：扫描尾部链表的数量以刷盘。BUF_FLUSH_LRU 的刷盘方式

+ 从 flush 链表中刷新一部分页面到磁盘

  定时从 flush 链表中刷一部分，刷新速率取决于系统是否繁忙。BUF_FLUSH_LIST 的刷盘方式

+ BUF_FLUSH_SINGLE_PAGE，Buffer Pool 中没有可用的缓冲页了，去 LRU 链表尾部找有无可以释放的，没有可以释放的，就把一些尾部的脏页刷盘

+ 系统繁忙的时候也可能会出现同步刷盘，但是这是不得已的，因为太慢了

### 17.2.9 多个 Buffer Pool 实例

多线程，锁，并发量高

将一个大的 Buffer Pool 拆分成若干个小的

innodb_buffer_pool_instances 修改实例个数（启动参数）

当 innodb_buffer_pool_size 的大小超过 1GB，才有效，否则固定为 1

### 17.2.10 innodb_buffer_pool_chunk_size

以一个 chunk 为单位申请 buffer pool 的空间

chunk 大小，innodb_buffer_pool_chunk_size，默认值 128MB，134,217,728字节

>  这个值也不包含对应控制块的内存空间

### 17.2.11 配置 Buffer Pool 时的注意事项

+ innodb_buffer_pool_size 必须是 innodb_buffer_pool_chunk_size × innodb_buffer_pool_instances 的倍数

  （为了保障每个 Buffer Pool 实例中 chunk 数量相同）

+ 如果 innodb_buffer_pool_chunk_size × innodb_buffer_pool_instances 的值 > innodb_buffer_pool_size，会被服务器设置为 innodb_buffer_pool_size ÷ innodb_buffer_pool_instances

### 17.2.12 查看 Buffer Pool 的状态信息

```
SHOW ENGINE INNODB STATUS;
```

|                              |                                                              |
| ---------------------------- | ------------------------------------------------------------ |
| Buffer pool size             | 可以容纳多少缓冲页                                           |
| Free buffers                 | free 链表节点数                                              |
| Database pages               | LRU 链表节点数，包含 young old                               |
| Old database pages           | LRU old 节点数                                               |
| Modified db pages            | 脏页数，flush 节点数                                         |
| Pending reads                | 准备从磁盘加载到 Buffer Pool 的页面数（已经分配了缓冲页和控制块，但是未真正读取） |
| Pending writes LRU           | 将从 LRU -> 磁盘的页面数量                                   |
| Pending writes flush list    | 将从 flush 链表 -> 磁盘的页面数量                            |
| Pending writes single page   | 将以单个页面的形式 -> 磁盘的页面数量                         |
| Pages made young             | LRU 中曾经从 old -> young 的数量                             |
| Pages made not young         | 首次访问后者后续访问 old 节点，间隔未超过 innodb_old_blocks_time 而未能移动到 young 的数量 |
| youngs/s                     | 每秒从 old -> young 的量                                     |
| non-youngs/s                 | 每秒不满足时间限制而未能移动到 young                         |
| Pages read、created、written | 读取、创建、写入了多少页，以及相应的速率                     |
| Buffer pool hit rate         | 平均访问 1000 次 Buffer Pool，命中率                         |
| young-making rate            | 平均 1000 次，多少次访问使页面移动到 young 头部（包含 从 young 移动到 young 头） |
| not (yount-making rate)      | 多少次访问没有使页面移动到 young 头部                        |
| LRU len                      | LRU 节点数量                                                 |
| unzip_LRU                    |                                                              |
| I/O sum                      | 最近 50s 读取磁盘页总数                                      |
| I/O cur                      | 现在正在读取磁盘页数量                                       |
| I/O unzip sum                | 最近 50s 解压的页面数量                                      |
| I/O unzip cur                | 正在...                                                      |

## 17.3 总结



------

[toc]