[toc]

------

页和记录的关系示意图

![image-20220511155852777](/Users/daydaylw3/Pictures/typora/image-20220511155852777.png)

>  在通过主键查找某条记录的时候可以在页目录中使用二分法快速定位到对应的槽，然后再遍历该槽对应分组中的记录即可快速找到指定的记录

## 6.1 没有索引时进行查找 

```mysql
SELECT [查询列表] FROM 表名 WHERE 列名 = xxx;
```

先只讨论某个列等于某个常数的情况

### 6.1.1 在一个页中查找

+ 以主键为搜索条件：二分法，遍历槽
+ 以其他列为搜索条件：从 `Infimum` 记录开始依次遍历每条记录

### 6.1.2 在很多页中查找

+ 定位到记录所在页
+ 从所在页内查找

>  在没有索引的情况下， 无论是根据主键列还是其他列的值进行查找，由于我们不能快速地定位到记录所在的页，所以只能从第一页沿着双向链表一直往下找。

## 6.2 索引

先建一个表

```mysql
mysql> CREATE TABLE index_demo (
    -> c1 INT,
    -> c2 INT,
    -> c3 CHAR(1),
    -> PRIMARY KEY(c1)
    -> ) ROW_FORMAT = COMPACT;
Query OK, 0 rows affected (0.38 sec)
```

简化 `index_demo` 表的行格式示意图

<img src="/Users/daydaylw3/Pictures/typora/image-20220511160854457.png" alt="image-20220511160854457" style="zoom:67%;" />

竖放

![image-20220511161009527](/Users/daydaylw3/Pictures/typora/image-20220511161009527.png)

### 6.2.1 一个简单的索引方案

可以想办法为快速定位记录所在的数据页而建立一个别的目录，在建这个目录的过程中必须完成两件事

1. 下一个数据页中用户记录的主键值必须大于上一个页中用户记录的主键值
2. 给所有的页建立一个目录项

> 假设：每个数据页最多能存放3 条记录

```mysql
mysql> INSERT INTO index_demo VALUES (1, 4, 'u'), (3, 9, 'd'), (5, 3, 'y');
Query OK, 3 rows affected (0.14 sec)
Records: 3  Duplicates: 0  Warnings: 0
```

![image-20220511162409362](/Users/daydaylw3/Pictures/typora/image-20220511162409362.png)

> index demo 表中的3 条记录都被插入到编号为10 的数据页中.此时再插入一条记录

```mysql
mysql> INSERT INTO index_demo VALUES (4, 4, 'a');
Query OK, 1 row affected (0.01 sec)
```

![image-20220511162519582](/Users/daydaylw3/Pictures/typora/image-20220511162519582.png)

> 新分配的数据页编号可能并不是连续的

页10 中用户记录最大的主键值是5 . 而页28 中有一条记录的主键值是4. 因为5 > 4. 所以这就不符合"下一个数据页中用户记录的主键值必须大于上一个页中用户记录的主键值"的要求，所以在插入主键值为4 的记录时需要伴随着一次记录移动

![image-20220511162737685](/Users/daydaylw3/Pictures/typora/image-20220511162737685.png)

>  在对页中的记录进行增删改操作的过程中，我们必须通过一些诸如记录移动的操作来始终保证这个状态一直成立.下一个数据页中用户记录的主键值必须大于上一个页中用户记录的主键值.这个过程也可以称为**页分裂**

由于数据页的编号可能并不是连续的，所以在向index demo 表中插入许多条记录后， 可能会形成如下的效果

![image-20220511163002734](/Users/daydaylw3/Pictures/typora/image-20220511163002734.png)

> 想从这么多页中根据主键值快速定位某些记录所在的页，就需要给它们编制一个目录，每个页对应一个**目录项**

每个目录项包括两个部分

+ **页的用户记录中最小的主键值**，用key 来表示
+ **页号**，用page_no 表示.

![image-20220511163145104](/Users/daydaylw3/Pictures/typora/image-20220511163145104.png)

只需要把几个目录项在物理存储量苦上连续存储，比如把它们放到一个数组中，就可以实现根据主键值快速查找某条记录的功能了

> 这个目录有一个别名，称为**索引**

### 6.2.2 InnoDB 中的索引方案

