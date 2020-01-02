OSSSM_ROOT := $(shell $(PWD)/root.sh)

root:
	echo $(OSSSM_ROOT)
.PHONY: root

env:
	$(shell $(PWD)/env.sh)
.PHONY: env
