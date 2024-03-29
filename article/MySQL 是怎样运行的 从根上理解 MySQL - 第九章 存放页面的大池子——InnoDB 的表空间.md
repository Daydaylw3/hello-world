[toc]

------

## 9.1 回忆一些旧知识

### 9.1.1 页面类型

+ InnoDB 是以页为单位管理存储空间的
+ 聚簇索引和其他的二级索引都是以B+ 树的形式保存到表空间中
+ B+ 树的节点就是数据页，这个数据页的类型名其实是 FIL_PAGE_INDEX。
+ InnoDB 也针对不同的目的设计了若干种不同类型的页面

### 9.1.2 页面通用部分

所有类型的页面都有下图这种通用结构（38B 的 File Header 和 8B 的 File Trailer）

![image-20220512151115726](/Users/daydaylw3/Pictures/typora/image-20220512151115726.png)

+ **File Header**：记录页面的一些通用信息.
+ **File Trailer**：校验页是否民整，保证页面在从内存刷新到磁盘后内容是相同的

> 第五章

+ 表空间中的每一个页都对应着一个页号，也就是 `FIL_PAGE_OFFSET`，可以通过页号在表空间中快速定位到指定的页面。
+ 页号由4 字节组成，也就是32 位，所以一个表空间最多可以拥有 `2^32` 个页。按照页的默认大小为16KB来算，一个表空间最多支持64TB 的数据。表空间中第一个页的页号为0，之后的页号分别是 1 、2、3 …
+ 某些类型的页可以组成链表，链表中相邻的两个页面的页号可以不连续，根据 FIL_PAGE_PREV 和 FIL_PAGE_NEXT 来存储上一个页和下一个页的页。这两个字段主要是用于 INDEX 类型的页
+ 每个页的类型由 FIL_PAGE_TYPE 表示

## 9.2 独立表空间结构

### 9.2.1 区的概念

为了更好地管理页面，InnoDB 提出了区（extent）的概念。对于16KB 的页来说，**连续的 64 个页**就是一个区，也就是说**一个区默认占 1MB 空间大小**。

无论是系统表空间还是独立表空间，都可以看成是由若干个连续的区组成的，**连续256 个区被划分成一组**

<img src="/Users/daydaylw3/Pictures/typora/image-20220512152918849.png" alt="image-20220512152918849" style="zoom:67%;" />

这些组的头几个页面的类型都是类似的

<img src="/Users/daydaylw3/Pictures/typora/image-20220512153225327.png" alt="image-20220512153225327" style="zoom:80%;" />

+ 第一个组最开始的3 个页面的类型是固定的。也就是说 extent 0 这个区最开始的 3 个页面的类型是固定的

  + `FSP_HDR`：这个类型的页面用来登记**整个表空间的一些整体属性**以及本组所有的区（extent 0 ~ 255）的属性。

    > 需要注意的是，整个表空间只有一个 `FSP_HDR` 类型的页面

  + `IBUF_BITMAP`：这个类型的页面用来存储关于 `Change Buffer` 的一些信息

  + `INODE`：这个类型的页面存储了许多称为 `INODE Entry` 的数据结构

+ 其余各组最开始的 2 个页面（extent 256、extent 512 ... ）的类型是固定的。

  + `XDES`：全称是 `extent descriptor`，用来登记**本组 256 个区的属性**
  + `IBUF_BITMAP`：这个类型的页面用来存储关于 `Change Buffer` 的一些信息

> 表空间被划分为许多连续的区，每个区默认由 64 个页组成，每256 个区划分为一组，每个组的最开始的几个页面类型是固定的

### 9.2.2 段的概念

> 为啥好端端地提出一个区（extent）的概念呢？

从理论上说，不引入区的概念，而只使用页的概念对存储引擎的运行并没啥影响，但是我们来考虑下面这个场景

我们每向表中插入一条记录，本质上就是向该表的聚簇索引以及所有二级索引代表的B+树的节点中插入数据，而B+ 树每一层中的页都会形成一个双向链表，如果以页为单位来分配存储空间，双向链表相邻的两个页之间的物理位置可能离得非常远

前面提到使用B+ 树来减少记录的扫描行数的过程是通过一些搜索条件到B+ 树的叶子节点中定位到第一条符合该条件的记录（对于全表扫描来说就是定位到第一个叶子节点的第一条记录），然后沿着由记录组成的单向链表以及由数据页组成的双向链表一直向后扫描就可以了

如果双向链表中相邻的两个页的物理位置不连续，对于传统的机械硬盘来说，需要重新定位磁头位置，也就是会产生随机 `I/O` 这样会影响磁盘的性能。

所以我们应该**尽量让页面链表中相邻的页的物理位置也相邻**，这样在扫描叶子节点中大量的记录时才可以使用顺序 `I/O`

所以才引入了区(extent)的概念。一个区就是在物理位置上连续的 64 个页（区里页面的页号都是连续的）。在表中的数据量很大时，为某个索引分配空间的时候就不再按照页为单位分配了，而是按照区为单位进行分配。甚至在表中的数据非常非常多的时候，可以一次性分配多个连续的区

我们在使用B+ 树执行查询时只是在扫描叶子节点的记录，而*如果不区分叶子节点和非叶子节点*，统统把节点代表的页面放到申请到的区中，扫描效果就大打折扣了。

所以 InnoDB 对B+ 树的叶子节点和非叶子节点进行了区别对待，也就是说叶子节点有自己独有的区，非叶子节点也有自己独有的区

存放叶子节点的区的集合就算是一个段（segment），存放非叶子节点的区的集合也算是一个段（segment）

>  一个索引会生成两个段：一个叶子节点段和一个非叶子节点段

默认情况下，一个使用 lnnoOB 存储引擎的表只有一个聚簇索引个索引会生成两个段。而段是以区为单位申请存储空间的，一个区默认占用 1MB 存储空间。所以，默认情况下一个只存放了几条记录的小表也需要 2MB 的存储空间么？以后每次添加一个索引都要多申请2MB 的存储空间么？

