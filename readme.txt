����
1��libipq.so(�Ѿ��ϲ�����Ŀ��)
2��ip_queue.ko(����netfilter��֧�ִ�ģ��)
3��iptables(androidϵͳ�Դ�)
4�������ںˣ�֧��netfilter(Ĭ���Ѿ�����)

��ֲ
1������ں˲�֧��netfilter
	ִ��make memuconfig������netfilterѡ��
	Networking-->Networking Options-->Network Packet Filtering Framework-->
	Core Netfilter Configuration(����Netfilter����)��IP��Netfilter Configuration ��IP��Netfilter���ã�
2����������õ�external/webad
3���޸Ŀ��������ļ�device/mediatek/mt6582/init.mt6582.rc����ĩβ���
	service webad /system/bin/webad
    	class main
    	user system
    	group system
4����android��Ŀ¼ִ��make����
5��������������ļ�����ˢ��

����
1��Unable to create netlink socket: Protocol not supported