设计InnoDB 的大叔需要一种可以灵活管理所有目录项的方式.他们发现这些目录项其实与用户记录长得很像，只不过目录项中的两个列是主键和页号而己， 所以他们灵光乍现，复用了之前存储用户记录的数据页来存储目录项.为了与用户记录进行区分， 我们把这些用来表示目录项的记录称为**目录项记录**。

InnoDB 使用记录头信息中的 record_ type 属性（1：目录项记录）区分一条记录是普通的用户记录还是目录项记录

![image-20220511173053340](/Users/daydaylw3/Pictures/typora/image-20220511173053340.png)

> 新分配了一个编号为3 0 的页来专门存储目录项记录

目录项记录和普通的用户记录的不同点

+ 目录项记录的record_ type 值是1，普通用户记录的re∞rd_type 值是0 。
+ 目录项记录只有主键值和页的编号两个列；普通用户记录的列是用户自己定义的，可能包含很多列，另外还有InnoDB 自己添加的隐藏列
+ 记录头信息名为min_recflag 的属性，只有目录项记录的属性才可能为1，普通用户记录的属性都是0

相同

+ 数据页(页面类型都是0x45BF ，这个属性在File Header 中
+ 页的组成结构也是一样的
+ 都会为主键值生成Page Directory( 页目录），从而在按照主键值进行查找时可以使用二分法来加快查询速度

如果表中的数据太多，以至于一个数据页不足以存放所有的目录项记录，再多整一个存储目录项记录的页

> 假设一个存储目录项记录的页最多只能存放4 条目录项记录

如果此时再向表 中插入一条主键值为320 的用户记录，那就需要分配一个新的存储目录项记录的页

![image-20220511184353763](/Users/daydaylw3/Pictures/typora/image-20220511184353763.png)

> 在插入了一条主键值为320 的用户记录之后，需要两个新的数据页

+ 为存储该用户记录而新生成了页31
+ 一个新的页32 来存放页31 对应的目录项目录.

**想根据主键值查找一条用户记录的步骤**

1. 确定存储目录项记录的页
2. 通过存储目录项记录的页确定用户记录真正所在的页
3. 在真正存储用户记录的页中定位到具体的记录

> 查询步骤的步骤 1 中， 我们需要定位存储目录项记录的页，如果表中的数据非常多，则会产生很多存储目录项记录的页，那我们怎么根据主键值快速定位一个存储目录项记录的页呢？

为这些存储目录项记录的页再生成一个更高级的目录，就像是一个多级目录一样

![image-20220511184713930](/Users/daydaylw3/Pictures/typora/image-20220511184713930.png)

> 生成了一个存储更高级目录项记录的页33 . 这个页中的两条记录分别代表页3 0 和页32. 如果用户记录的主键值在[1 ， 320) 之间，则到页30 中查找更详细的目录项记录;如果主键值不小子320 ，就到页32 中查找更详细的目录项记录

其实这是一种组织数据的形式，或者说是一种数据结构，它的名称是**B+ 树**

+ 真正的用户记录其实都存放在B+ 树最底层的节点上，这些节点也称为**叶子节点**或**叶节点**
+ 其余用来存放目录项记录的节点称为**非叶子节点**或者**内节点**

![image-20220511191337424](/Users/daydaylw3/Pictures/typora/image-20220511191337424.png)

一个B+ 树的节点其实可以分成好多层，InnoDB 规定最下面的那层〈也就是存放用户记录的那层〉为第0 层，之后层级依次往上加

> 数据页的Page Header 部分的 PAGE_LEVEL 的，它就代表着这个数据页作为节点在B+ 树中的层级.

#### 1. 聚簇索引

B+ 树本身就是一个目录，或者说本身就是一个索引， 如果有下面两个特点

+ 使用记录主键值的大小进行记录和页的排序
  1. 页〈包括叶子节点和内节点〉内的记录按照主键的大小顺序排成一个单向链表，页内的记录被划分成若干个组，每个组中主键值最大的记录在页内的偏移量会被当作槽依次存放在页目录中，可以在页目录中通过二分法快速定位到主键列等于某个值的记录
  2. 各个存放用户记录的页也是根据页中用户记录的主键大小顺序排成一个双向链表
  3. 存放目录项记录的页分为不同的层级， 在同一层级中的页也是根据页中目录项记录的主键大小顺序排成一个双向链表
+ B+ 树的叶子节点存储的是**完整的用户记录**（指这个记录中存储了所有列的值，包括隐藏列）

把具有这两个特点的B+ 树称为**聚簇索引**

+ 所有完整的用户记录都存放在这个聚簇索引的叶子节点处
+ 在 `InnoDB` 存储引擎中， 聚簇索引就是数据的存储方式

#### 2. 二级索引

> 如果我们想以别的列作为搜索条件该怎么办？

比如，用 c2 列的大小作为数据页、页中记录的排序规则， 然后再建一棵B+ 树

![image-20220511195543947](/Users/daydaylw3/Pictures/typora/image-20220511195543947.png)

这个B+ 树与前文介绍的聚簇索引有几处不同

+ 使用记录c2 列的大小迸行记录和页的排序， 这包括3 方面的含义.
  1. 页(包括叶子节点和内节点)内的记录是按照c2 列的大小顺序排成一个单向链表，页内的记录被划分成若平个组， 每个组中c2 列值最大的记录在页内的偏移量会被当作槽依次存放在页目录中，可以在页目录中通过二分法快速定位到c2 列等于某个值的记录
  2. 各个存放用户记录的页也是根据页中记录的c2 列大小顺序排成一个双向链表.
  3. 存放目录项记录的页分为不同的层级，在同一层级中的页也是根据页中目录项记录的c2 列大小顺序排成一个双向链表
+ B+ 树的叶子节点存储的并不是完整的用户记录，而只是c2 列+主键这两个列的值
+ 目录项记录中不再是主键+页号的搭配， 而变成了c2 列+页号的搭配

> 比方说我们想查找满足搜索条件c2=4的记录，就可以使用刚刚建好的这棵B+ 树

**注意**

+ 因为c2 列并没有唯一位约束，也就是说满足搜索条件c2=4的记录可能有很多条
+ 只需要在该B+ 树的叶子节点处定位到第一条满足搜索条件c2=4的那条记录，然后沿着由记录组成的单向链表一直向后扫描即可
+ 各个叶子节点组成了双向链衰，搜索完了本页面的记录后可以很顺利地跳到下一个页面中的第一条记录，然后继续沿着记录组成的单向链表向后扫描

**回表**：通过携带主键信息到聚簇索引中重新定位完整的用户记录的过程也称为回表

这种以非主键列的大小为排序规则而建立的B+ 树需要执行回表操作才可以定位到完整的用户记录，所以这种B+ 树也称为二级索引（Secondary Index）或辅助索引。

#### 3. 联合索引

> 同时以多个列的大小作为排序规则，也就是同时为多个列建立索引

比如，让B+ 树按照c2 和c3 列的大小进行排序，这里面包含两层含义

+ 先把各个记录和页按照c2 列进行排序
+ 在记录的c2 列相同的情况下，再采用c3 列进行排序.

![image-20220511201102994](/Users/daydaylw3/Pictures/typora/image-20220511201102994.png)

+ 每条目录项记录都由c2 列、c3 列、页号这3 部分组成.各条记录先按照c2 列的值进行排序， 如果记录的c2 列相同，则按照c3 列的值进行排序
+ B+ 树叶子节点处的用户记录由c2 列、c3 列和主键c1 列组成。

联合索引，也称为复合索引或多列索引，它本质上也是一个二级索引

### 6.2.3 InnoDB 中 B+ 树索引的注意事项

#### 1. 根页面万年不动窝

实际上B+ 树的形成过程是下面这样的

+ 每当为某个表创建一个B+ 树索引（聚簇索引不是人为创建的， 它默认就存在）时，都会为这个索引创建一个根节点页面.最开始表中没有数据的时候，每个B+ 树索引对应的根节点中既没有用户记录，也没有目录项记录
+ 随后向表中插入用户记录时， 先把用户记录存储到这个根节点中.
+ 在根节点中的可用空间用完时继续插入记录，此时会将根节点中的所有记录复制到一个新分配的页(比如页a) 中， 然后对这个新页进行页分裂操作，得到另一个新页(比如页b ). 这时新插入的记录会根据键值(也就是聚簇索引中的主键值，或二级索引中对应的索引列的值〉的大小分配到页a 或页b 中。
+ 根节点此时便升级为存储目录项记录的页，也就需要把页a 和页b 对应的目录项记录插入到根节点中。

> 在这个过程中，需要特别注意的是， 一个B+ 树索引的根节点自创建之日起便不会再移动（也就是页号不再改变）。

这样只要我们对某个表建立一个索引，那么它的根节点的页号便会被记录到某个地方，后续凡是InnoDB 存储引擎需要用到这个索引时，都会从那个固定的地方取出根节点的页号，从而访问这个索引

#### 2. 内节点中目录项记录的唯一性

> 在B+ 树索引的内节点（非叶子节点）中， 目录项记录的内容是索引列加页号的搭配，但是这个搭配对于二级索引来说有点儿不严谨

二级索引的内节点的目录项记录的内容实际上是由3 部分构成的

+ 索引列的值
+ 主键值
+ 页号

> 对于二级牵引记录来说， 是先按照二级牵引列的位进行排序，在二级索引直相同的情况下，再按照主键位进行排序.

> 对于唯一二级索引来说，也叮能会出现多条记录键值相同的情况（多个 NULL 值，MVCC 服务），唯一二级索引的内节点的目录项记录也会包含记录的主键值

#### 3. 一个页面至少容纳2 条记录

> InnoDB 的一个数据页至少可以存放2 条记录

### 6.2.4 MyISAM 中索引方案简介

> MyISAM 的索引方案虽然也使用树形结构，但是却将索引和数据分开存储

+ 将表中的记录按照记录的插入顺序单独存储在一个文件中(称之为数据文件).这个文件并不划分为若干个数据页，有多少记录就往这个文件中塞多少记录.这样一来可以通过行号快速访问到一条记录
+ MyISAM 记录也需要记录头信息来存储一些额外数据
+ 使用 MyISAM 存储引擎的表会把索引信息单独存储到另外一个文件中〈称为索引文件）
+ MylSAM 会为袤的主键单独创建一个索引， 只不过在索引的叶子节点中存储的不是完整的用户记录，而是主键值与行号的组合
+ 其他列分别建立索引或者建立联合索引， 其原理与lnnoDB中的索引差不多，只不过严叶子节点处存储的是相应的列+行号。这些索引也全部都是二级索引

> MylSAM 的行格式有定长记录格式( Static ) 、变长记录格式( Dynamic) 、压缩记录格式( Compressed )等。采用定长记录格式，也就是一条记录占用的存储空间是固定的， 这样就可以使用行号轻松算出某条记录在数据文件中的地址偏移量了。变长记录格式MyISAM 会直接;在索引叶子节点处存储该条记录在数据文件中的地址偏移量。由此可以看出， MyISAM 的回农操作是十分快速的，因为它是拿着地址偏移量直接到文件中取数据

> innoDB 中的“索引即数据，数据即索引"
>
> 在MyISAM 中是"索号|是索引，数据是数据'

### 6.2.5 MySQL 中创建和删除索引的语句

lnnoDB 和 MyISAM 会自动为主键或者带有UNION 属性的列建立索引

为其他的列建立索引， 就需要显式地指明

```mysql
CREATE TABLE 表名 (
	各个列的信息...,
	(KEY|INDEX) 索引名 (需要被索引的单个列或多个列)
)

ALTER TABLE 表名 ADD (KEY|INDEX) 索引名 (需要被索引的单个列或多个列);
```

> KEY 和INDEX 是同义词，任意选用一个就可以。

```mysql
ALTER TABLE 表名 DROP (INDEX|KEY) 索引名;
```

## 6.3 总结

+ InnoDB 存储引擎的索引是一棵B+ 树， 完整的用户记录都存储在B+ 树第0 层的叶子节点；其他层次的节点都属于存储目录项记录的内节点。
+ InnoDB 的索引分为两种。
  + 聚簇索引
  + 二级索引
+ InnoDB 存储引擎的B+ 树根节点自创建之日起就不再移动.
+ 在二级索引的B+ 树内节点中，目录项记录由索引列的值、**主键值**和页号组成.
+ 一个数据页至少可以容纳2 条记录.
+ MyISAM 存储引擎的数据和索引分开存储

------

[toc]