这个问题的症结在于现在为止我们介绍的区都是非常纯粹的，也就是一个区被整个分配给某一个段，或者说区中的所有页面都是为了存储同一个段的数据而存在的。即使段的数据填不满区中所有的页面，剩下的页面也不能挪作他用。

现在为了考虑 “以完整的区为单位分配给某个段时，对于数据量较小的表来说太浪费存储空间” 这种情况，InnoOB 提出了碎片( fragment ) 区的概念。

在一个碎片区中， 并不是所有的页都是为了存储同一个段的数据而存在的，碎片区中的页可以用于不同的目的，比如有些页属于段A. 有些页属于段B. 有些页甚至不属于任何段。碎片区直属于表空间， 并不属于任何一个段

为某个段分配存储空间的策略是这样的：

+ 在刚开始向表中插入数据时，段是从某个碎片区以单个页面为单位来分配存储空间的
+ 当某个段已经占用了 32 个碎片区页面之后， 就会以完整的区为单位来分配存储空间（原先占用的碎片区页面并不会被复制到新申请的完整的区中）

> 段现在不能仅定义为某些区的集合，更精确的来说， 应该是某些零散的页面以及一些完整的区的集合
>
> 除了索引的叶子节点段和非叶子节点段之外，InnoOB 中还有为存储一些特殊的数据而定义的段，比如回滚段

### 9.2.3 区的分类

表空间是由若干个区组成的，这些区大致可以分为4 种类型，也可以称为区的4 种状态（State）

+ 空闲的区
+ 有剩余空闲页面的碎片区
+ 没有剩余空闲页面的碎片区
+ 附属于某个段的区：每一个索引都可以分为叶子节点段和非叶子节点段。除此之外. lnnoOB 还会另外定义一些特殊用途的段，当这些段中的数据量很大时，将使用区作为基本的分配单位，这些区中的页面完全用于存储该段中的数据（而碎片区可以存储属于不同段的数据）

| 状态名      | 含义                     |
| ----------- | ------------------------ |
| `FREE`      | 空闲的区                 |
| `FREE_FRAG` | 有剩余空闲页面的碎片区   |
| `FULL_FRAG` | 没有剩余空闲页面的碎片区 |
| `FSGE`      | 附属于某个段的区         |

> 处于 FREE、FREE FRAG 以及FULL FRAG 这3 种状态的区都是独立的，算是直属于表空间
>
> 处于FSEG 状态的区是附属于某个段的

> 如果把表空间比作一个集团军，段就相当于师，区就相当于团。一般来说，团都是隶属于某个师，就像是处于 FSEG 的区全都隶属于某个段；而处于FREE、FREE FRAG 以及 FULL_FRAG 这3 种状态的区却直接隶属于表空间，就像独立团直接听命于军部一样。

为了方便管理这些区，InnoDB 设计了一个称为 `XDES Entry（Extent Descriptor Entry）`的结构。每一个区都对应着一个 XDES Entry 结构，这个结构记录了对应的区的一些属性。

![image-20220512211122905](/Users/daydaylw3/Pictures/typora/image-20220512211122905.png)

XDES Entry 结构有 40 字节，大致分为 4 个部分

+ `Segment ID`：8 字节，每一个段都有一个唯一的编号；表示的就是该区所在的段，前提是该区已经被分配给某个段了，不然该字段的值没有意义
+ `List Node`：12 字节，这个部分可以将若干个 XDES Entry 结构串连成一个链表
  + `Pre Node Page Number` 和 `Pre Node Offset` 的组合就是指向前一个 XDES Entry 的指针
  + `Next Node Page Number` 和 `Next Node Offset` 的组合就是指向前一个 XDES Entry 的指针
+ `State`：4 字节
+ `Page State Bitmap`：16 字节，也就是128 位。一个区默认有 64 个页，这128 位被划分为64 个部分，每个部分有2 位，对应区中的一个页。这2 个位中的第 1 位表示对应的页是否是空闲的，第2 位还没有用到

#### 1. XDES Entry 链表

> 我们已经提出的概念五花八门——区、段、碎片区、附属于段的区、XDES Entry。初心仅仅是想减少随机 `I/O`，而又不至于让数据盘少的表浪费空间

捋一捋向某个段中插入数据时，申请新页面的过程

+ **当段中数据较少时**
  + 首先会查看表空间中是否有状态为 `FREE_FRAG` 的区（还有空闲页面的碎片区）
  + 找到了那么从该区中取一个零散页把数据插进去
  + 否则到表空间中申请一个状态为 `FREE` 的区（空闲的区），把该区的状态变为`FREE_FRAG` ，然后从该新申请的区中取一个零散页把数据插进去
  + 之后，在不同的段使用零散页的时候都从该区中取，直到该区中没有空闲页面；然后该区的状态变成 `FULL_FRAG`

> 现在的问题是我们怎么知道表空间中哪些区的状态是FREE，哪些区的状态是FREE FRAG 呢？表空间是可以不断增大的，当增长到GB 级别的时候，区的数量也就上千了，总不能每次都遍历这些区对应的XDES Entry 结构吧？

我们可以通过 List Node 中的指针做下面3 件事

+ 通过Li st Node 把状态为FREE 的区对应的XDES En try 结构连接成一个链表，这个链表称为FREE 链表.
+ 通过List Node 把状态为FREE FRAG 的区对应的XDES Entry 结构连接成一个链衰，这个链表称为FREE FRAG 链衰。
+ 通过List Node 把状态为FULLJRAG 的区对应的XDES Entry 结构连续成一个链表，这个链表称为FULL FRAG 链表.

想查找一个FREE FRAG 状态的区时，就直接把FREE FRAG 链表的头节点拿出来，从这个节点对应的区中取一些零散页来插入数据。

当这个节点对应的区中没有空闲的页面时， 就修改它的State 字段的值， 然后将其从FREE FRAG 链表中移到 FULL FRAG 链表中

