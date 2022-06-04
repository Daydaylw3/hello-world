

```
/etc/sudoers

root ALL=(ALL) ALL
%adm ALL=(ALL) NOPASSWD:ALL

gpasswd -a 用户名 adm
```

Redis

+ 单线程

快

+ C 语言编写
+ 单线程，少了上下文切换
+ IO 多路复用
+ 基于内存

+ 支持数据持久化
+ 支持主从集群（安全策略、读写分离），分片集群（水平扩展）

## 安装

```
https://redis.io
```

Download

sftp 上传至云服务器

解压

进入 redis 目录

```
sudo yum install -y gcc tcl # 安装gcc
sudo make && sudo make install
```

#### redis 配置文件

```
redis 目录下的 redis.conf
```

#### redis 重要配置

+ bind 0.0.0.0
+ daemonize yes
+ requirepass 密码
+ port 6379
+ dir ./
+ databases 数据库数量
+ maxmemory 512mb
+ logfile "redis.log"

#### redis 开机自启

```
sudo vi /etc/systemd/system/redis.service
[Unit]
Description=redis-server
After=network.target

[Service]
Type=forking
ExecStart=/usr/local/bin/redis-server /usr/local/src/redis-6.2.7/redis.conf
PrivateTmp=true

[Install]
WantedBy=multi-user.target

sudo systemctl daemon-reload
sudo systemctl start redis
sudo systemctl status redis
sudo systemctl stop redis
sudo systemctl restart redis
sudo systemctl enable redis # 开机自启
```



```
setnx
set key value nx


```

![image-20220531211402570](/Users/daydaylw3/Pictures/typora/image-20220531211402570.png)

Redis key 的层级结构

Proj:Type:name

Hash

`hset key field value`

![image-20220601224210322](/Users/daydaylw3/Pictures/typora/image-20220601224210322.png)

![image-20220602215035199](/Users/daydaylw3/Pictures/typora/image-20220602215035199.png)
