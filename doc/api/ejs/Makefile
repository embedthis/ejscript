#
#	Makefile -- Ejscript API Documentation
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

STAGING		:= staging

include		.makedep

gen:
	make -C ../../../package/dist dist
	rm -fr ./$(STAGING) xml html
	mkdir -p ./$(STAGING) xml html
	tar xf $(BLD_OUT_DIR)/releases/$(BLD_PRODUCT)-dist.tgz -C $(STAGING)
	mv $(STAGING)/ejs-$(BLD_VERSION)/* $(STAGING)
	rmdir $(STAGING)/ejs-$(BLD_VERSION)
	$(call log) "[Generate]" "doxygen ejs.spec"
	doxygen ejs.spec
	$(call log) "[Transform]" "gendoc"
	ejs $(BLD_TOOLS_DIR)/gendoc.es --bare --title "Ejscript Native API" --tags ../ejs.tags \
		xml/ejs_8h.xml xml/group* xml/struct_*.xml >../ejsBare.html
	ejs $(BLD_TOOLS_DIR)/gendoc.es --title "Ejscript Native API" --tags ../ejs.tags \
		xml/ejs_8h.xml xml/group* xml/struct_*.xml >../ejs.html
	rm -fr ./$(STAGING) xml html

cleanExtra:
	rm -rf xml html ../ejs.tags

clobberExtra:
	rm -f ../ejs.html ../ejsBare.html ../mpr.html
	rm -fr ./$(STAGING)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
