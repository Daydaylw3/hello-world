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

+ MLOG_REC_INSERT（9）：表示在插入一条使用非紧凑行格式( REDUNDANT)的记录
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



------

[toc]