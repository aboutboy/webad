SUBDIRS=`ls -d */ |grep -v 'conf'|grep -v 'install'`
define make_subdir
	for subdir in $(SUBDIRS) ; do \
	( cd $$subdir && ./configure && make ) \
	done
endef
all:
	$(call make_subdir , all)
debug:
	$(call make_subdir , debug)
clean:
	$(call make_subdir , clean) 
