[toc]

------

## 16.1 optimizer trace 简介

系统变量 optimizer_trace

```mysql
set optimizer_trace="enabled=on";
```

information_schema 数据库下的 OPTIMIZER_TRACE 表

有 4 列

+ QUERY：
+ TRACE：
+ MISSING_BYTE_BEYOND_MAX_MEM_SIZE：超过某个限制，多余的文本将不会显示。展示被忽略的字节数
+ INSUFFICIENT_PRIVILEGES：

**TRACE**

大致 3 个阶段

+ prepare
+ optimize
+ execute

成本优化主要集中在 optimize

单表查询关注 rows_estimation

多表关注 considered_execution_plans

------

[toc]