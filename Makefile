SUBDIRS=`ls -d */ |grep -v 'conf'|grep -v 'install'`
CURRENT_DIR=`pwd`
define make_subdir
	cd $(CURRENT_DIR)/libmnl && chmod a+x configure && ./configure --prefix=/usr && make clean && make && make install
        cd $(CURRENT_DIR)/libnfnetlink && chmod a+x configure && ./configure --prefix=/usr && make clean && make && make install
        cd $(CURRENT_DIR)/libnetfilter_queue && chmod a+x configure && ./configure --prefix=/usr LIBNFNETLINK_CFLAGS=-I$(CURRENT_DIR)/libnfnetlink/include/ LIBNFNETLINK_LIBS=-L$(CURRENT_DIR)/libnfnetlink/src/.libs/ LIBMNL_CFLAGS=-I$(CURRENT_DIR)/libmnl/include/ LIBMNL_LIBS=-L$(CURRENT_DIR)/libmnl/src/.libs/  && make clean && make && make install
	cd $(CURRENT_DIR)/webad && rm webad -f&& make
endef
all:
	$(call make_subdir , all)
