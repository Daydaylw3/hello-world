[toc]

----

第 1 章曾经讲过， MySQL Sever 有一个称为优化器的模块. MySQL Server 在对一条查询语句进行语法解析之后，就会将其交给优化器来优化，优化的结果就是生成一个所谓的**执行计划**。

这个执行计划表明了应该使用哪些索引进行查询、表之间的连接顺序是啥样的；等等。

MySQL 怎么执行单表查询的

```mysql
CREATE TABLE single_table (
	id INT NOT NULL AUTO_INCREMENT,
	key1 VARCHAR(100),
	key2 INT,
	key3 VARCHAR(100),
	key_part1 VARCHAR(100),
	key_part2 VARCHAR(100),
	key_part3 VARCHAR(100),
	common_field VARCHAR(100),
	PRIMARY KEY (id),
	KEY idx_key1 (key1),
	UNIQUE KEY uk_key2 (key2),
	KEY idx_key3 (key3),
	KEY idx_key_part(key_part1, key_part2, key_part3)
) Engine=InnoDB CHARSET=utf8;
```

## 10.1 访问方法的概念

MySQL 执行查询语句的方式称为访问方法（access method）或者访问类型。

同一个查询语句可以使用不同的访问方法来执行，是不同的访问方法花费的时间成本可能差距甚大

## 10.2 const

可以通过主键列来定位一条记录，MySQL 会直接利用主键值在聚簇索引中定位对应的用户记录

```mysql
select * from single_table where id = 1438;
```

根据唯一二级索引列来定位一条记录的速度也是贼快的

```mysql
select * from single_table where key2 = 3841;
```

>  MySQL 的认为，通过主键或者唯一二级索引列与常数的等值比较来定位一条记录是像坐火箭一样快的，所以他们把这种通过主键或者唯一二级索号列来定位一条记录的访问方法定义为 const（意思是常数级别的， 代价是可以忽略不计的）。

不过这种coost 访问方法只能在主键列或者唯一二级索引列与一个常数进行等值比较时才有效.

如果主键或者唯一二级索引的索引列由多个列构成，则只有在索引列中的每一个列都与常数进行等值比较时，这个const 访问方法才有效（这是因为只有在该索引的每一个列都采用等值比较时，才可以保证最多只有一条记录符合搜索条件）

对于唯一二级索引列来说， 在查询列为NULL 值时，情况比较特殊。因为唯一二级索引列并不限制NULL 值的数量， 所以可能访问到多条记录。也就是说不可以使用const 访问方法来执行

## 10.3 ref

有时，我们需要将某个普通的二级索引列与常数进行等值比较

```mysql
select * from single_talbe where key1 = 'abc';
```



----

[toc]