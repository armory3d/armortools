MAKEFLAGS += --silent

.PHONY: all
all:
	node Kromx/make -g opengl
	cd Kromx && node Kinc/make -g opengl --compiler clang --compile
	strip Kromx/Deployment/Krom

.PHONY: clean
clean:
	rm -rf build
	git submodule foreach --recursive git clean -dfx
