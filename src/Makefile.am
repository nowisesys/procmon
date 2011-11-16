bin_PROGRAMS = procmon
procmon_SOURCES = main.c procmon.c procmon.h procdisp.h procdisp.c

man_MANS = procmon.1 procmond.8

install-exec-hook:
	$(MKDIR_P) $(DESTDIR)$(sbindir)
	$(LN_S)	$(bindir)/program$(EXEEXT) \
		$(DESTDIR)$(sbindir)/program$(EXEEXT)d
