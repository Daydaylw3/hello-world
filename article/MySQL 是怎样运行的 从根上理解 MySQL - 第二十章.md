[toc]

------

## 20.1 事务回滚的需求

回滚，保证事务的原子性

撤销日志（undo log）：为了回滚而记录的东西

## 20.2 事务 id

### 20.2.1 分配事务 id 的时机

某个事务在执行过程中树某个表执行了增删改操作，InnoDB 分配唯一的 id

+ 只读事务，对某个用户创建的临时表执行增删改操作（与 select 用到的内部临时表不同
+ 读写事务，第一次对某个表进行增删改操作

### 20.2.2 事务 id 生成

和 row_id 相似

全局变量，256倍数时刷新到 表空间页号为5的页面的 Max Trx ID 属性，系统重启时，加载 Max Trx ID 值，并加上 256 

### 20.2.3 trx_id 隐藏列

InnoDB 行格式的 trx_id 隐藏列，记录对该记录进行改动的语句所在事务的事务 ID

## 20.3 undo 日志的格式

为了实现事务的原子性， InnoDB 存储引擎在实际进行记录的增删改操作时，都需要先把对应的undo 日志记下来。一般每对一条记录进行一次改动，就对应着一条undo 日志。一个事务在执行过程中需要记录很多条对应的undo 日志

undo no，按顺序 从 0 ~ n

undo 日志被记录到类型为 FIL_PAGE_UNDO_LOG 的页面中

```mysql
mysql> create table undo_demo (
    -> id int not null,
    -> key1 varchar(100),
    -> col varchar(100),
    -> primary key (id),
    -> key idx_key1 (key1)
    -> )engine=InnoDB charset=utf8;
Query OK, 0 rows affected, 1 warning (0.29 sec)

mysql> select * from information_schema.innodb_tables where name = 'dayday/undo_demo';
+----------+------------------+------+--------+-------+------------+---------------+------------+--------------+--------------------+
| TABLE_ID | NAME             | FLAG | N_COLS | SPACE | ROW_FORMAT | ZIP_PAGE_SIZE | SPACE_TYPE | INSTANT_COLS | TOTAL_ROW_VERSIONS |
+----------+------------------+------+--------+-------+------------+---------------+------------+--------------+--------------------+
|     1073 | dayday/undo_demo |   33 |      6 |    12 | Dynamic    |             0 | Single     |            0 |                  0 |
+----------+------------------+------+--------+-------+------------+---------------+------------+--------------+--------------------+
1 row in set (0.17 sec)
```

table id = 1073

### 20.3.1 INSERT 操作对应的 undo 日志

TRX_UNDO_INSERT_REC 类型的 undo 日志

![image-20220524003732157](/Users/daydaylw3/Pictures/typora/image-20220524003732157.png)

undo no 从 0开始递增

> 当我们向某个表中插入一条记录时，实际上需要向聚玉皇索引和所有二级索引都插入一条记录.不过在记录undo 日志时，我们只需要针对聚簇索引记录来$己录一条undo 日志就好了.聚簇索引记录和二级索引记录是一一对应的，我们在回滚INSERT 操作时，只需要知道这条记录的主键信息， 然后根据主键信息进行对应的删除操作。在执行删除操作时，就会把聚簇索引和所有二级索引中相应的记录都删掉

行列式 roll_pointer 是一个指向记录对应的 undo 日志的指针

### 20.3.2 DELETE 操作对应的 undo 日志

删除的过程经历两个阶段

+ 记录的 deleted_flag 标志位 1（delete mask 阶段）

  并**没有**将 记录添加到 垃圾链表中，直到事务提交

+ 删除 语句的事务提交后，专门的线程把记录从正常记录链表中移除，加入到垃圾链表中，调整一些其他信息（purge 阶段）

  删除的记录加入到垃圾链表中是加入到头结点，还要跟着修改 PAGE_FREE 的值

TRX_UNDO_DEL_MARK_REC 类型的 undo 日志

![image-20220524005733611](/Users/daydaylw3/Pictures/typora/image-20220524005733611.png)

在对一条记录进行 delete mark 操作前，需要把该记录的 trx_id 和roll_pointer 隐藏列的旧值（该记录上一次更新的 undo 日志）都记到对应的 undo 日志中的 trx id 和roll pointer 属性中

版本链

![image-20220524010027305](/Users/daydaylw3/Pictures/typora/image-20220524010027305.png)

### 20.3.3 UPDATE 操作对应的 undo 日志

#### 1. 不更新主键

+ 就地更新（in-place update

更新后的列与更新前的每个列占用的存储空间一样大

+ 先删除旧记录，再插入新纪录

有任何一个被更新的列在更新前和更新后占用的存储空间大小不一致

