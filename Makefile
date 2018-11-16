
#---------------------------------------
# RULES
#---------------------------------------
include Make.properties

all:
	(cd $(SRCDIR) && make -f $(MAKEFILE)) || exit 1;
bin:
	(cd $(SRCDIR) && make -f $(MAKEFILE) bin) || exit 1;
clean:
	(cd $(SRCDIR) && make -f $(MAKEFILE) clean) || exit 1;
