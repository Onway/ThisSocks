#!/bin/bash

http=(
	http://cn.bing.com/
	http://www.sina.com.cn/
)

https=(
	https://www.baidu.com/
	https://github.com/
	https://www.zhihu.com/
)

proxy=127.0.0.1:1081

curl_opt="-o /dev/null"

function run_curl()
{
	func_opt=$1
	protocol=$2
	url=$3
	cmd="curl $func_opt $curl_opt -x ${protocol}${proxy} $url"
	echo $cmd
	$cmd
	ret=$?
	if [ $ret -ne 0 ]; then
		exit 1
	fi
	echo
}

function get_test()
{
	for url in ${http[@]}
	do
		run_curl "" "http://" $url
	done
}

function connect_test()
{
	for url in ${http[@]}
	do
		run_curl "-p" "http://" $url
	done

	for url in ${https[@]}
	do
		run_curl "-p" "http://" $url
	done
}

function socks_test()
{
	for url in ${http[@]}
	do
		run_curl "" "sock5://" $url
	done

	for url in ${https[@]}
	do
		run_curl "-p" "socks5-hostname://" $url
	done
}

get_test
connect_test
socks_test
