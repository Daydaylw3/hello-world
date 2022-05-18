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

------

[toc]