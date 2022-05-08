### 慢 SQL

**问：遇到慢 SQL，第一反应？**

答：explain，查看是否有效利用到索引，构建合理索引

**问：为啥加了索引就一定快？底层机制如何？**

### 索引的本质

> 索引是帮助 MySQL 高效获取数据的**排好序**的**数据结构**

**索引的数据结构**

+ 二叉树
+ 红黑树
+ Hash 表
+ B-Tree

### BufferPool 缓存机制

#### 索引存储在磁盘的位置

```
MyISAM
/data/${database_name}/${table_name}_myisam.frm - 表结构文件frame
/data/${database_name}/${table_name}_myisam.MYD - data
/data/${database_name}/${table_name}_myisam.MYI - index

Innodb
/data/${database_name}/${table_name}_innodb_lock.frm - 表结构
/data/${database_name}/${table_name}_innodb_lock.ibd - 数据+索引
```

+ 建议 Innodb 表必须建主键，并且推荐使用整型的自增主键？
+ 为什么非主键索引结构叶子节点存储的是主键值？（一致性和节省存储空间）

#### B 树

+ 数据页之间没有指针
+ B 树非叶子节点也有数据

#### 联合索引底层存储结构

### binlog

+ 二进制日志
+ server 层实现（引擎共用）
+ 逻辑日志，记录一条语句原始逻辑
+ 不限大小，追加写入，不会覆盖以前的日志

**开启 binlog**

```mysql
mysql> show variables like '%log_bin%';
+---------------------------------+------------------------------------+
| Variable_name                   | Value                              |
+---------------------------------+------------------------------------+
| log_bin                         | ON                                 |
| log_bin_basename                | /usr/local/mysql/data/binlog       |
| log_bin_index                   | /usr/local/mysql/data/binlog.index |
| log_bin_trust_function_creators | OFF                                |
| log_bin_use_v1_row_events       | OFF                                |
| sql_log_bin                     | ON                                 |
+---------------------------------+------------------------------------+
6 rows in set (0.27 sec)
```

**binlog 位置**

mac 下：`/usr/local/mysql/data/binlog.*`，默认需要赋权，或者 `sudo`

**查看命令**

`mysqlbinlog --no-defaults binlog.*`，默认需要赋权，或者 `sudo`

```
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=1*/;
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# at 4
#220508 19:50:04 server id 1  end_log_pos 126 CRC32 0x2bed3546 	Start: binlog v 4, server v 8.0.29 created 220508 19:50:04
# Warning: this binlog is either in use or was not closed properly.
BINLOG '
bK53Yg8BAAAAegAAAH4AAAABAAQAOC4wLjI5AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAEwANAAgAAAAABAAEAAAAYgAEGggAAAAICAgCAAAACgoKKioAEjQA
CigAAUY17Ss=
'/*!*/;
# at 126
#220508 19:50:04 server id 1  end_log_pos 157 CRC32 0x83ab5f9a 	Previous-GTIDs
# [empty]
# at 157
#220508 19:50:56 server id 1  end_log_pos 236 CRC32 0xe1bd2974 	Anonymous_GTID	last_committed=0	sequence_number=1	rbr_only=yes	original_committed_timestamp=1652010656175908	immediate_commit_timestamp=1652010656175908	transaction_length=302
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=1652010656175908 (2022-05-08 19:50:56.175908 CST)
# immediate_commit_timestamp=1652010656175908 (2022-05-08 19:50:56.175908 CST)
/*!80001 SET @@session.original_commit_timestamp=1652010656175908*//*!*/;
/*!80014 SET @@session.original_server_version=80029*//*!*/;
/*!80014 SET @@session.immediate_server_version=80029*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at 236
#220508 19:50:56 server id 1  end_log_pos 313 CRC32 0x162dd951 	Query	thread_id=11	exec_time=0	error_code=0
SET TIMESTAMP=1652010656/*!*/;
SET @@session.pseudo_thread_id=11/*!*/;
SET @@session.foreign_key_checks=1, @@session.sql_auto_is_null=0, @@session.unique_checks=1, @@session.autocommit=1/*!*/;
SET @@session.sql_mode=1168113696/*!*/;
SET @@session.auto_increment_increment=1, @@session.auto_increment_offset=1/*!*/;
/*!\C utf8mb4 *//*!*/;
SET @@session.character_set_client=255,@@session.collation_connection=255,@@session.collation_server=255/*!*/;
SET @@session.lc_time_names=0/*!*/;
SET @@session.collation_database=DEFAULT/*!*/;
/*!80011 SET @@session.default_collation_for_utf8mb4=255*//*!*/;
BEGIN
/*!*/;
# at 313
#220508 19:50:56 server id 1  end_log_pos 374 CRC32 0x4bd9b9a6 	Table_map: `dayday`.`t1` mapped to number 115
# at 374
#220508 19:50:56 server id 1  end_log_pos 428 CRC32 0x0668fa36 	Write_rows: table id 115 flags: STMT_END_F

BINLOG '
oK53YhMBAAAAPQAAAHYBAAAAAHMAAAAAAAEABmRheWRheQACdDEABQMDAwMPAlAAHgEBAAID/P8A
prnZSw==
oK53Yh4BAAAANgAAAKwBAAAAAHMAAAAAAAEAAgAF/wAJAAAACQAAAAkAAAAJAAAAAWE2+mgG
'/*!*/;
# at 428
#220508 19:50:56 server id 1  end_log_pos 459 CRC32 0xa2b7dda1 	Xid = 487
COMMIT/*!*/;
SET @@SESSION.GTID_NEXT= 'AUTOMATIC' /* added by mysqlbinlog */ /*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=0*/;
```

