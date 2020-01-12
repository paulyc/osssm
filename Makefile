OSSSM_ROOT := $(shell $(PWD)/root.sh)
BUILD_DIR := build
INSTALL_DIR := install
CMAKE := cmake
CMAKE_FLAGS := -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)
MAKE := make
MAKE_FLAGS := -j1

all: build

renderosm: renderosm.cpp
	g++ -o renderosm renderosm.cpp `mapnik-config --all-flags`

cmake: cmake_proj

build: make_proj

install: install_proj

cmake_proj:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ../PROJ/
.PHONY: cmake_proj

make_proj: cmake_proj
	cd $(BUILD_DIR) && $(MAKE) $(MAKE_FLAGS)

install_proj: make_proj
	cd $(BUILD_DIR) && $(MAKE) install

clean:
	rm -rf $(BUILD_DIR) $(INSTALL_DIR)
.PHONY: clean

root:
	echo $(OSSSM_ROOT)
.PHONY: root

env:
	$(shell $(PWD)/env.sh)
.PHONY: env
