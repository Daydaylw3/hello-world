[toc]

------

### 21.2.1 事务并发执行时遇到的一致性问题

#### 脏写（Dirty Write

一个事务修改了另一个未提交事务修改过的数据（P0

```
w1[x]...w2[x]...((c1 or a1) and (c2 or a2) in any order)
```

破坏一致性，原子性，持久性的案例

```
w1[x=1]w2[x=2]w2[y=2]c2w1[y=1]

w1[x=2]w2[x=3]w2[y=3]c2a1
```

#### 脏读（Dirty Read

一个事务读到了另一个未提交事务修改过的数据（P1

```
w1[x]...r2[x]...((c1 or a1) and (c2 or a2) in any order)
```

引发一致性问题

```
w1[x=1]r2[x=1]r2[y=0]c2w1[y=1]c1
```

P1 的严格解释 A1

```
w1[x]...r2[x]...(a1 and c2 in any order)
```

#### 不可重复度（Non-Repeatable Read

一个事务修改了另一个未提交事务读取的数据（P2

```
r1[x]...w2[x]...((c1 or a1) and (c2 or a2) in any order)
```

一致性问题

```
r1[x=0]w2[x=1]w2[y=1]c2r1[y=1]c1
```

P2 的严格解释 A2

```
r1[x]...w2[x]...c2...r1[x]...c1
```

#### 幻读（Phantom

一个事务先根据某些搜索条件查询出一些记录，在该事务未提交时，另一个事务写入一些了符合那些搜索条件的记录（P3

```
r1[P]...w2[y in P]...((c1 or a1) and (c2 or a2) an order)
```

P3 的严格解释 A3

```
r1[P]...w2[y in P]...c2...r1[P]...c1
```

### 21.2.2 SQL 标准中的 4 种隔离级别

+ READ UNCOMMITTED
+ READ COMMITTED
+ REPEATABLE READ
+ SERIALIZABLE

| 隔离级别         | 脏读 | 不可重复读 | 幻读 |
| ---------------- | ---- | ---------- | ---- |
| READ UNCOMMITTED | √    | √          | √    |
| READ COMMITTED   | ×    | √          | √    |
| REPEATABLE READ  | ×    | ×          | √    |
| SERIALIZABLE     | ×    | ×          | ×    |

### 21.2.3 MySQL 中支持的 4 种隔离级别

默认是 REPEATABLE READ

#### 设置事务隔离级别

```
SET [GLOBAL|SESSION] TRANSACTION ISOLATION LEVEL level;
```

启动选项 transaction_isolation

修改系统变量 transaction_isolation

| 语法                                        | 作用范围   |
| ------------------------------------------- | ---------- |
| SET GLOBAL transaction_isolation = 隔离级别 | 全局       |
| SET @@GLOBAL.var_name = 隔离级别            | 全局       |
| SET SESSION var_name = 隔离级别             | 会话       |
| SET @@SESSION.var_name = 隔离级别           | 会话       |
| SET var_name = 隔离级别                     | 会话       |
| SET @@var_name = 隔离级别                   | 下一个事务 |

## 21.3 MVCC 原理

### 21.3.1 版本链

多版本并发控制（Multi-Version Concurrency Control）

### 21.3.2 ReadView

READ COMMITTED 和 REPEATABLE READ 创建 ReadView 的时机不同