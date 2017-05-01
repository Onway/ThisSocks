# ThisSocks
基于linux的代理C/S程序

## 当前功能
* TCP代理
* HTTP/HTTPS代理
* 远程DNS
* 用户验证
* 数据加密

## 编译
    git clone https://github.com/Onway/ThisSocks.git
	cd ThisSocks/src
	make

## 运行
    # 服务端侦听127.0.0.1:8389
	./ThisSocks -D -C ../conf/server.conf

	# 客户端侦听127.0.0.1:1081
	./ThisSocks -D -C ../conf/client.conf

	# 运行使用了curl的测试脚本
    cd ../test
	./curl.bash

## 安装
	sudo make install

	# 启动客户端和服务端服务
	service ThisSocks_C start
	service ThisSocks_S start

	# 停止服务
	service ThisSocks_C stop
	service ThisSocks_S stop

## 卸载
	# 在src目录执行
    sudo make uninstall

## 开机启动
	# 安装后执行
	sudo update-rc.d ThisSocks_C defaults
	sudo update-rc.d ThisSocks_S defaults

	# 卸载后执行
	sudo update-rc.d ThisSoks_C remove
	sudo update-rc.d ThisSoks_S remove
