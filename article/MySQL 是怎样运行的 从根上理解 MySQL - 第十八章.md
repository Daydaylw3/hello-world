[toc]

------

## 18.1 事务的起源

### 18.1.1 原子性（Atomicity）

>  如果在执行操作的过程中发生了错误，就把己经执行的操作恢复成没执行之前的样子

### 18.1.2 隔离性（Isolation）

### 18.1.3 一致性（Consistency）

数据库中的数据符合现实世界的约束（比如年龄不能为负）

> 我们一般在定义一致性需求时， 只要某些数据库操作满足原子性和隔离性规则，那么这些操作执行后的结果就会满足一致性需求

### 18.1.4 持久性（Durability）

## 18.2 事务的概念

要保证 ACID 的一个或者多个数据库操作称为事务（transaction）

**状态**

+ 活动的（active）
+ 部分提交的（partially committed）：事务最后一个操作完成，但是还在内存未刷盘
+ 失败的（failed）：事务处于活动或者部分提交的状态，遇到某些错误导致无法继续执行，停止了当前事务的执行
+ 中止的（aborted）：回滚之后回到了执行事务之前的状态，就是中止的状态
+ 提交的（committed）：

<img src="/Users/daydaylw3/Pictures/typora/image-20220519214440780.png" alt="image-20220519214440780" style="zoom:50%;" />

## 18.3 MySQL 中事务的语法

### 18.3.1 开启

+ BEGIN [WORK];
+ START TRANSACTION;
  + READ ONLY：（可以读写临时表）
  + READ WRITE：（默认模式）
  + WITH CONSISTENT SNAPSHOT：启动一致性读

### 18.3.2 提交事务

COMMIT [WORK];

### 18.3.3 手动中止事务

ROLLBACK [WORK];

### 18.3.4 支持事务的存储引擎

InnoDB 和 NDB

### 18.3.5 自动提交

系统变量 autocommit

想关闭自动提交

+ 显式地使用 START TRANSACTION 或者 BEGIN 语句开启一个事务

  会暂时关闭自动提交

+ SET autocommit = OFF;

### 18.3.6 隐式提交

某些语句会导致事务悄悄地提交掉

+ 定义或者修改数据库对象的数据定义语言（DDL，Data Definition Language）

+ 隐式使用或者修改 mysql 数据库中的表

+ 事务控制 或者 关于锁定的语句 LOCK TABLES、UNLOCK TABLES

+ 加载数据的语句 LOAD DATA

+ 关于 MySQL 复制的一些语句

  START SLAVE、STOP SLAVE、RESET SLAVE、CHANGE MASTER TO

+ 其他语句

  ANALYZE TABLE、CACHE INDEX、CHECK TABLE、FLUSH、LOAD INDEX INTO CACHE、OPTIMIZE TABLE、REPAIR TABLE、RESET 等

### 18.3.7 保存点

（savepoint）

ROLLBACK 可以指定回到哪个点

SAVEPOINT 保存点名称;

ROLLBACK [WORK] TO [SAVEPOINT] 保存点名称;

RELEASE SAVEPOINT 保存点名称;

------

[toc]