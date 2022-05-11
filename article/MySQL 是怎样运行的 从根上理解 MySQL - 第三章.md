**目录**

------

[toc]

------

## 3.1 字符集和比较规则简介

### 3.1.1 字符集简介

> 计算机中实际存储的是二进制数据，为了存储字符串，就需要建立**字符**与**二进制数据**的**映射关系**

需要搞清楚

+ 界定字符范围：要把哪些字符映射成二进制数据？
+ 怎么映射
  + 编码：字符 --（映射）--> 二进制
  + 解码：二进制 --（映射）--> 字符

> 抽象出一个字符集的概念来描述某个字符范围的编码规则

比如有如下表字符集

| 字符  | 字节   |
| ----- | ------ |
| `'a'` | `0x01` |
| `'b'` | `0x02` |
| `'A'` | `0x03` |
| `'B'` | `0x04` |

则有下面的转换

+ 字符串（`'bA'`） --> 二进制（`0000001000000001`）（十六进制 `0x0203`）
+ 字符串（`'baB'`） --> 二进制（`0000001000000001000001000`）（十六进制 `0x020104`）
+ 字符串（`'cd'`） 无法表示，因为上表字符集里木有 `'c'` 也木有 `'d'`

### 3.1.2 比较规则简介

> 该怎么比较两个字符的大小呢？

> 最容易想到的就是直接比较这两个字符对应的二进制编码的大小

比如按上表，`'a'` < `'b'` < `'A'` < `'B'`

也可以加上不区分大小写比较，那么 `'a'` = `'A'` < `'b'` = `'B'`

>  对于某一种字符集来说，可以制定用来比较字符大小的多种规则，aka：同一种字符集可以有多种比较规则

### 3.1.3 一些重要的字符集

一些常用字符集

+ **ASCII 字符集**：共收录128 个字符，可以使用一个字节来进行编码
+ **ISO 8859-1 字符集**：共收录 256 个字符，可以使用一个字节来进行编码
+ **GB2312 字符集**：收录了汉字以及拉丁字母等，同时又兼容 `ASCII` 字符集，如果该字符在 `ASCII` 字符集中，则采用一字节编码，否则采用两字节编码
+ **GBK 字符集**：收录的字符范围上对 `GB2312` 字符集进行了扩充，编码方式兼容 `GB2312` 字符集
+ **UTF-8 字符集**：几乎收录了当今世界各个国家/地区使用的字符，兼容 `ASCII` 字符集，采用变长编码方式（准确来说，`UTF-8` 只是 `Unicode` 字符集的一种编码方案）

## 3.2 MySQL 中支持的字符集和比较规则

### 3.2.1 MySQL 中的 utf8 和 utf8mb4

+ **utf8mb3**：“阉剖” 过的 UTF-8 字符集， 只使用 1 ~ 3 字节表示字符
+ **utf8mb4**：正宗的 UTF-8 字符集，使用1  ~ 4 字节表示字符

> 在 MySQL 8.0 中，MySQL 已经很大程度地优化了 `utf8mb4` 字符集的性能，而且已经将其设置为默认的字符集

### 3.2.2 字符集的查看

```mysql
SHOW (CHARACTER SET|CHARSET) [LIKE 匹配的模式];
```

### 3.2.3 比较规则的查看

```mysql
SHOW COLLATION [LIKE 匹配的模式];
```

比如

