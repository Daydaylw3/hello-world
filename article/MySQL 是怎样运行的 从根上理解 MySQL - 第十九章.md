[toc]

------

## 19.2 redo 日志是啥

我们只是想让已经提交了的事务对数据库中的数据所做的修改能永久生效，即使后来系统崩溃， 在重启后也能把这种修改恢复过来

把修改的内容记录一下，系统崩溃重启后，重做一次

redo 日志占用的空间很小

redo 日志是顺序写盘，使用了顺序 I/O

## 19.3 redo 日志格式

有多种类型的 redo 日志，但是都有通用结构

| type | space ID  | page number | data                   |
| ---- | --------- | ----------- | ---------------------- |
| 类型 | 表空间 ID | 页号        | 某条 redo 日志具体内容 |

### 19.3.1 简单的 redo 日志类型

row_id 的例子，隐藏列赋值方式

物理日志，写入数据的多少划分 redo 日志类型：

MLOG_1BYTE、

MLOG_2BYTE、

MLOG_4BYTE、

MLOG_8BYTE、

MLOG_WRITE_STRING

### 19.3.2 复杂一些的 redo 日志类型

在执行一条语句时会修改非常多的页面（系统页面，用户页面）

+ 表中包含多少个索引，一条 INSERT 语句就可能更新多少裸B+ 树.
+ 针对某一棵 B+ 树来说，既可能更新叶子节点页面，也可能更新内节点页面， 还可能创建新的页面

+ MLOG_REC_INSERT（9）：表示在插入一条使用非紧凑行格式（REDUNDANT）的记录
+ MLOG_CMP_REC_INSERT（38）：表示在插入一条使用紧凑行格式( COMPACT、DYNAMIC、COMPRESSED ) 的记录
+ MLOG_COMP_PAGE_CREATE（58）：表示在创建一个存储紧凑行格式记录的页面
+ MLOG_COMP_REC_DELETE（42）：表示在删除一条使用紧凑行格式记录
+ MLOG_COMP_LIST_START_DELETE（44）：表示在从某条给定记录开始删除草面中一系列使用紧凑行格式的记录
+ MLOG_COMP_LIST_END_DELETE（43）：表示删除一系列记录，直到 MLOG_COMP_LIST_START_DELETE 类型的 redo 日志对应的记录为止
+ MLOG_ZIP_PAGE_COMPRESS：

这些类型的redo 日志既包含物理层面的意思，也包含逻辑层面的意思

+ 物理：指明对哪个表空间的哪个页
+ 逻辑：不能直接根据这些日志中的记载，在页面内的某个偏移量处恢复某数据， 而是需要调用一些事先准备好的函数

> redo 日志并没有记录 PAGE_N_DIR_SLOTS 等值被修改成什么样，而是把本页面中插入一条记录所必备的要素记了下来。

## 19.4 Mini-Transaction

### 19.4.1 以组的形式写入 redo 日志

在执行语句的过程中产生的 redo 日志，被 InnoDB 划分成了若干个不可分割的组

+ 有些需要保证原子性的操作会生成多条 redo 日志

在该组中的最后一条 redo 日志后面加上一条特殊类型的 redo 日志。该类型的 redo 日志的名称为 MLOG_MULTI_REC_END，某个需要保证原子性的操作所产生的一系列 redo 日志，必须以一条类型为 MLOG_MULTI_REC_END 的 redo 日志结尾。

这样在系统因崩溃而重启恢复时，只有解析到类型为 MLOG_MULTI_REC_END 的 redo 日志才认为解析到一组完整的 redo 日志

+ 有的原子性操作只有一条 redo 日志

type 字段的第一个比特位为 1。

### 19.4.2 Mini-Transaction 的概念

对!高层页面进行一次原子访问的过程，MTR

一个 MTR 可以包含一组 redo 日志

## 19.5 redo 日志的写入过程

### 19.5.1 redo log block

把通过 MTR 生成的 redo 日志都放在大小为 512 字节的页中（block）

<img src="/Users/daydaylw3/Pictures/typora/image-20220520155626950.png" alt="image-20220520155626950" style="zoom:80%;" />

LOG_BLOCK_HDR_DATA_LEN：block 已经使用了多少字节；从 12 开始

