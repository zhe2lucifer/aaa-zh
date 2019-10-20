#!/bin/bash
platform="$1"
module="$2"
ota="$3"
DDK_ROOT_DIR=$PWD
if [ "$platform" = "tds" ]
then

	if [ "$module" = "aui" ]
	then
		echo "******************Compile TDS aui******************"
		make aui_clean
		make aui
		cp ./output/tds/libaui.a ../ddk/
	fi
	if [ "$module" = "aui_test" ]
	then
		echo "******************Compile TDS aui samples******************"
		make aui_test_clean
		make aui_test
		cp ./output/tds/libaui_test.a ../ddk/
	fi
	
	if [ "$module" = "nestor" ]
	then
	    echo "******************Compile TDS nestor framework*************"
	    make aui_nestor_clean
	    make aui_nestor
	    cp ./nestor/targetapp/libnestor.a ../ddk/
	fi

	if [ "$module" = "nestor_sym" ]
	then
	    echo "******************Compile TDS nestor framework*************"
	    make aui_nestor_clean
	    make -C ./nestor/targetapp build_sym
	    cp ./nestor/targetapp/libnestor.a ../ddk/
	fi
	
fi	
