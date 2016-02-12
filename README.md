# ThisSocks
基于linux的简单的socks5代理C/S程序

## 当前功能
* TCP代理
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

## 浏览器设置
firefox浏览器设置SOCKSv5代理
![image](https://github.com/Onway/ThisSocks/raw/master/img/setting.png)

## 注意
* 不支持socks5的浏览器无法使用
* 服务端采用的是perfork模式
