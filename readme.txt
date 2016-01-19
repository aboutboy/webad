依赖
1、libmnl(已经合并到项目中)
2、libnfnetlink(已经合并到项目中)
3、libnetfilter_queue(已经合并到项目中)
4、配置内核，支持netfilter,netlink(默认已经开启)

注意
如果内核不支持netfilter
	执行make memuconfig，开启netfilter选项
	Networking-->Networking Options-->Network Packet Filtering Framework-->
	Core Netfilter Configuration(核心Netfilter配置)和IP：Netfilter Configuration （IP：Netfilter配置）

移植
webad用户态代码
1、将代码放置到external/webad
2、修改开机启动文件device/mediatek/mt6582/init.mt6582.rc或/system/core/rootdir/init.rc
	service webad /system/bin/webad
		class main
    	user root
    	group root

webad内核态代码
1、将代码放置到kernel-3.10/drivers/webad
2、添加obj-$(CONFIG_WEBAD) += webad.o 到 kernel-3.10/drivers/webad/Makefile
3、添加obj-$(CONFIG_WEBAD)		+= webad/ 到 ../kernel-3.10/drivers/Makefile
4、添加source "drivers/webad/Kconfig" 到 ../kernel-3.10/drivers/Kconfig

编译安装
1、在android根目录执行make命令或者针对用户态代码可执行mmm external/webad
2、将编译出来的文件进行刷机