```mysql
mysql> show collation like 'utf8\_%';
+--------------------------+---------+-----+---------+----------+---------+---------------+
| Collation                | Charset | Id  | Default | Compiled | Sortlen | Pad_attribute |
+--------------------------+---------+-----+---------+----------+---------+---------------+
| utf8_bin                 | utf8mb3 |  83 |         | Yes      |       1 | PAD SPACE     |
| utf8_croatian_ci         | utf8mb3 | 213 |         | Yes      |       8 | PAD SPACE     |
| utf8_czech_ci            | utf8mb3 | 202 |         | Yes      |       8 | PAD SPACE     |
| utf8_danish_ci           | utf8mb3 | 203 |         | Yes      |       8 | PAD SPACE     |
| utf8_esperanto_ci        | utf8mb3 | 209 |         | Yes      |       8 | PAD SPACE     |
| utf8_estonian_ci         | utf8mb3 | 198 |         | Yes      |       8 | PAD SPACE     |
| utf8_general_ci          | utf8mb3 |  33 | Yes     | Yes      |       1 | PAD SPACE     |
| utf8_general_mysql500_ci | utf8mb3 | 223 |         | Yes      |       1 | PAD SPACE     |
| utf8_german2_ci          | utf8mb3 | 212 |         | Yes      |       8 | PAD SPACE     |
| utf8_hungarian_ci        | utf8mb3 | 210 |         | Yes      |       8 | PAD SPACE     |
| utf8_icelandic_ci        | utf8mb3 | 193 |         | Yes      |       8 | PAD SPACE     |
| utf8_latvian_ci          | utf8mb3 | 194 |         | Yes      |       8 | PAD SPACE     |
| utf8_lithuanian_ci       | utf8mb3 | 204 |         | Yes      |       8 | PAD SPACE     |
| utf8_persian_ci          | utf8mb3 | 208 |         | Yes      |       8 | PAD SPACE     |
| utf8_polish_ci           | utf8mb3 | 197 |         | Yes      |       8 | PAD SPACE     |
| utf8_romanian_ci         | utf8mb3 | 195 |         | Yes      |       8 | PAD SPACE     |
| utf8_roman_ci            | utf8mb3 | 207 |         | Yes      |       8 | PAD SPACE     |
| utf8_sinhala_ci          | utf8mb3 | 211 |         | Yes      |       8 | PAD SPACE     |
| utf8_slovak_ci           | utf8mb3 | 205 |         | Yes      |       8 | PAD SPACE     |
| utf8_slovenian_ci        | utf8mb3 | 196 |         | Yes      |       8 | PAD SPACE     |
| utf8_spanish2_ci         | utf8mb3 | 206 |         | Yes      |       8 | PAD SPACE     |
| utf8_spanish_ci          | utf8mb3 | 199 |         | Yes      |       8 | PAD SPACE     |
| utf8_swedish_ci          | utf8mb3 | 200 |         | Yes      |       8 | PAD SPACE     |
| utf8_tolower_ci          | utf8mb3 |  76 |         | Yes      |       1 | PAD SPACE     |
| utf8_turkish_ci          | utf8mb3 | 201 |         | Yes      |       8 | PAD SPACE     |
| utf8_unicode_520_ci      | utf8mb3 | 214 |         | Yes      |       8 | PAD SPACE     |
| utf8_unicode_ci          | utf8mb3 | 192 |         | Yes      |       8 | PAD SPACE     |
| utf8_vietnamese_ci       | utf8mb3 | 215 |         | Yes      |       8 | PAD SPACE     |
+--------------------------+---------+-----+---------+----------+---------+---------------+
28 rows in set (0.00 sec)
```

+ 比较规则的名称以与其关联的字符集的名称开头
+ 后面紧跟着该比较规则所应用的语言
+ 名称后缀意味着该比较规则是否区分语言中的重音、大小写等

| 后缀   | 英文释义             | 描述             |
| ------ | -------------------- | ---------------- |
| `_ai`  | `accent insensitive` | 不区分重音       |
| `_as`  | `accent sensitive`   | 区分重音         |
| `_ci`  | `case insensitive`   | 不区分大小写     |
| `_cs`  | `case sensitive`     | 区分大小写       |
| `_bin` | `binary`             | 以二进制方式比较 |

> `Defualt` 列的值 `YES` 说明是该字符集的默认比较规则

## 3.3 字符集和比较规则的应用

### 3.3.1 各级别的字符集和比较规则

**4 个级别的字符集和比较规则**

+ 服务器
+ 数据库
+ 表
+ 列

#### 3.3.1.1 服务器级别

| 系统变量               | 描述                 |
| ---------------------- | -------------------- |
| `character_set_server` | 服务器级别的字符集   |
| `collation_server`     | 服务糖级别的比较规则 |

可以通过启动选项或者在服务器程序运行过程中使用 `SET` 语句来修改这两个变量的值

比如：

```
[server]
character_set_server=gb2312
collation_server=gb2312_chinese_ci
```

#### 3.3.1.2 数据库级别

在创建和修改数据库时可以指定该数据库的字符集和比较规则

```mysql
CREATE DATABASE 数据库名
	[[DEFAULT] CHARACTER SET 字符集名称]
	[[DEFAULT] COLLATE 比较规则名称];
	
ALTER DATABASE 数据库名
	[[DEFAULT] CHARACTER SET 字符集名称]
	[[DEFAULT] COLLATE 比较规则名称];
```

> 可以不用写 `DEFAULT`

查看当前数据库使用的字符集和比较规则可以查看表中的两个系统变量的值（前提是使用 `USE` 语句选择当前的默认数据库，如果没有默认数据库，则变量与服务器级别下相应的系统变量具有相同的值)）

