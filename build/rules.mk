
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $(MAKECMDGOALS) || exit "$$?"; \
	done

clean-subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $(MAKECMDGOALS) || exit "$$?"; \
	done

install-subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $(MAKECMDGOALS) || exit "$$?"; \
	done

tds_check:
	@echo "Check TDS compilation"

.PHONY: subdirs clean-subdirs install-subdirs tds_check