LOG_BLOCK_FIRST_REC_GROUP：该 block 中第一个 MTR 生成的第一条 redo 日志的偏移量（如果一个 MTR 横跨很多 block，最后一个 block 的该属性代表这个 MTR 对应结束的地方）

### 19.5.2 redo 日志缓冲区

redo log buffer

启动选项 innodb_log_buffer_size

### 19.5.3 redo 日志写入 log buffer

顺序写入，buf_free 全局变量指示写的哪个 block 哪个偏移量

一个 MTR 结束时才一起写入 log buffer

## 19.6 redo 日志文件

### 19.6.1 redo 日志刷盘时机

+ log buffer 空间不足（总量 50%
+ 事务提交时
+ 后台线程（大约每秒一次
+ 正常关闭服务器
+ 做 checkpoint

### 19.6.2 redo 日志文件组

ib_logfile0、ib_logfile1

+ innodb_log_group_home_dir：
+ innodb_log_file_size：大小，
+ innodb_log_files_in_group：文件个数，默认 2，最大值 100

循环写，从 0 开始往后写

![image-20220520163728323](/Users/daydaylw3/Pictures/typora/image-20220520163728323.png)

### 10.6.3 redo 日志文件格式

redo 日志文件也是由若干个 512 字节大小的 block 组成

redo 日志文件

+ 前 2048 字节（4 个 blcok）存储管理信息
+ 后面的存储 log buffer 的 block 镜像

> 循环写是从 2048 字节开始写

**前 2048 个字节**

![image-20220522163432014](/Users/daydaylw3/Pictures/typora/image-20220522163432014.png)

+ LOG_HEADER_START_LSN：2048 字节处对应的 lsn 值
+ LOG_HEADER_CREATOR：创建者，正常为 MySQL版本号，SQL 5.7.22，mysqlbackup 命令创建时是 ibbackup + 创建时间

![image-20220522163737457](/Users/daydaylw3/Pictures/typora/image-20220522163737457.png)

+ LOG_CHECKPOINT_NO：每执行一次checkpoint，就+1
+ LOG_CHECKPOINT_LSN：服务器再结束 checkpoint 时对应的 lsn 值；系统在崩溃会恢复时将从该值开始
+ LOG_CHECKPOINT_OFFSET：上个属性中的 lsn 值在 redo 日志文件组中的偏移量

> 革统中 checkpoint 的相关信息其实只存储在redo 日志文件组的第一个日志文件中

## 19.7 log sequence number

lsn（log sequence number）记录总共已写入的 redo 日志量，初始 8704

统计 lsn 增长量时，是按照实际写入的日志量 + log block header 和 log block trailer 来计算的

> 每一组由 MTR 生成的redo 日志都有一个唯一的 lsn 值与其对应；Isn 值越小，说明redo 日志产生得越早

### 19.7.1 flushed_to_disk_lsn

buf_next_to_write 全局变量：用来标记当前 log buffer 中已经有哪些日志被刷新到磁盘中

flushed_to_disk_lsn 全局变量：表示刷新到磁盘中的 redo 日志量，和 lsn 对应

![image-20220523145646351](/Users/daydaylw3/Pictures/typora/image-20220523145646351.png)

![image-20220523145658306](/Users/daydaylw3/Pictures/typora/image-20220523145658306.png)

### 19.7.2 lsn 值和 redo 日志文件组中的偏移量的对应关系

很容易计算某一个 lsn 值在 redo 日志文件组中的偏移量

<img src="/Users/daydaylw3/Pictures/typora/image-20220523150044312.png" alt="image-20220523150044312" style="zoom:67%;" />

### 19.7.3 flush 链表中的 lsn

一个MTR 代表对底层页面的一次原子访问，在访问过程中可能会产生一组不可分割的redo 日志；在MTR 结束时，会把这一组redo 日志写入到 log buffer 中

在MTR 结束时还要把在MTR 执行过程中修改过的页面加入到 Buffer Pool 的 flush 链表中

当第一次修改某个已经加载到 Buffer Pool 中的页面时， 就会把这个页面对应的控制块插入到 flush 链表的头部；再次修改，不再次插入；

>  flush 链表中的脏页是按照页面首次修改时间进行排序的

+ oldest_modification：第一次修改，将修改该页面的 MTR 开始时的 lsn 写入
+ newest_modification：每一次修改，将修改页面结束的 MTR 对应的 lsn 写入