| 系统变量                 | 描述               |
| ------------------------ | ------------------ |
| `character_set_database` | 当前数据库的字符集 |
| `collation_database`     | 当前数据库比较规则 |

> 这两个变量只能看不能改（改了没用）
>
> 数据库的创建语句中也可以不指定字符集和比较规则，这将使用**服务器级别**的字符集和比较规则作为数据库的字符集和比较规则

#### 3.3.1.3 表级别

在创建和修改表的时候指定表的字符集和比较规则

```mysql
CREATE TABLE 表名 (列信息)
	[[DEFAULT] CHARACTER SET 字符集名称]
	[COLLATE 比较规则名称];
	
ALTER TABLE 表名
	[[DEFAULT] CHARACTER SET 字符集名称]
	[COLLATE 比较规则名称];
```

> 建表没写，默认用所在数据库的字符集和比较规则

#### 3.3.1.4 列级别

对于**存储字符串**的列，同一个表中不同的列也可以有不同的字符集和比较规则

```mysql
CREATE TABLE 表名 (
	列名 字符串类型 [CHARACTER SET 字符集名称] [COLLATE 比较规则名称],
	其他列...
);

ALTER TABLE 表名 MODIFY 列名 字符串类型 [CHARACTER SET 字符集名称] [COLLATE 比较规则名称];
```

> 同样的，如果没有指定默认是以表的

#### 3.3.1.5 仅修改字符集或仅修改比较规则

由于字符集和比较规则之间相互关联，因此如果只修改字符集，比较规则也会跟着变化；如果只修改比较规则，字符集也会跟着变化，具体规则如下：

+ 只修改字符集，则比较规则将变为修改后的字符集默认的比较规则
+ 只修改比较规则，则字符集将变为修改后的比较规则对应的字符集

#### 3.3.1.6 各级别字符集相比较规则小结

列 <-- 表 <-- 库

### 3.3.2 客户端和服务器通信过程中使用的字符集

#### 3.3.2.1 编码和解码使用的字符集不一致

>  字符串在计算机上的体现就是一个字节序列.如果使用不同的字符集去解码这个字节序列，最后得到的结果可能让你挠头

#### 3.3.2.2 字符集转换的概念

字节序列用 A 字符集进行解码，再用 B 字符集进行编码，转成新的字节序列，这个过程称为**字符集的转换**

#### 3.3.2.3 MySQL 中的字符集转换过程

**客户端发送请求**

一般情况下，客户端编码请求字符串时使用的字符集与操作系统当前使用的字符集一致

+ 类 `UNIX` 操作系统

  + `LC_ALL、LC_CTYPE、LANG` 这 3 个环境变量的值决定了操作系统当前使用的是哪种字符集，优先级 `LC_ALL` > `LC_CTYPE` > `LANG`

  + 如果这 3 个环境变量都没有设置，那么操作系统当前使用的字符集就是其默认的字符集。比如在我的 macOS 10.15.3 操作系统中，默认的字符集为 US-ASCII 

+ `Windows` 操作系统

  + 字符集称为代码页（code page )，一个代码页与一个唯一的数字相关联。比如 936 代表 `GBK` 字符集，65001 代表 `UTF-8` 字符集
  + 在 Windows 命令行窗口运行 `chcp` 命令， 查看当前代码页
  + 在启动 MySQL 客户端程序时携带了`default-character-set` 启动选项， 那么 MySQL 客户端将以该启动选项指定的字符集对请求的字符串进行编码（这一点不同于类 `Unix` 系统）

**服务器接收请求**

从本质上来说，服务器接收到的请求就是一个字节序列，服务器将这个字节序列看作是使用系统变量 `character_set_client` 代表的字符集进行编码的字节序列（每个客户端与服务器建立连接后，服务器都会为该客户端维护-个单独的 `character_set_client` 变量，这个变量是 `SESSION` 级别的）

> 客户端在编码请求字符串时实际使用的字符集，与服务器在收到一个字节序列后认为该字节序列所采用的编码字符集，是两个独立的字符集 。一般情况下， 我们应该尽量保证这两个字符集是一致的

>  另外需要注意的是，如果 `character_set_client` 对应的字符集不能解释请求的字节序列，那么服务器就会发出警告 

```mysql
mysql> set character_set_client=ascii;
Query OK, 0 rows affected (0.00 sec)

mysql> select '我';
+-----+
| ??? |
+-----+
| ??? |
+-----+
1 row in set, 1 warning (0.00 sec)

mysql> show warnings\G;
*************************** 1. row ***************************
  Level: Warning
   Code: 1300
Message: Cannot convert string '\xE6\x88\x91' from ascii to utf8mb4
1 row in set (0.01 sec)
```