同理， 如果FREE FRAG 链表中一个节点都没有， 那么就直接从FREE 链表中取一个节点移动到FREE FRAG 链衰， 并修改该节点的STATE 字段值为FREE FRAG

当段中的数据已经占满了32 个零散的页后， 就直接申请完整的区来插入数据了

> 我们怎么知道哪些区属于哪个段呢？

可以基于链表来快速查找只属于某个段的区。

InnoDB 为每个段中的区对应的XDES Entry 结构建立了3 个链表

+ FREE 链表：同一个段中， 所有页面都是空闲页面的区对应的XDES Entry 结构会被加入到这个链表中

  > 注意， 这与直属于表空间的FREE 链袭区别开了，此处的FREE 链表是附属于某个段的链表

+ NOT_FULL 链表：同一个段中， 仍有空闲页面的区对应的XDES Entry 结构会被加入到这个链表中

+ FULL 链表：同一个段中，己经没有空闲页面的区对应的 XDES Entry 结构会被加入到这个链表中

> 再强调一遍， 每一个索引都对应两个段， 每个段都会维护上述3 个链表

```mysql
mysql> create table t (
    -> c1 int not null auto_increment,
    -> c2 varchar(100),
    -> c3 varchar(100),
    -> primary key (c1),
    -> key idx_c2 (c2)
    -> ) engine=InnoDB;
```

> 表t 共有两个索引: 一个聚簇索引和一个二级索引idx_c2. 所以这个表共有4 个段，每个段都会维护上述3 个链衰，总共是12 个链表。再加上前文说过的直属于表空间的3 个链衰，整个独立表空间共需要维护15 个链表

#### 2. 链表基节点

> 怎么找到这些链表呢， 或者说怎么找到某个链表的头节点或者尾节点在表空间中的位置呢？

InnoDB 设计了个名为List Base Node ( 链表基节点〉的结构。这个结构中包含了链表的头节点和尾节点的指针以及这个链表中包含了多少个节点的信息

<img src="/Users/daydaylw3/Pictures/typora/image-20220512215748820.png" alt="image-20220512215748820" style="zoom:67%;" />

+ `List Length`：表明该链表→共有多少个节点
+ `First Node Page Number` 和 `First Node Offset`：表明该链表的头节点在表空间中的位置
+ `Last Node Page Number` 和 `Last Node Offset`：表明该链袤的尾节点在表空间中的位置

#### 3. 链表小结

+ 表空间是由若干个区组成的
+ 每个区都对应一个XDES Entry 结构
+ 直属于表空间的区对应的XDES Entry 绪构可以分成FREE、FREE_FRAG 和FULL_FRAG 这3 个链表
+ 每个段可以拥有若干个区，每个段中的区对应的XDES Entry 结构可以构成FREE、NOT_FULL 和 FULL 这 3 个链表
  + 每个链表都对应一个List Base Node 结构，这个结构中记录了链表的头尾节点的位置以及该链表中包含的节点数
+ 正是因为这些链袤的存在，管理这些区才变成了一件相当容易的事情

### 9.2.4 段的结构

> 段其实不对应表空间中某-个连续的物理区域，而是一个逻镇上的概念，由若干个零散的页面以及一些完整的区组成

InnoDB 为**每个段**都定义了一个的 INODE Entry 结构来记录这个段中的属性

<img src="/Users/daydaylw3/Pictures/typora/image-20220512220726087.png" alt="image-20220512220726087" style="zoom:80%;" />

+ `Segment ID`：这个 INODE Entry 结构对应的段的编号( lD）
+ `NOT_FULL_N_USED`：在NOT_FULL 链表中已经使用了多少个页面
+ 3 个 `List Base Node`：分别为段的FREE 链表、NOT_FULL 链表、FULL 链表定义了 List Base Node，当想查找某个段的某个链袤的头节点和尾节点时，可以直接到这个部分找到对应链表的 List Base Node
+ `Magic Number`：用来标记这个INODE Entry 是否已经被初始化（即把各个字段的值都填迸去了）。如果这个数字的值是 97,937,874 ， 表明该INODE Entry 已经初始化，否则没有
+ `Fragment Array Entry`：段是一些零散页面和一些完整的区的集合，每个 Fragment Array Entry 结构都对应着一个零散的页面，这个结构一共4 字节，表示一个零散页面的页号

### 9.2.5 各类型页面详细情况

> 表空间、段、区、XDES Entry、INODE Entry、各种以XDES Entry 为节点的链袤的基本概念

#### 1. FSP_HDR 类型

> 第一个组的第一个页面，也是表空间的第一个页面，页号为0，这个页面的类型是FSP_DIR

存储了表空间的一些整体属性以及第一个组内2 56 个区对应的 XDES Entry 结构

![image-20220512222712517](/Users/daydaylw3/Pictures/typora/image-20220512222712517.png)

| 名称                | 中文名       | 占用空间大小（字节 | 简单描述                       |
| ------------------- | ------------ | ------------------ | ------------------------------ |
| `File Header`       | 文件头部     | 38                 | 页的一些通用信息               |
| `File Space Header` | 表空间头部   | 112                | 表空间的一些整体属性信息       |
| `XDES Entry`        | 区描述信息   | 10240              | 存储本组256 个区对应的属性信息 |
| `Empty Space`       | 尚未使用空间 | 5986               | 用于页结构的填充，没啥实际意义 |
| `File Trailer`      | 文件尾部     | 8                  | 校验页是否完整                 |

##### （1）File Space Header 部分

 用来存储表空间的一些整体属性

![image-20220512223629486](/Users/daydaylw3/Pictures/typora/image-20220512223629486.png)

| 名称                                      | 占用空间大小（字节 | 描述                                                         |
| ----------------------------------------- | ------------------ | ------------------------------------------------------------ |
| `Space ID`                                | 4                  | 表空间的lD                                                   |
| `Not Used`                                | 4                  | 未被使用， 可以忽略                                          |
| `Size`                                    | 4                  | 当前表空间拥有的页面数                                       |
| `FREE Limit`                              | 4                  | 尚未被初始化的最小页号， 大于或等于这个页号的区对应的XDES Entry 结构都没有被加入FREE 链表 |
| `Space Flags`                             | 4                  | 表空间的一些占用存储空间比较小的属性                         |
| `FRAG_N_USED`                             | 4                  | FREE FRAG 链表中已使用的页面数量                             |
| `List Base Node for FREE List`            | 16                 | FREE 链衰的基节点                                            |
| `List Base Node for FREE_FRAG List`       | 16                 | FREE FRAG 链袤的基节点                                       |
| `List Base Node for FULL_FRAG List`       | 16                 | FULL FRAG 链袤的基节点                                       |
| `Next Unused Segment ID`                  | 8                  | 当前表空间中下一个未使用的 Segment ID                        |
| `List Base Node for SEG_INODES_FULL List` | 16                 | SEG lNODES FULL 链表的基节点                                 |
| `List Base Node for SEG_INODES_FREE List` | 16                 | SEG lNODES FREE 链表的基节点                                 |

+ `List Base Node for FREE List`、`List Base Node for FREE_FRAG List`、`List Base Node for FULL_FRAG List`：

  分别是直属于表空间的FREE 链表的基节点、FREE]RAG 链表的基节点、FULL]RAG 链衰的基节点。这3 个链袤的基节点在表空间的位置是固定的， 就是在表空间的第一个页面（也就是 FSP_HDR 类型的页面）的 File Space Header 部分