## 19.8 checkpoint

循环使用 redo 日志组文件

判断某些redo 日志占用的磁盘空间是否可以覆盖的依据，就是它对应的脏页是否已经被刷新到了磁盘中

![image-20220523155131132](/Users/daydaylw3/Pictures/typora/image-20220523155131132.png)

虽然mtr_1 和mtr_2 生成的redo 日志都已经写到了磁盘中， 但是它们修改的脏页仍然留在Butfer Pool 中，所以它们生成的r附日志在磁盘中的空间是不可以被覆盖的。之后随着系统的运行，如果页a 被刷新到了磁盘，那么页a 对应的控制块就会从flush 链表中移除，这样mtr_1 生成的 redo 日志就没有用了，这些redo 日志占用的磁盘空间就可以被覆盖掉

checkpoint_lsn 全局变量：表示当前系统中可以被覆盖的redo 日志总量，初始8704

执行一次 checkpoint：页被刷新到磁盘上，对应的 mtr 生成的 redo 日志就可以被覆盖了，进行一个增加 checkpoint_lsn 的操作

> 脏页刷盘和执行一次checkpoint是两回事。

checkpoint 步骤

1. 计算当前系统中可以被覆盖章的redo 日志对应的 Isn 值最大是多少：当前系统中最早修改的脏页（flush 链表尾节点）对应的 oldest_modification 值
2. 将 checkpoint_lsn 与对应的 redo 日志文件组偏移量以及此次 checkpoint 的编号写到日志文件的管理信息（cp1，cp2）

checkpoint_no 变量：统计系统执行了多少次 checkpoint

当 checkpoint_no 是奇数 --> checkpoint 信息写到 checkpoint1，

是偶数 --> 写到 checkpoint2

> 记录完checkpoint 的信息之后. redo 日志文件组中各个Isn 值的关系如图

![image-20220523162929348](/Users/daydaylw3/Pictures/typora/image-20220523162929348.png)

## 19.9 用户线程批量从 flush 链表中刷出脏页

## 19.10 查看系统中的各种 lsn 值

```
SHOW ENGINE INNODB STATUS\G;
...
LOG
---
Log sequence number          83216130
Log buffer assigned up to    83216130
Log buffer completed up to   83216130
Log written up to            83216130
Log flushed up to            83216130
Added dirty pages up to      83216130
Pages flushed up to          83216130
Last checkpoint at           83216130
246 log i/o's done, 0.00 log i/o's/second
---
```

+ Pages flushed up to 表示fiush 链表中被最早修改的那个页面对应的 oldest_modification 属性值
+ Last checkpoint at：checkpoint_lsn

## 19.11 innodb_flush_log_at_trx_commit

+ 0：表示在事务提交时不立即向磁盘同步redo 日志，这个任务交给后台线程来处理。服务器挂了事务可能丢失
+ 1：表示在事务提交时需要将redo 日志同步到磁盘； 默认值
+ 2：表示在事务提交时需要将redo 日志写到操作系统的缓冲区中，但并不需要保证将日志真正地刷新到磁盘。操作系统不挂就行

## 19.12 崩溃恢复

### 19.12.1 确定恢复的起点

从 checkpoint_lsn 开始，从日志文件组中第一个文件的管理信息中，checkpoint1 和 checkpoint2 比对大小，大的说明是最近一次

### 19.12.2 确定恢复的终点

扫描 redo 日志文件中的block，直到某个block 的 LOG_BLOCK_HDR_DATA_LEN 值不等于 512 为止（block 没满）

block被填满 --> LOG_BLOCK_HDR_DATA_LEN = 512

### 19.12.3 恢复

加快恢复过程

+ 使用哈希表（space ID 和 page number 做哈希值），避免很多页面读取的随机 I/O

+ 跳过己经刷新到磁盘中的页面

  页面 FIL_PAGE_LSN > checkpoint_lsn 就跳过

## 19.13 LOG_BLOCK_HDR_NO 计算

```
((lsn / 512) & 0x3FFFFFFF) + 1
```

最大 1G

LOG_BLOCK_HDR_NO 值的第一个比特比较特殊，称为flush hit. 如果该值为1，代表本b10ck 是在将10g buffer 中的 block 刷新到磁盘的某次操作中时，第一个被刷入的 block

------

[toc]