客户端实际使用 UTF-8 字符集来编码请求的字符串，现在把 `character_set_client` 设置成 `ASCII` 字符集，而请求字符串中包含了一个汉字 '我'（对应的字节序列就是 `0xE68891`），那么会发生这样的事情

**服务器处理请求**

服务器会将请求的字节序列当作采用 `character_set_client` 对应的字符集进行编码的字节序列，不过在真正处理请求时又会将其转换为使用 `SESSION` 级别的系统变量 **`character_set_connection`** 对应的字符集进行编码的字节序列

可以通过 `SET` 命令单独修改 `character_set_connection` 系统变量

考虑下面的查询语句

```mysql
SELECT 'a' = 'A';
```

仅仅根据这个语句是不能确定结果的，这是因为我们并不知道这两个字符串到底采用了什么字符集进行编码，也不知道这里使用的比较规则是什么

`character_set_connection` 系统变量表示这些字符串应该使用哪种字符集进行编码，还有一个与之配套的系统变量 `collation_connection` ，这个系统变量表示这些字符串应该使用哪种比较规则

```mysql
mysql> SET character_set_connection=gbk;
Query OK, 0 rows affected (0.06 sec)

mysql> SET collation_connection=gbk_chinese_ci;
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT 'a' = 'A';
+-----------+
| 'a' = 'A' |
+-----------+
|         1 |
+-----------+
1 row in set (0.04 sec)

mysql> SET character_set_connection=gbk;
Query OK, 0 rows affected (0.00 sec)

mysql> SET collation_connection=gbk_bin;
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT 'a' = 'A';
+-----------+
| 'a' = 'A' |
+-----------+
|         0 |
+-----------+
1 row in set (0.00 sec)
```

考虑请求中的字符串和某个列进行比较的情况，比如我们有一个表：

```mysql
mysql> CREATE TABLE tt (
    -> c VARCHAR(100)
    -> ) ENGINE=INNODB CHARSET=utf8;
Query OK, 0 rows affected, 1 warning (0.70 sec)
```

列 `c` 采用的字符集和表级别字符集 utf8 一致，这里采用默认的比较规则 `utf8_general_ci`，表中有一条记录

```mysql
mysql> SELECT * FROM tt;
+------+
| c    |
+------+
| 我   |
+------+
1 row in set (0.05 sec)
```

假设现在 `character_set_connection` 和 `collation_connection` 的值分别设置为 `gbk` 和 `gbk_chinese_ci`，然后我们有下面这样一条查询语句：

```mysql
SELECT * FROM tt WHERE c = '我';
```

+ 字符串 `'我'` 是使用 `gbk` 字符集进行编码的，比较规则是 `gbk_chinese_ci`
+ 列 `c` 是采用 `utf8` 字符集进行编码的，比较规则是 `utf8_general_ci`
+ 规定上述情况下，列的字符集和排序规则的优先级更高
+ 因此需要将请求中的字符串 `'我'` 先从 `gbk` 字符集转换为 `utf8` 字符集，然后再使用列 `c` 的比较规则 `utf8_general_ci` 进行比较

**服务器生成响应**

列 `c` 是使用 `utf8` 字符集进行编码的，所以字符串 `'我'` 在列中的存放格式就是 `0xE68891`，执行语句

```mysql
SELECT * FROM tt;
```

是否是将 `0xE68891` 读出后发送到客户端呢？

> 这可不一定， 这取决于 `SESSION` 级别的系统变量 `character_set_results` 的值

服务器会先将字符串 `'我'` 从 `utf8` 字符集编码的 `0xE68891` 转换成 `character_set_results` 系统变量对应的字符集编码后的字节序列，之后再发给客户端

可以使用 `SET` 命令来修改 `character_set_results` 的值

```mysql
SET character_set_results = gbk;
```

> 执行 `SELECT * FROM tt` 语句，在服务器返回给客户端的响应中，字符串 `'我'` 对应的就是字节序列 `0xCED2`

**总结**

| 系统变量                   | 描述                                                         |
| -------------------------- | ------------------------------------------------------------ |
| `character_set_client`     | 服务器认为**请求**是按照该系统变量指定的字符集进行编码的     |
| `character_set_connection` | 服务器在**处理**请求时，会把请求字节序列从 `character_set_client` 转换为 `character_set_connection` |
| `character_set_results`    | 服务器采用该系统变量指定的字符集对**返回**给客户端的字符串进行编码 |

