����
1��libmnl(�Ѿ��ϲ�����Ŀ��)
2��libnfnetlink(�Ѿ��ϲ�����Ŀ��)
3��libnetfilter_queue(�Ѿ��ϲ�����Ŀ��)
4�������ںˣ�֧��netfilter,netlink(Ĭ���Ѿ�����)

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
4����android��Ŀ¼ִ��make�������ִ��mmm external/webad
5��������������ļ�����ˢ��
