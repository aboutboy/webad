����
1��libmnl(�Ѿ��ϲ�����Ŀ��)
2��libnfnetlink(�Ѿ��ϲ�����Ŀ��)
3��libnetfilter_queue(�Ѿ��ϲ�����Ŀ��)
4�������ںˣ�֧��netfilter,netlink(Ĭ���Ѿ�����)

ע��
����ں˲�֧��netfilter
	ִ��make memuconfig������netfilterѡ��
	Networking-->Networking Options-->Network Packet Filtering Framework-->
	Core Netfilter Configuration(����Netfilter����)��IP��Netfilter Configuration ��IP��Netfilter���ã�

��ֲ
webad�û�̬����
1����������õ�external/webad
2���޸Ŀ��������ļ�device/mediatek/mt6582/init.mt6582.rc��/system/core/rootdir/init.rc
	service webad /system/bin/webad
		class main
    	user root
    	group root

webad�ں�̬����
1����������õ�kernel-3.10/drivers/webad
2�����obj-$(CONFIG_WEBAD) += webad.o �� kernel-3.10/drivers/webad/Makefile
3�����obj-$(CONFIG_WEBAD)		+= webad/ �� ../kernel-3.10/drivers/Makefile
4�����source "drivers/webad/Kconfig" �� ../kernel-3.10/drivers/Kconfig

���밲װ
1����android��Ŀ¼ִ��make�����������û�̬�����ִ��mmm external/webad
2��������������ļ�����ˢ��