>  这 3 个系统变量在服务器中的作用范围都是 `SESSION` 级别；
>
> 每个客户端在与服务器建立连接后，服务器都会为这个连接维护这 3 个变量

![image-20220510143729056](/Users/daydaylw3/Pictures/typora/image-20220510143729056.png)

+ 每个 MySQL 客户端都维护着一个客户端默认字符集
+ 客户端在启动时会自动检测所在操作系统当前使用的字符集，并**按照一定的规则映射成 MySQL 支持的字符集**，然后将该字符集作为客户端默认的字符集
+ 通常的情况是，操作系统当前使用什么字符集，就映射为什么字符集
+ 存在一些特殊情况：假如操作系统当前使用的是 `ASCII` 字符集，则会被映射为 MySQL 支持的 `latin1` 字符集
+ 如果 MySQL 不支持操作系统当前使用的字符集，则会**将客户端默认的字符集设置为 MySQL 的默认字符集**

> 如果在启动 MySQL 客户端时设置了 `default-character-set` 启动选项，那么服务器会忽视操作系统当前使用的字符集，直接将 `default-character-set` 启动选项中指定的值作为客户端的默认字符集

+ 在连接服务器时，客户端将默认的字符集发给服务器（连同用户名密码等信息）

+ 服务器收到后将 `character_set_client`、`character_set_connection`、`character_set_results` 这 3 个系统变量的值初始化成客户端默认的字符集

+ 客户端连接成功后，可以使用 `SET` 语句分别修改`character_set_client`、`character_set_connection`、`character_set_results` 系统变量的值

+ 也可以使用下面的语句一次性修改

  ```mysql
  SET NAMES charset_name;
  ```

+ `SET NAME` 语句并不会修改客户端在编码请求字符串时使用的字符集，也不会修改客户端的默认字符集

**客户端接收到响应**

> 客户端收到的响应其实也是一个字节序列

需要用一定的规则进行**解码**

+ 类 `UNIX`：把接收到的字节序列，默认使用操作系统当前使用的字符集来解码（基本上）
+ 类 `Windows`：使用客户端默认的字符集来解码

### 3.3.3 比较规则的应用

> 比较规则通常用来比较字符串的大小以及对某些字符串进行排序，所以有时候也称为排序规则

**举例**

表 `t` 的列 `c` 使用的字符集是 `gbk`，比较规则是 `gbk_chinese_ci`

```mysql
mysql> INSERT INTO t (c) VALUES ('a'), ('b'), ('A'), ('B'), ('我');
Query OK, 5 rows affected (0.02 sec)
Records: 5  Duplicates: 0  Warnings: 0

mysql> SELECT * FROM t ORDER BY c;
+------+
| c    |
+------+
| a    |
| A    |
| b    |
| B    |
| 我   |
+------+
5 rows in set (0.00 sec)
```

> 可以看到在默认的比较规则的 `gbk_chinese_ci` 中是不区分大小写的

将列 `c` 的比较规则更改为 `gbk_bin`

```mysql
mysql> ALTER TABLE t MODIFY c VARCHAR(100) COLLATE gbk_bin;
Query OK, 0 rows affected (0.01 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> SELECT * FROM t ORDER BY c;
+------+
| c    |
+------+
| A    |
| B    |
| a    |
| b    |
| 我   |
+------+
5 rows in set (0.00 sec)
```

> 由于 `gbk_bin` 是直接比较字符的二迸制编码，所以是区分大小写的

## 3.4 总结

+ 字符集指的是某个字符范围的编码规则
+ 比较规则是对某个字符集中的字符比较大小的一种规则
+ 字符集和比较规则是一对多，字符集有默认的比较规则
+ 查看字符集和比较规则的方法
+ MySQL 4 个级别的字符集和比较规则：服务器级别，数据库级别，表级别，列级别
+ 从发送请求到接收响应过程中发生的字符集转换
  + 客户端发迭的请求字节序列是采用哪种字符线进行编码的
  + 服务器接收到请求字节序列后会认为它是采用哪种字符集进行编码的
  + 服务器在运行过程中会把请求的字节序列转换为以哪种字符集编码的字节序列
  + 服务器在向客户端返回字节序列时，是采用哪种字符集进行编码的
  + 客户端在收到响应字节序列后，是怎么把它们写到黑框框中的
+ 比较规则通常用来比较字符串的大小以及对某些字符串进行排列

------

[toc]