先把这条旧记录从聚簇索引页面中删除，再根据更新后列的值创建一条新的记录并插入到页面中；这里的删除是真正的删除（将记录从正常链表中移到垃圾链表中

TRX_UNDO_UPD_EXIST_REC 类型的 undo 日志

![image-20220524011338752](/Users/daydaylw3/Pictures/typora/image-20220524011338752.png)

#### 2. 更新主键

步骤1. 将旧记录进行 delete mask 操作

步骤2. 根据更新后各列的值创建一条新记录，并将其插入到聚簇索引中

步骤1 记录 TRX_UNDO_DEL_MARK_REC 日志，步骤2 记录 TRX_UNDO_INSERT_REC 日志

### 20.3.4 增删改操作对二级索引的影响

UPDATE 操作有点和聚簇索引的操作不同

update 语句涉及二级索引的列更新

+ 旧的二级索引记录执行 delete mask 操作
+ 创建一条新的二级索引记录，加入到 B+ 树中

> 影响 二级索引记录所在页面的 Page Header 部分的 PAGE_MAX_TRX_ID 的属性（当前页的最大事务 id）

## 20.4 通用链表结构

在写入undo 日志的过程中， 会用到多个链表。很多链表都有同样的节点结构

![image-20220524012235641](/Users/daydaylw3/Pictures/typora/image-20220524012235641.png)

   基节点，存储链表头结点，尾结点，长度信息

![image-20220524012418605](/Users/daydaylw3/Pictures/typora/image-20220524012418605.png)

## 20.5 FIL_PAGE_UNDO_LOG 页面

该页面类型专门存储 undo 日志

![image-20220524012608932](/Users/daydaylw3/Pictures/typora/image-20220524012608932.png)

Undo Page Header

![image-20220524012847236](/Users/daydaylw3/Pictures/typora/image-20220524012847236.png)

好几种类型的 undo 日志，被分为两个大类：TRX_UNDO_INSERT（1）、TRX_UNDO_UPDATE（2）

不同大类的undo 日志不能混着存储

> 之所以把皿do 日志分成2 个大类，是因为 TRX_UNDO_INSERT undo 日志在事务提交后可以直接删除掉，而其他类型的 undo 日志不能直接删除掉，因此对它们的处理需要区别对待

TRX_UNDO_PAGE_START、TRX_UNDO_PAGE_FREE：当前页面开始存储 undo 日志 和最后一条 undo 日志结束的偏移量

TRX_UNDO_PAGE_NODE：链表节点

## 20.6 Undo 页面链表

### 20.6.1 单个事务中的 Undo 页面链表

因为一个事务可能包含多个捂句， 而且一个语句可能会对若干条记录进行改动，而对每条记录进行改动前（再强调一下， 这里指的是聚簇索引记录）都需要记录 1 条或 2 条 undo 日志。所以在一个事务执行过程中可能产生很多 undo 日志。

这些日志可能在一个页面中放不下， 需要放到多个页面中。

TRX_UNDO_PAGE_NODE 属性连成链表

链表中的第一个Undo 页面 first undo page，包含其他的一些管理信息；其余的 normal undo page。

一个事务可能需要 2 个 undo 页面链表：insert undo，update undo

>  对普通表和临时表的记录改动时所产生的 undo 日志要分别记录

在一个事务中最多有4 个 undo 链表

**具体分配策略**：按需分配， 啥时候需要啥时候分配， 不需要就不分配。

### 20.6.2 多个事务中的 Undo 页面链表

不同事务执行过程中产生的 undo 日志需要写入不同的 Undo 页面链表中

### 20.7 undo 日志具体写入过程

### 20.7.1 段的概念

第九章

### 20.7.2 Undo Log Segment Header

每个 undo 页面链表都对应一个段，Undo Log Segment

first undo page -> Undo Log Segment Header 部分

![image-20220524110158754](/Users/daydaylw3/Pictures/typora/image-20220524110158754.png)

Undo Log Segment Header

![image-20220524110303306](/Users/daydaylw3/Pictures/typora/image-20220524110303306.png)

TRX_UNDO_STATE：链表处于的状态：

TRX_UNDO_ACTIVE：活跃，一个活跃的事务在写入

TRX_UNDO_CACHED：被缓存，等待之后被其他事务重用

TRX_UNDO_TO_FREE：在他对应的事务被提交后，该链表不能被重用处于的状态，等待被释放

TRX_UNDO_TO_PURGE：对于 update undo 链表来说，在他对应的事务被提交之后，该链表不能被重用处于的状态

TRX_UNDO_PREPARED：用户存储处于 PREPARE 阶段的事务产生的日志

> 事务的 PREPARE 状态在分布式事务会出现

TRX_UNDO_FSEG_HEADER：段的 Segment Header 信息

