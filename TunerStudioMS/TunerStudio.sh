#!/bin/sh

ARCH=$( uname -m )
KERNEL=$( uname -s )
SCRIPT=$0

while [ -h "$SCRIPT" ]; do
	SCRIPT=$( readlink "$SCRIPT" )
done

BASE_PATH="${SCRIPT%/*}"
LAST_PATH="$PWD"

if [ ! -e "$BASE_PATH/TunerStudio.properties" ]; then
	echo "This script must be located in the TunerStudio directory. To be able"
	echo "to launch without path create a symlink of this script to a directory in PATH."
	echo
	echo "Example: ln -s /home/john_doe/TunerStudioMS/TunerStudio.sh /usr/local/bin"
	exit 1
fi

if [ "$KERNEL" = "Darwin" ]; then
	SERIAL_DRIVER="lib/alternateLinuxDrivers/OSX/:"
elif [ "$KERNEL" = "Linux" ]; then
	if [ "$ARCH" = "x86_64" ]; then
		SERIAL_DRIVER="lib/alternateLinuxDrivers/x86_64-linux:"
	fi
fi

cd "$BASE_PATH" && java -Dcom.ibm.crypto.provider.DoRSATypeChecking=false -Djava.library.path=${SERIAL_DRIVER}lib -cp ".:lib:plugins" -jar TunerStudioMS.jar $1
RESULT=$?

if [ -d "$LAST_PATH" ]; then
	cd "$LAST_PATH"
fi

exit $RESULT
