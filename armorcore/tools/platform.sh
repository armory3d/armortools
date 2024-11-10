if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	MACHINE_TYPE=`uname -m`
	if [[ "$MACHINE_TYPE" == "aarch64"* ]]; then
		KINC_PLATFORM=linux_arm64
	elif [[ "$MACHINE_TYPE" == "x86_64"* ]]; then
		KINC_PLATFORM=linux_x64
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	KINC_PLATFORM=macos
fi
