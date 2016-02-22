# ThisSocks
基于linux的简单的socks5代理C/S程序

## 当前功能
* TCP代理
* HTTP/HTTPS代理
* 远程DNS
* 用户验证
* 数据加密

## 编译
    git clone https://github.com/Onway/ThisSocks.git
	cd ThisSocks
	make

## 运行
    # 服务端
	./ThisSocks -C conf/server.conf

	# 客户端
	./ThisSocks -C conf/client.conf

## 注意
* 服务端采用的是perfork + select