TRX_UNDO_PAGE_LIST：是 TRX_UNDO_PAGE_NODE 形成的链表的基节点

### 20.7.3 Undo Log Header

写完一条紧接着写另一条，各条undo 日志是亲密无间的。写完一个 Undo 页面后，再从段中申请一个新页面，然后把这个页面插入到Undo 页面链表中，继续写

同一个事务向一个 Undo 页面链表中写入的undo 日志算是一个组。每写一组都会在日志前记录一些关于组的属性，存储属性的地方（Undo Log Header）

> Undo 页丽链表的第一个页面在真正写入undo 日志前， 其实都会被填充 Undo Page Header、Undo Log Segment Header、Undo Log Header 这 3 个部分

![image-20220524144352147](/Users/daydaylw3/Pictures/typora/image-20220524144352147.png)

| 属性                  | 解释                                                         |
| --------------------- | ------------------------------------------------------------ |
| TRX_UNDO_TRX_ID       | 本组undo 日志的事务id                                        |
| TRX_UNDO_TRX_NO       | 事务提交后生成的一个序号，此序号用来标记事务的提交顺序       |
| TRX_UNDO_DEL_MARKS    | 标记本组undo 日志中是否包含由delete mark 操作产生的 undo 日志 |
| TRX_UNDO_LOG_START    | 表示本组undo 日志中第一条undo 日志在页面中的偏移量.          |
| TRX_UNDO_XID_EXISTS   | 本组undo 曰志是否包含XID 信息                                |
| TRX_UNDO_DICT_TRANS   | 标记本组undo 日志是不是由DDL 语句产生的                      |
| TRX_UNDO_TABLE_ID     | 如果 TRX_UNDO_DICT_TRANS 为真，表示 DDL 语句操作的表的 table id |
| TRX_UNDO_NEXT_LOG     | 下一组undo 日志在页面中开始的偏移量                          |
| TRX_UNDO_PREV_LOG     | 上一组undo 日志在页面中开始的偏移量.                         |
| TRX_UNDO_HISTORY_NODE | 一个 History 链表的节点                                      |

> undo 链表可能会重用，因此 TRX_UNDO_NEXT_LOG、TRX_UNDO_PREV_LOG 是有用的

### 20.7.4 小结

![image-20220524144931921](/Users/daydaylw3/Pictures/typora/image-20220524144931921.png)

## 20.8 重用 Undo 页面

一个Undo 页面链表被重用， 那么它需要符合下面两个条件

+ 该链表中只包含一个Undo 页面
+ 该 Undo 页面已经使用的空间小于整个页面空间的 3/4

insert undo 和 update undo 被重用的策略

+ insert undo

  这种类型的 undo 日志在事务提交之后就没用了，可以被消除掉。直接把之前事务写入的一组undo 日志覆盖掉， 从头开始写入新事务的一组 undo 日志

+ update undo

  不能立即删除掉（MVCC）追加

## 20.9 回滚段

### 20.9.1 回滚段的概念

系统在同一时刻其实可以存在许多个Undo 页面链表

更好的管理这些链表，Rollback Segment Header 页面，存放各个 Undo 页面链表的 first undo page 的页号（undo slot）

> 链表相当于一个班，first undo page 相当于班长，Rollback Segment Header 相当于会议室，召集各个班的班长

![image-20220524161103731](/Users/daydaylw3/Pictures/typora/image-20220524161103731.png)

每一个 Rollback Segrocnt Header 页面都对应着一个段，这个段就称为回滚段 ( Rollback Segroent) 

回滚段只有一个页面

TRX_RSEG_FSEG_HEADER：这个回滚段对应的10 字节大小的Segroent Headcr 结构，通过它可以找到本回滚段对应的 INODE Entry

-个页号占用4 字节，对于1大小为16KB的页面来说， 这个TRX_ RSEG UNDO SLOTS 部分共存储了 1024 个 undo slot

### 20.9.2 从回滚段中申请 Undo 页面链表

在初始情况下，由于未向任吁事务分配任何U础页面链表，所以对于一个 Rollback Segment Header 页面面来说，它的各个undo slot 都被设置为一个特殊的值：FIL_NULL（对应的十六进制就是 0xFFFFFFFF），这表示该 undo slot 不指向任何页面

开始有事多需要分配Undo 页面链表了.于是从回滚段的第一个 undo slot 开始

如果是FIL NULL ，那么就在表空间中新创建一个段（Undo Log Segment），然后从段中申请个页面作为Undo 页面链表的 first undo page，最后把该 undo slot 的值设置为刚刚申请的这个页面的地址

如果不是FIL NULL.说明该undo slot 已经指向了一个undo 链表. 这就需要跳到下一个undo slot，判断...