+ `FRAG_N_USED`：表明在FREE FRAG 链表中已经使用的页面数量。

+ `FREE Limit`：表空间对应着具体的磁盘文件。表空间在最初创建时会有一个默认的大小。而且磁盘文件一般都是自增长文件，也就是当该文件不够用时，会自动增大文件大小。这就带来了下面这两个问题

  + 最初创建表空间时， 可以指定一个非常大的磁盘文件；接着需要对表空间完成一个初始化操作，包括为表空间中的区建立对应的XDES Entry 结构、为各个段建立 INODE Entry 结构、建立各种链表等在内的各种操作。但是对于非常大的磁盘文件来说，实际上有绝大部分的空间都是空闲的。我们可以选择把所有的空闲区对应的XDES Entry 结构加入到FREE 链衰，也可以选择只把一部分空闲区加入到FREE 链表， 等空闲链表中的 XDES Entry 结构对应的区不够使的时候，再把之前没有加入FREE 链表的空闲区对应的 XDES Entry 结构加入到FREE 链表

  + 对于自增长的文件来说，可能在发生一次自增长时分配的磁盘空间非常大。同样，我们可以选择把新分配的这些磁盘空间代表的空闲区对应的XDES Entry 结构加入到FREE 链表；也可以选择只把一部分空闲区加入到FREE 链表，等空闲链表中的XDES Entry 结构对应的区不够使的时候，再把之前没有加入FREE 链表的空闲区对应的 XDES Entry 结构加入到FREE 链表。

    InnoDB 采用的都是后者， 中心思想就是等用的时候再把它们加入到FREE 链表。他们为表空间定义了FREE Limit 字段，在该字段表示的页号之后的区都未被使用，而且尚未被加入到FREE 链表

+ `Next Unused Segment ID`：表中每个索引都对应两个段，每个段都有一个唯一的lD。当我们为某个表新创建一个索引的时候， 意味着需要创建两个新的段.那么， 怎么为这个新创建的段分配一个唯一的 ID 呢？所以 InnoDB 提出了这个名为 Next Unused Segment lD 的字段，该字段表明当前表空间中最大的段 lD 的下一个 ID。这样在创建新段时为其赋予一个唯一的 ID 值就相当容易了——直接使用这个字段的值，然后把该字段的值递增一下就好了

+ `Space Flags`：表空间中与一些布尔类型相关的属性， 或者只需要寥寥几个比特搞定的属性，都存放在这个Space Flags 中.虽然这个字段只有4 字节02 比特) . 却储了表空间的好多属性

  | 标志名称        | 占用空间大小（比特 | 描述                                               |
  | --------------- | ------------------ | -------------------------------------------------- |
  | `POST_ANTELOPE` | 1                  | 表示文件格式是否在 ANTELOPE 格式之后               |
  | `ZIP_SSIZE`     | 4                  | 表示压缩页面的大小                                 |
  | `ATOMIC_BLOBS`  | 1                  | 表示是否自动把占用存储空间非常多的字段放到溢出页中 |
  | `PAGE_SSIZE`    | 4                  | 页面大小                                           |
  | `DATA_DIR`      | 1                  | 表示表空间是否是从数据目录中获取的                 |
  | `SHARED`        | 1                  | 是否为共享褒空间                                   |
  | `TEMPORARY`     | 1                  | 是否为临时表空间                                   |
  | `ENCRYPTION`    | 1                  | 表空间是否加密                                     |
  | `UNUSED`        | 18                 | 没有使用到的比特                                   |

+ `List Base Node for SEG_INODES_FULL List` 和 `List Base Node for SEG_INODES_FULL List`：

  每个段对应的 INODE Entry 结构会集中存放到一个类型为的 INODE 的页中。如果表空间中的段特别多，则会有多个的 NODE Entry 结构，此时可能一个页放不下， 就需要多个的 INODE 类型的页丽.这些 INODE 类型的页会构成下面两种链表

  + `SEG_INODES_FULL` 链表：在该链表中，INODE  类型的页面都已经被的 INODE Entry 结构填充满，没有空闲空间存放额外的INODE Entry 
  + `SEG_INODES_FREE` 链表：在该链表中，INODE 类型的页面仍有空闲空间来存放 INODE Entry 结构

##### （2）XDES Entry 部分

> 存储在表空间的第一个页面中，一个XDES Entry 结构的大小是40 字节

由于一个页面的大小有限， 只能存放数量有限的 XDES Entry 结构， 所以我们才把256 个区划分成一组， 在每组的第一个页面中存放256 个XDES Entry 结构；其中 XDES Entry 0 对应着 extent 0，XDES Entry 1 对应着 extent 1 ... XDES Entry255 对应着 extent 255

