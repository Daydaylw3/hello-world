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



------

[toc]