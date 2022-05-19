[toc]

------

### 15.1.1 table

explain 语句输出的每条记录都对应着某个**单表**的访问方式，table 列代表该单表的表名

```mysql
explain select * from s1 inner join s2;
```

### 15.1.2 id

一条查询语句出现多个 select 关键字，就会有多个 id

+ ```mysql
  select * from s1 where key1 in (select key3 from s2);
  ```

+ ```mysql
  select * from s1 union select * from s2;
  ```

一个 select 后的 from 子句可以跟多个表；

在连接查询的执行计划中，每个表都会对应一条记录，这些记录的 id 是相同的；出现在前面的是驱动表，出现在后面的是被驱表

> 如果想知道查询优化器对某个包含子查询的语句是否进行了重写

```mysql
explain select * from s1 where key1 in (select key3 from s2 where common_field = 'a');
```

查询优化器将子查询转换为了连接查询

### 15.1.3 select_type

取值

| 名称                 | 描述 |
| -------------------- | ---- |
| SIMPLE               |      |
| PRIMARY              |      |
| UNION                |      |
| UNION RESULT         |      |
| SUBQUERY             |      |
| DEPENDENT SUBQUERY   |      |
| DEPENDENT UNIOIN     |      |
| DERIVED              |      |
| MATERIALIZED         |      |
| UNCACHEABLE SUBQUERY |      |
| UNCACHEABLE UNION    |      |

+ SIMPLE：查询语句中不包含UNION 或者子查询的查询

+ PRIMARY：对于包含UNION 、UNION ALL 或者子查询的大查询来说，其中最左边那个查询的select_type 值就是PRlMARY

  ```
  explain select * from s1 union select * from s2;
  ```

+ UNION：其余的小查询

+ UNION RESULT：临时表完成 UNION，针对该临时表的查询

+ SUBQUERY：如果包含子查询的查询语句不能够转为对应的半连接形式，并且该子查询是不相关子查询，，而且查询优化器决定采用将该子查询物化的方案来执行该子查询时，该子查询的第一个SELECT 关键字代表的那个查询

  ```
  explain select * from s1 where key1 in (select key1 from s2) or key3 = 'a';
  ```

  > 由于select_type 为SUBQUERY 的子查询会被物化，所以该子查询只需要执行一遍

+ DEPENDENT SUBQUERY：如果包含子查询的查询语句不能够转为对应的半连接形式，并且该子查询被查询优化器转换为相关子查询的形式，，则该子查询的第一个 SELECT 

  ```
  explain select * from s1 where key1 in (select key 1 from s2 where s1.key2 = s2.key2) or key3 = 'a';
  ```

  > 子查询可能会被执行多次

+ DEPENDENT UNION：在包含UNION 或者UNION ALL 的大查询中，如果各个小查询部依赖于外层查询，则除了最左边的那个小查询之外，其余小查询的

  ```mysql
  explain select * from s1 where key1 in (select key1 from s2 where key1 = 'a' union select key1 from s1 where key1 = 'b');
  ```

+ DERIVED：在包含派生表的查询中，如果是以物化派生表的方式执行查询，则派生表对应的子查询

  ```
  explain select * from (select key1, count(*) as c from s1 group by key1) as derived_s1 where c > 1;
  ```

+ MATERIALIZED：当查询优化器在执行包含子查询的语句时，选择将予查询物化之后与外层查询进行连接查询，该子查询对应的

  ```
  explain select * from s1 where key1 in (select key1 from s2);
  ```

### 15.1.4 partitions

### 15.1.5 type

system、const、eq_ref、ref、

### 15.1.7 key_len

从执行计划中直接可以看出形成扫描区间的边界条件是什么。

key_len 的值由 3 部分组成

+ **该列的实际数据最多占用的存储空间长度**。对于固定长度类型的列来说，比方说对于 INT 类型的列来说，该历史记数据最多占用的存储空间长度就是 4 字节。对于变长类型的列来说，对于变长的来说，比方说对于使用 utf8 字符集，类型为 VARCHAR(100) 来说，该列的实际数据最多占用的存储空间长度就是在 utf8 字符集中表示一个字符最多占用的字节数，乘以该类型最多可以存储的字符数的积。也就是 3 × 100 = 300 字节
+ 如果该列**可以存储 NULL 值**，则 key_len 值在该列的实际数据最多占用的存储空间长度的基础上，再加 1 字节
+ 对于使用变长类型的列来说，都会有 2 字节的空间来存储该变列的**实际数据占用的存储空间长度**，key_len 值还要在原先的基础上加 2 字节。

```mysql
explain select * from s1 where key_part1 = 'a' and key_part3 = 'a';
```

```mysql
explain select * from s1 where key_part1 = 'a' and key_part2 > 'b';
```

### 15.1.8 ref

展示的是与索引列进行等值匹配的东西是什么？比如只是一个常数或者是某个列，有时是个函数 func

### 15.1.9 rows

在查询优化器。决定使用全表扫描对某个表进行查询时，执行计划的 row 列就代表该表的估计行数。

如果使用索引来执行扫描，就代表预计扫描的索引记录行数

### 15.1.10 filtered

条件过滤（condition filtering）的概念

+ 如果使用全表扫描的方式来执行单表查询。那么计算驱动表扇出时，需要估计出满足全部搜索条件的记录到底有多少条？
+ 如果使用索引来执行单表扫描，那么计算驱动表扇出时需要估计出在满足形成索引扫描区间的搜索条件外，还满足其他搜索条件的记录有多少条？

```mysql
explain select * from s1 where key1 > 'z' and common_field = 'a';
```

rows 的值是 266，说明预测有 266 条记录满足 key1 > 'z' 的条件，filtered 的值是 10.00，表示预测出 266 条记录中，10%的满足 common_field = 'a' 的条件

对于单表查询来说，这个列的值没有什么意义，我们更关注在连接查询中驱动表对应的执行计划的 filtered 值。

```mysql
explain select * from s1 inner join s2 on s1.key1 = s2.key1 where s1.common_field = 'a';
```

### 15.1.11 Extra

+ No tables used：没有 From 子句

+ Impossible WHERE：where子句永远为 false

+ No matching min/max row：

+ Using index：使用覆盖索引执行查询

+ Using index condition：有些搜索条件虽然出现了索引列，但是却不能充当边界条件来形成扫描区间

  ```mysql
  explain select * from s1 where key1 > 'z' and key1 like '%a';
  ```

  这里有个前后版本的索引下推（Index Condition Pushdown）的优化行为（自行复习查看）

+ Using where：当某个搜索条件需要在 server 层进行判断（8太好

+ Using join buffer（Block Nested Loop）：被驱动表不能有效地利用索引加快访问速度，分配一块缓冲区

+ Using intersect(...)、Using union(...)、Using sort_union(...)：

+ Zero limit：

+ Using filesort：排序无法使用索引，需要在内存中或者磁盘中排序

+ Using  temporary：建立临时表（不太好

+ Start temporary，End temporary：子查询将 in 子查询转换为半连接。建立临时表为外层查询中的记录去重

+ LooseScan：in 子查询转换为半连接

+ FirstMatch(tbl_name)：

## 15.2 JSON 格式的执行计划

`FORMAT=JSON`

prefix_cost：

eval_cost：

read_cost：

## 15.3 Extented EXPLAIN

```
SHOW WARNINGS
```

## 15.4 总结



------

[toc]