因为每个区对应的 XDES Entry 结构的地址是固定的， 因此我们可以很轻松地访问 extent n 对应的 XDES Entry 结构

#### 2. XDES 类型

每一个XDES Eniry 结构对应表空间的一个区

一个XDES Eniry 结构只占用 40 字节，在区的数量非常多时，一个单独的页可能无法存放足够多的XDES Eniry 结构

所以我们把表空间的区分为若干个组， 每组开头的一个页面记录着本组内所有的区对应的XDES Eniry 结构

由于第一个组的第一个页面有些特殊，所以除了记录本组中所有区对应的XDES Eniry 结构外， 还记录着表空间的一些整体属性，这个页面的类型就是FSP HDR 类型

> 整个表空间里只有一个这种类型的页面

除第一个分组以外，之后每个分组的第一个页面只需要记录本组内所有的区对应的 XDES Entry结构即可， 不需要再记录表空间的属性

把之后每个分组中第一个页面的类型定义为XDES， 它的结构与FSP HDR 类型是非常相似的

![image-20220513153148587](/Users/daydaylw3/Pictures/typora/image-20220513153148587.png)

> XDES 类型的页面除了没有 File Space Header 部分之外，其余的部分是一样的

#### 3. IBUF_BITMAP 类型

> 每个分组中第二个页面的类型都是 `IBUF_BITMAP` 

这种类型的页中记录了一些有关 Change Buffer 的东西

> 向表中插入一条记录，其实本质上是向每个索引对应的B+ 树中插入记录。该记录首先插入聚簇索引页面，然后再插入每个二级索引页面。

这些页面在表空间中随机分布，将会产生大量的随机 `I/O`，严重影响性能。对于 UPDATE 和DELETE 操作来说，也会带来许多的随机 `I/O `

InnoDB 引入了一种称为 Change Buffer 的结构（本质上也是表空间中的一颗B+ 树，它的根节点存储在系统表空间中）

在修改非唯一二级索引页面时，如果该页面尚未被加载到内存中，那么该修改将先被暂时缓存到 Change Buffer 中，之后服务器空闲或者其他什么原因导致对应的页面从磁盘上加载到内存中时，再将修改合并到对应页面。

> 在很久之前的版本中只会缓存 INSERT 操作对二级索引页面所做的修改，所以 Change Buffer 以前被称作 Insert Buffer，所以在各种命名上延续了之前的叫法，比如 IBUF 其实是 Insert Buffer 的缩写

#### 4. INODE 类型

>  第一个分组中第三个页面的类型是 INODE

InnoDB 为每个索引定义了两个段，而且为某些特殊功能定义了特殊的段。为了方便管理，他们又为每个段设计了一个 INODE Entry 结构，这个结构记录了这个段的相关属性

INODE 类型的页就是为了存储 INODE Entry 结构

![image-20220513155000762](/Users/daydaylw3/Pictures/typora/image-20220513155000762.png)

| 名称                            | 中文名       | 占用空间大小（字节 | 简单描述                                       |
| ------------------------------- | ------------ | ------------------ | ---------------------------------------------- |
| `File Header`                   | 文件头部     | 38                 | 页的一些通用信息                               |
| `List Node for INODE Page List` | 通用链表节点 | 12                 | 存储上一个 INODE 页面和下一个 INODE 页面的指针 |
| `INODE Entry`                   | 段描述信息   | 16320              | 具体的 INODE Entry 结构                        |
| `Empty Space`                   | 尚未使用空间 | 6                  | 用于页结构的填充，没啥实际意义                 |
| `File Trailer`                  | 文件尾部     | 8                  | 校验页是否完整                                 |

**List Node for INODE Page List**

如果一个表空间中存在的段超过85 个，那么一个 INODE 类型的页面不足以存储所有的段对应的 INODE Entry 结构，所以就需要额外的INODE 类型的页面来存储这些结构

为了方便管理这些 INODE 类型的页面，InnoDB 将这些 INODE 类型的页面串连成两个不同的链表

+ `SEG_INODES_FULL` 链表：在该链表中， INODE 类型的页面中己经没有空闲空间来存储额外的 INODE Entry 结构
+ `SEG_INODE_FREE` 链表：在该链表中，INODE 类型的页面中还有空闲空间来存储额外的 INODE Entry 结构

> 这两个链表的基节点就存储在 FSP_HDR 类型页面的 File Space Header 中，所以这两个链袤的基节点的位置是固定的，从而可以轻松访问这两个链表

以后每当新创建一个段（创建索引时就会创建段）时，都会创建一个与之对应的 INODE Entry 结构。存储 INODE Entry 的过程

+ 查看 `SEG_INODE_FREE` 链表是否为空。
  + 不为空，直接从该链表中获取一个节点，相当于获取到一个仍有空闲空间的 INODE 类型的页面，然后把该 INODE Entry 结构放到该页面中，当该页面中无剩余空间时，就把该页放到 `SEG_INODES_FULL` 链表中
  + 为空，需要从表空间的 `FREE_FRAG` 链表中申请一个页面，并将该页丽的类型修改为 INODE ，把该页面放到 `SEG_INODE_FREE` 链表中；同时把该 INODE Entry 结构放入该页面

### 9.2.6 Segment Header 结构的运用

> 一个索引会产生两个段，分别是叶子节点段和非叶子节点段，而每个段都会对应一个 INODE Entry 结构

怎么知道某个段对应哪个 INODE Entry 结构呢？

在数据页（也就是索引页）有一个 Page Header 部分

| 名称                | 占用空间大小（字节 | 描述                                               |
| ------------------- | ------------------ | -------------------------------------------------- |
| `PAGE_BTR_SET_LEAF` | 10                 | B+ 树叶子节点段的头部信息，仅在B+ 树的根页中定义   |
| `PAGE_BTR_SEG_TOP`  | 10                 | B+ 树非叶子节点段的头部信息，仅在B+ 树的根页中定义 |

