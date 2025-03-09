if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	MACHINE_TYPE=`uname -m`
	if [[ "$MACHINE_TYPE" == "aarch64"* ]]; then
		IRON_PLATFORM=linux_arm64
	elif [[ "$MACHINE_TYPE" == "x86_64"* ]]; then
		IRON_PLATFORM=linux_x64
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	IRON_PLATFORM=macos
fi
