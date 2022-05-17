[toc]

------

## 13.1 统计数据的存储方式

+ 永久
+ 非永久

innodb_stats_persistent

STATS_PERSISTENT 属性

```mysql
create table 表名 (...) engine=InnoDB, STATS_PERSISTENT = (1|0);

alter table 表名 engine=InnoDB, STATS_PERSISTENT = (1|0);
```

## 13.2 基于磁盘的永久性统计数据

```mysql
mysql> show tables from mysql like 'innodb%stats';
+--------------------------------+
| Tables_in_mysql (innodb%stats) |
+--------------------------------+
| innodb_index_stats             |
| innodb_table_stats             |
+--------------------------------+
2 rows in set (0.00 sec)
```

+ innodb_index_stats：存储了关于表的统计数据，每一条记录对应着一个袤的统计数据;
+ innodb_table_stats：存储了关于索引的统计数据， 每一条记录对应着一个索号|的一个统计项的统计数据

### 13.2.1 innodb_table_stats

```mysql
mysql> select * from mysql.innodb_table_stats;
+---------------+-----------------------------+---------------------+--------+----------------------+--------------------------+
| database_name | table_name                  | last_update         | n_rows | clustered_index_size | sum_of_other_index_sizes |
+---------------+-----------------------------+---------------------+--------+----------------------+--------------------------+
| db_now_banner | now_banner2_for_qq_channel  | 2021-08-27 01:47:24 |      4 |                    1 |                        0 |
| db_now_banner | now_banner2_for_qq_live     | 2021-08-31 01:34:12 |      7 |                    1 |                        0 |
| db_now_banner | t_now_topic_white_list_data | 2021-09-04 19:52:03 |  34441 |                  161 |                      260 |
| db_now_banner | tbl_ugc_whitelist_anchor    | 2021-09-04 16:34:10 |   3324 |                   15 |                        6 |
| hongbao_wxpay | t_mch_real_info             | 2021-12-26 15:29:27 |      6 |                    1 |                        1 |
+---------------+-----------------------------+---------------------+--------+----------------------+--------------------------+
215 rows in set (0.01 sec)
```

| 字段名                    | 描述                       |
| ------------------------- | -------------------------- |
| `database_name`           |                            |
| `table_name`              |                            |
| `last_update`             |                            |
| `n_rows`                  |                            |
| `clustered_index_size`    | 表的聚簇索引占用的页面数量 |
| `sum_of_other_index_size` | 袤的其他索引占用的页面数量 |

#### 1. n_rows 统计项的收集

InnoDB 在统计一个表中有多少行记录时，套路是这样的: 按照一定算法〈并不是纯粹随机的〉从聚簇索引中选取几个叶子节点页面，统计每个页面中包含的记录数量，然后计算一个页面中平均包含的记录数量，并将其乘以全部叶子节点的数量， 结果就是该表的 n_rows 值

**innodb_stats_persistent_sample_pages 系统变量**。该值越大，统计的 n_rows 越精确，耗时越久

InnoDB 默认以表为单位来收集和存储统计数据.我们也可以单独设置某个表的采样页面的数量

```mysql
create table 表名 (...) engine=InnoDB, STATS_SAMPLE_PAGES = 具体的采样页面数量;
alter table 表名 engine=InnoDB, STATS_SAMPLE_PAGES = 具体的采样页面数量;
```

#### 2. clustered_index_size 和 sum_of_other_index_size 统计项的收集

一个索引占用2 个段，每个段由一些零散的页面以及一些完整的区构成

clustered_index_size 代表聚簇索引占用的页面数量

sum_of_other_index_size 代表其他索引总共占用的页面数量

所以在收集这两个统计项的数据时， 需要统计各个索引对应的叶子节点段和非叶子节点段分别占用的页面数量

> 当一个段的数据非常多时(超过32 个页丽) .会以区为单位来申请空间， 这里的问题是以区为单位申请空间后，有一些页可能并没有使用，但是在统计时都把它们算进去了，所以聚簇索引和其他索引实际占用的页面数可能比这两个统计项的值要小一些

### 13.2.2 innodb_index_stats

### 13.2.3 定期更新统计数据

+ 开启 innodb_stats_auto_recalc

  默认打开，异步，变动记录数量超过表大小 10%

  建表或者修改表指定 STATS_AUTO_RECALC 属性（1、0）

+ 手动调节 ANALYZE TABLE 语句来更新统计信息

  ```mysql
  ANALYZE TABLE single_table;
  ```

  同步的

### 13.2.4 手动更新 innodb_table_stats 和 innodb_index_stats 表

更新

优化器重新加载

```mysql
FLUSH TABLE 表名;
```

## 13.3 基于内存的非永久性统计数据

统计数据采样的页丽数量是由系统变量 innodb_stats_transient_sample_pages 控制，默认 8

## 13.4 innodb_stats_method 的使用

索引列中不重复的值的数量对于MySQL 优化器十分重要，通过它可以计算出在索引列中一个值平均重复多少行

+ 单表查询中的单点扫描区间太多

  直接依赖统计数据中一个值平均重复多少行来计算单点扫描区间对应的记录数量

+ 在执行连接查询时，如果有涉及两个表的等值匹配连接条件， 该连接条件对应的被驱动表中的列又拥有索引时，则可以使用ref 访问方法来查询被驱动表

------

[toc]