PAGE_BTR_SET_LEAF 和 PAGE_BTR_SEG_TOP 都占用 10 字节，他们对应一个名为 Segment Header 的结构

<img src="/Users/daydaylw3/Pictures/typora/image-20220513161127274.png" alt="image-20220513161127274" style="zoom:80%;" />

| 名称                             | 占用空间大小（字节 | 描述                               |
| -------------------------------- | ------------------ | ---------------------------------- |
| `Space ID of the INODE Entry`    | 4                  | INODE Entry 结构所在的表空间 ID    |
| `Page Number of the INODE Entry` | 4                  | INODE Entry 结构所在的页面页号     |
| `Byte Offset of the INODE Entry` | 2                  | INODE Entry 结构在该页面中的偏移量 |

PAGE_BTR_SET_LEAF 记录着叶子节点段对应的 INODE Entry 结构的地址是哪个表空间中哪个页面的哪个偏移量

PAGE_BTR_SEG_TOP 记录着非叶子节点段对应的 INODE Entry 结构的地址是哪个表空间中哪个页面的哪个偏移量

索引和对应的段的关系就建立起来了

> 因为一个索引只对应两个段，所以只需要在索引的根页面中记录这两个结构即可

### 9.2.7 真实表空间对应的文件大小

表空间文件是自扩展的，随着表中数据的增多，表空间对应的文件也逐渐增大

## 9.3 系统表空间

系统表空间的结构与独立表空间基本类似

系统表空间中需要记录一些与整个系统相关的信息，所以会比独立表空间多出一些用来记录这些信息的页面

因为这个系统表空间最重要，所以它的表空间 ID（Space ID）是 0

### 9.3.1 系统表空间的整体结构

与独立表空间相比，系统表空间有一个非常明显的不同之处，就是在表空间开头有许多记录整个系统属性的页面

![image-20220513161801182](/Users/daydaylw3/Pictures/typora/image-20220513161801182.png)

系统表空间页号为 3 - 7 的页面是系统表空间特有的

| 页号 | 页面类型  | 英文描述                 | 描述                         |
| ---- | --------- | ------------------------ | ---------------------------- |
| 3    | `SYS`     | `Insert Buffer Header`   | 存储Change Buffer 的头部信息 |
| 4    | `INDEX`   | `Insert Buffer Root`     | 存储Change Buffer 的根页面   |
| 5    | `TRX_SYS` | `Transction System`      | 事务系统的相关信息           |
| 6    | `SYS`     | `First Rollback Segment` | 第一个回滚段的信息           |
| 7    | `SYS`     | `Data Dictionary Header` | 数据字典头部信息             |

>  除了这几个记录系统属性的页面之外， 系统表空间的extent 1 和extent 2 这两个区，也就是页号从 64-1 91 的这128 个页面称为Doublewrite Buffer (双写缓冲区)

#### InnoDB 数据字典

平时使用 INSERT 语句向表中插入的那些记录称为用户数据

MySQL 只是作为一个软件来为我们来保管这些数据， 提供方便的增删改查接口而己

但是每当向一个表中插入一条记录时， MySQL 先要校验插入语句所对应的表是否存在， 以及插入的列和表中的列是否符合。如果语法没有问题， 还需要知道该表的聚簇索引和所有二级索引对应的根页面是哪个表空间的哪个页面， 然后把记录插入对应索引的B+ 树中。

所以，MySQL 除了保存着插入的用户数据之外，还需要保存许多额外的信息

+ 某个表属于哪个表空间， 表里面有多少列
+ 表对应的每一个列的类型是什么
+ 该表有多少个索引，每个索引对应哪几个字段，该索引对应的根页面在哪个表空间的哪个页面
+ 该表有哪些外键，外键对应哪个表的哪些列，
+ 某个表空间对应的文件系统上的文件路径是什么

上述信息是为了更好地管理用户数据而不得己引入的一些额外数据，这些数据也称为**元数据**

InnoDB 存储引擎定义了一系列的内部系统表（internal system table）来记录这些**元数据**

| 表名               | 描述                                                        |
| ------------------ | ----------------------------------------------------------- |
| `SYS_TABLES`       | 整个 lnnoOB 存储引擎中所有袤的信息                          |
| `SYS_COLUMNS`      | 整个 lnnoOB 存储引擎中所有列的信息                          |
| `SYS_INDEXES`      | 整个lnnoqB 存储引擎中所有索引的信息                         |
| `SYS_FIELDS`       | 整个InnoOB 存储引擎中所有索引对应的列的信息                 |
| `SYS_FOREIGN`      | 整个 InnoOB 存储引擎中所有外键的信息                        |
| `SYS_FOREIGN_COLS` | 整个InnoDB 存储引擎中所有外键对应的列的信息                 |
| `SYS_TABLESPACES`  | 整个InnoDB 存储引擎中所有的表空间倍息                       |
| `SYS_DATAFILES`    | 整个InnoDB 存储引擎中所有表空间对应的文件系统的文件路径信息 |
| `SYS_VIRTUAL`      | 整个InnoDB 存储引擎中所有虚拟生成的列的信息                 |

这些系统表也被为数据字典，它们都是以B+ 树的形式保存在系统表空间的某些页面中。

其中SYS_TABLES 、SYS_COLUMNS 、SYS_INDEXES 、SYS_FIELDS 这4 个表尤其重要，称为基本系统表（basic system table) 

##### （1）SYS_TABLES 表

SYS TABLES 表的列

| 列名         | 描述                                             |
| ------------ | ------------------------------------------------ |
| `NAME`       | 袤的名称                                         |
| `ID`         | 在InnoDB 存储引擎中，每个表都有的一个唯一的 ID   |
| `N_COLS`     | 该表拥有列的个数                                 |
| `TYPE`       | 袤的类型，记录了一些文件格式、行格式、压缩等信息 |
| `MIX_ID`     | 己过时，忽略                                     |
| `MIX_LEN`    | 表的一些额外属性                                 |
| `CLUSTER_ID` | 未使用，忽略                                     |
| `SPACE`      | 该表所属表空间的m                                |