如果有 1024 个 undo slot 都有用，就会报错：`Too many active concurrent transactions`

当一个事务被提交，

如果 undo slot 指向的undo 链表符合被重用的条件，undo slot 就处于被缓存的状态（TRX_UNDO_STATE属性），被缓存的 undo slot 会被加入到一个链表中。（insert undo cached 链表 或者 update undo cached 链表）

如果不符合被重用条件

+ insert undo 链表，该 Undo 页面链表的属性被设置为 TRX_UNDO_TO_FREE。之后该链表对应的段会被释放，然后 undo slot 会被设置为 FIL_NULL
+ update undo 链表，该 Undo 页面链表的属性被设置为 TRX_UNDO_TO_PRUGE。undo slot 会被设置为 FIL_NULL，然后将本次事务写入的一组 undo 日志放到 History 链表中

### 20.9.3 多个回滚段

128个回滚段，支持同时多个事务执行

对应 128个 Rollback Segment Header 页面，在系统表空间 5号页面的某个区域有 128个 8字节大小的格子

8个字节由 2 部分组成

4字节的Space ID，代表一个表空间ID

4字节的Page number，代表一个页号

8字节大小的格子相当于一个指向某个表空间中的某个页面的指针（Rollback Segment Header 页面）

> 不同的回滚段可能在不同的表空间

![image-20220524165938445](/Users/daydaylw3/Pictures/typora/image-20220524165938445.png)

### 20.9.4 回滚段的分类

0~127号回滚段

+ 0，33~127号属于一类；0号必须在系统表空间中（Rollback Segment Header）。33~127 可以在配置的 undo 表空间中

  如果一个事务在执行过程中对普通表的记录进行了改动， 得要分配Undo 页面链衰， 就必须从这一类的段中分配相应的 undo slot

+ 1~32 号属于一类。必须在临时表空间中（对应数据目录中的 ibtmp1 文件）

  如果一个事务在执行过程中对临时表的记录进行了改动。。。

> 为啥要针对普通表和临时表来划分不同种类的回滚段呢

在修改针对普通表的回滚段中的Undo 页面时，需要记录对应的redo 日志，而修改针对临时袤的回滚段中的Undo 页面时， 不需要记录对应的redo 日志.

### 20.9.5 roll_pointer 的组成

属性本质上就是一个指针，它指向一条undo 日志的地址.

![image-20220524203451791](/Users/daydaylw3/Pictures/typora/image-20220524203451791.png)

+ is_insert：是否是 TRX_UNDO_INSERT 大类的 undo 日志
+ rseg id：undo 日志的回滚段编号，0~127，7比特

### 20.9.6 为事务分配 Undo 页面链表的详细过程

## 20.10 回滚段相关配置

### 20.10.1 配置回滚段数量

innodb_rollback_segments 启动选项，配置范围 1~128。选项不影响针对临时表的回滚段数量。

1：只有1个针对普通表的可用回滚段，临时32个

2~33：效果和1相同

\> 33：针对普通表的可用回滚段数量 = -32

### 20.10.2 配置 undo 表空间

第33 - 127 号回滚段可以通过配置放到自定义的undo 表空间中

在系统初始化(创建数据目录时）时使用

+ innodb_undo_directory 指定表空间所在目录。没指定默认 undo表空间所在目录为数据目录
+ innodb_undo_tablespaces 定义 undo 表空间数量，默认0

> 如果在系统初始化时指定了创建undo 表空间， 那么系统表空间中的第0 号回滚段将处于不可用状态

33 ~ 127 号回滚段可以平均分布到不同的 undo 表空间中

设立 undo 表空间的好处是在 undo 表空间文件大到一定程度，可以自动截断（truncate）成一个小文件。系统表空间只能不断增大

## 20.11 undo 日志再崩溃恢复时的作用

为了保证事务的原子性，有必要在服务器重启时将这些未提交的事务回滚掉

我们可以通过系统表空间中第5 号页面定位到 128 个回滚段的位置，在每一个回滚段的 1024 个 undo slot 中找到值不为 FIL_NULL 的 undo slot，每个 undo slot 对应一个 undo 链表。

从 undo 链表第一个页面 Undo Segment Header 中找到 TRX_UNDO_STATE 属性，找到值为 TRX_UNDO_ACTIVE 的 undo 链表（说明有一个活跃的事务再往这个 undo 链表中写入 undo 日志），然后再在 Undo Segment Header 中找到 TRX_UNDO_LAST_LOG 属性，通过该属性找到本 undo 链表最后一个 Undo Log Header 的位置（确定该事务头和尾）。

从该 Undo Log Header 中找到对应事务的 id 和一些其他信息，该事务id就是未提交的事务。通过 undo 日志回滚

## 20.12 总结

------

[toc]