SYS_TABLES 表有两个索引

+ 以NAME 列为主键的聚簇索引
+ 以 ID 列建立的二级索引

##### （2）SYS_COLUMNS 表

| 列名       | 描述                                                         |
| ---------- | ------------------------------------------------------------ |
| `TABLE_ID` | 该列所属表对应的 ID                                          |
| `POS`      | 该列在表中是第几列                                           |
| `NAME`     | 该列的名称                                                   |
| `MTYPE`    | 主数据类型(main data type) .                                 |
| `PRTYPE`   | 精确数据类型( precise type），修饰主数据类型，比如是否允许 NULL 值，是否允许负数 |
| `LEN`      | 该列最多占用存储空间的字节数                                 |
| `PREC`     | 该列的精度〈不过这列貌似都没有使用) .默认值都是0             |

SYS COLUMNS 表只有一个聚簇索引，即以( TABLE_ID，POS ) 列为主键的聚簇索引

##### （3）SYS_INDEXES 表

| 列名              | 描述                                                         |
| ----------------- | ------------------------------------------------------------ |
| `TABLE_ID`        | 该索引所属表对应的m                                          |
| `ID`              | 在InnoDB 存储引擎中，每个表都有的一个唯一的 ID               |
| `NAME`            | 该索引的名称                                                 |
| `N_FIELDS`        | 该索引包律的列的个数                                         |
| `TYPE`            | 该索引创类型，比如聚穰索引、唯一二级索引、更改缓冲区的索引、全文索引、普通的二级索引 |
| `SPACE`           | 该索引根严面所在的表空间ID                                   |
| `PAGE_NO`         | 该索引根页面所在的页丽号                                     |
| `MERGE_THRESHOLD` | 如果就页面中的记录被删除到某个比例， 就尝试把该页面和相邻页面合并；这个值个就是这个比例 |

SYS_INDEXES 表只有一个聚簇索引， 即以（TABLE_ID，ID ) 列为主键的聚簇索引

##### （4）SYS_FIELDS 表

| 列名       | 描述                   |
| ---------- | ---------------------- |
| `INDEX_ID` | 该列所属索引的 ID      |
| `POS`      | 该列在索引列中是第几列 |
| `COL_NAME` | 该列的名称             |

##### （5）Data Dictionary Header 页面

只要有了上述4 个基本系统表，也就意味着可以获取其他系统表以及用户定义的表的所有元数据。比如，我们想看一下SYS~TABLESPACES 系统表中存储了哪些表空间以及表空间对应的属性，就可以执行下述操作

+ 根据表名到SYS一TABLES 表中定位到具体的记录，从而获取到SYS TABLESPACES 表的TABLE_ID
+ 使用获取的 TABLE_ID 到 SYS_COLUMNS 表中就可以获取到属于该袤的所有列的信息
+ 使用获取的 TABLE_ID 还可以到 SYS_INDEXES 表中获取所有的索引的信息。索引的信息中包括对应的INDEX ID，还记录着该索引对应的B+ 树根页面是哪个表空间的哪个页面
+ 使用获取的 INDEX_ID 就可以到SYS FIELDS 表中获取所有索引列的信息

InnoDB 又拿出一个固定的页面来记录这4 个袤的聚簇索引和二级索引对应的B+ 树位置。这个页面就是页号为7 的页面， 类型为SYS， 记录了Data Dictionary Header （数据字典的头部信息）。

除了这4 个表的5 个索引的根页面信息外，这个页号为7 的页面还记录了整个 InnoDB 存储引擎的一些全局属性

![image-20220513194020333](/Users/daydaylw3/Pictures/typora/image-20220513194020333.png)

| 名称                     | 占用空间大小（字节 | 简单描述                                                     |
| ------------------------ | ------------------ | ------------------------------------------------------------ |
| `File Header`            | 38                 | 页的一些通用信息                                             |
| `Data Dictionary Header` | 52                 | 记录一些基本系统表的根页面位置以及 lnnoDB 存储引擎的一些全局信息 |
| `Unused`                 | 4                  | 未使用                                                       |
| `Segment Header`         | 10                 | 记录本页面所在段对应的 INODE Entry 位置信息                  |
| `Empty Space`            | 16272              | 用于页结构的填充，没啥实际意义                               |
| `File Trailer`           | 8                  | 校验页是否完整                                               |

这个页面中竟然有Se伊entH?der 部分， 这意味着设计InnoDB 的大叔把这些有关数据字典的信息当成一个段来分配空间，我们就姑且叫数据字典段。由于目前要记录的数据字典信息非常少（可以看到 Data Dictionary Header 部分仅占用了 52 字节），所以该段只有一个碎片页，也就是页号为 7 的这个页

**Data Dictionary Header**

+ **Max Row ID**：中果不显式地为表定义主键，而且表中也没有不允许存储 NULL 值的 UNIQUE 键，那么 InnoDB 会默认生成一个名为 row_id 的列作为主键。因为它是主键，所以每条记录的 row_id 列的值不能重复。原则上只要一个表中的 row_id 列不重复就好了，不过 InnoDB 只提供了这个 Max Row ID 字段，无论哪个拥有 row_id 列的表插入一条记录，该记录的 row_id 列的值就是 Max Row ID 对应的值，然后再把 Max Row ID 对应的值加 1.也就是说 Max Row ID 是全局共享的

  > 并不是每分配一个 row_id 值都会将 Max Row ID 列刷新到磁盘一次

+ **Max Table ID**：在InnoDB 存储号|擎中，所有的表都对应一个唯一的ID，每次新建一个表时， 就会把本字段的值的值加 1，然后将其作为该表的 ID

+ **Max Index ID**：在InnoDB 存储号|擎中，所有的索引都对应一个唯一的ID，每次新建一个索引时， 就会把本字段的值的值加 1，然后将其作为该索引的 ID

+ **Max Space ID**：在InnoDB 存储号|擎中，所有的表空间都对应一个唯一的ID，每次新建一个表空间时， 就会把本字段的值的值加 1，然后将其作为该表空间的 ID

+ **Mix ID Low(Unused)**：这字段没啥用

+ **Root of SYS_TABLES clust index**：表示 SYS_TABLES 表聚簇索引的根页面的页号

+ **Root of SYS_TABLE_IDS sec index**：表示SYS_TABLE_IDS 表为 ID 列建立的二级索引的根页面的页号

+ **Root of SYS_COLUMNS clust index**：表示SYS_COLUMNS 表聚簇索引的根页面的页号

+ **Root of SYS_INDEXES clust index**：表示SYS_INDEXES 表聚簇索引的根页面的页号

+ **Root of SYS_FIELDS clust index**：表示 SYS_FIELDS 表聚簇索引的根页面的页号

##### （6）information_schema 系统数据库

用户不直接访问InnoDB 的这些内部系统衰， 除非直接去解析系统表空间对应的文件系统上的文件。

不过 InnoDB 考虑到，查看这写表的内容可能有助于分析问题，所以在系统数据库 information_schema 中提供了一些以 `INNODB_SYS` 开头的表（MySQL 8 是 `INNODB` 开头）

```mysql
mysql> show tables like 'INNODB_%';
+-----------------------------------------+
| Tables_in_information_schema (INNODB_%) |
+-----------------------------------------+
| INNODB_BUFFER_PAGE                      |
| INNODB_BUFFER_PAGE_LRU                  |
| INNODB_BUFFER_POOL_STATS                |
| INNODB_CACHED_INDEXES                   |
| INNODB_CMP                              |
| INNODB_CMP_PER_INDEX                    |
| INNODB_CMP_PER_INDEX_RESET              |
| INNODB_CMP_RESET                        |
| INNODB_CMPMEM                           |
| INNODB_CMPMEM_RESET                     |
| INNODB_COLUMNS                          |
| INNODB_DATAFILES                        |
| INNODB_FIELDS                           |
| INNODB_FOREIGN                          |
| INNODB_FOREIGN_COLS                     |
| INNODB_FT_BEING_DELETED                 |
| INNODB_FT_CONFIG                        |
| INNODB_FT_DEFAULT_STOPWORD              |
| INNODB_FT_DELETED                       |
| INNODB_FT_INDEX_CACHE                   |
| INNODB_FT_INDEX_TABLE                   |
| INNODB_INDEXES                          |
| INNODB_METRICS                          |
| INNODB_SESSION_TEMP_TABLESPACES         |
| INNODB_TABLES                           |
| INNODB_TABLESPACES                      |
| INNODB_TABLESPACES_BRIEF                |
| INNODB_TABLESTATS                       |
| INNODB_TEMP_TABLE_INFO                  |
| INNODB_TRX                              |
| INNODB_VIRTUAL                          |
+-----------------------------------------+
31 rows in set (0.01 sec)
```

> 在information schema 数据库中， 这些以 INNODB_SYS 开头的表并不是真正的内部系统表，而是在存储引擎启动时读取系统表，然后填充到这些以 INNODB_SYS 开头的表中；这些表的字段和系统表不完全一样

## 9.4 总结

+ InnoDB 出于不同的目的而设计了不同类型的页面，这些不同类型的页面基本都有 `File Header` 和 `File Trailer` 的通用结构
+ 表空间被划分为许多连续的区，对于大小为 16KB 的页面来说，每个区默认由 64 个页组成（1MB），每 256 个区划分为一组（256MB），每个组最开始的几个页面类型是固定的
+ 段是一个逻辑上的概念，是某些零散的页面以及一些完整的区的集合
+ **每个区都对应一个 XDES Entry 结构**，这个结构中存储了一些与这个区有关的属性。这些区可以被划分为几种类型
  + 空闲的区：会被加入到 FREE 链表
  + 有空闲页面的碎片区：会被加入到 FREE_FRAG 链表
  + 没有剩余页面的碎片区：会被加入到 FULL_FRAG 链表
  + 附属于某个段的区：每个段所属的区又会被组织成下面几种链表
    + FREE 链表：在同一个段中，所有页面都是空闲页面的区对应的 XDES Entry 结构会被加入到这个链表。
    + NOT_FULL 链表：在同一个段中，仍有空闲页面的区对应的 XDES Entry 结构会被加入到这个链表
    + FULL 链表：在同一个段中，已经没有空闲页面的区对应的 XDES Entry 结构会被加入到这个链表。
+ **每个段都会对应一个 INODE Entry 结构**，该结构中存储了一些与这个段有关的属性
+ 表空间中第一个页面的类型为 FSP_HDR，它存储了表空间的一些整体属性以及第一个组内 256 个区对应的 XDES Entry 结构。
+ 除了表空间的第一个组以外，其余组的第一个页面的类型为 XDES ，这种页面的结构和 FSP_HDR 类型的页面对比，除了少了File Space Header 部分之外，其余部分是一样的
+ 每个组的第二个页面的类型为 IBUF BITMAP ，存储了一些关于Change Buffer 的信息
+ 表空间中第一个分组的第三个页面的类型是 INODE ，它是为了存储 INODE Entry 结构而设计的，这种类型的页面会组织成下面两个链表
  + SEG_INODES_FULL 链表：在该链表中，INODE 类型的页面中己经没有空闲空间来存储额外的 INODE Entry 结构
  + SEG_INODES_FREE 链表：在该链表中，INODE 类型的页面中还有空闲空间来存储额外的 INODE Entry 结构
+ Segment Header 结构占用10 字节，是为了定位到具体的 INODE Entry 结构而设计的
+ 系统表空在表空间开头有许多记录整个系统属性的页面
+ InnoDB 提供了一系列系统表来描述元数据，其中 SYS_TABLES、SYS_COLUMNS、SYS_INDEXES、SYS_FIELDS 这 4 个表尤其重要，称为基本系统表（basic system table）。
+ 系统表空间的第7 个页面记录了数据字典的头部信息

------

[toc]