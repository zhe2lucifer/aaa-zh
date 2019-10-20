#!/bin/sh

# $1 $(packagename)
# $2 $(VERSION)
# $3 $(packagepath)


packagename=$1
packagepath=$3
export VERSION="$2"
export PACKAGE="$1"


DOXYGEN=doxygen
CHM=chmcmd

if [ $# -eq 0 ]; then
	PACKAGE='AUI'
	packagename=$PACKAGE
	packagepath="$(cd "$(dirname $0)"/../.. && pwd)"
	VERSION=$(awk -F "=" '/AUI_VERSION/ {print $2}' $packagepath/aui.cfg)

fi

DOXYGENCFG=$packagepath/doc/doxygen/doxygen.cfg


echo "packagepath is $packagepath"

echo "PACKAGE is $PACKAGE"
echo "VERSION is $VERSION"


export INPUT_SRC="$packagepath/inc"
echo "INPUT_SRC is $INPUT_SRC"

export OUTPUT_DIR="$packagepath/doc"
echo "OUTPUT_DIR is $OUTPUT_DIR"

export OUTPUT_SUBMOD_DIR=`basename $packagepath`
echo "OUTPUT_SUBMOD_DIR is $OUTPUT_SUBMOD_DIR"

export EXAMPLES="$packagepath/samples"
echo "EXAMPLES is $EXAMPLES"

# strip the file path in doxygen output
export STRIP_PATH=$INPUT_SRC


if [ "$(uname)" = "Darwin" ]; then
	# Do something under Mac OS X platform
	echo OSX
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
	# Do something under GNU/Linux platform
	echo Linux
	DOXYGEN=doxygen
	CHM=chmcmd
elif [ "$(expr substr $(uname -s) 1 6)" = "CYGWIN" ]; then
	# convert to windows path
	echo CYGWIN
	DOXYGENCFG="$(cygpath -w $packagepath/doc/doxygen/doxygen.cfg)"
	export INPUT_SRC="$(cygpath -w $packagepath/inc)"
	export OUTPUT_DIR="$(cygpath -w $packagepath/doc)"
	export STRIP_PATH="$(cygpath -w $INPUT_SRC)"
	export OUTPUT_SUBMOD_DIR=$OUTPUT_SUBMOD_DIR-$VERSION
	DOXYGEN=doxygen.exe
	CHM=hhc.exe
fi


cp -a Preface.txt $INPUT_SRC/
$packagepath/nestor/tools/$DOXYGEN $DOXYGENCFG 
cp -rf html_resource/*.* $OUTPUT_DIR/$OUTPUT_SUBMOD_DIR/
cd $OUTPUT_DIR/$OUTPUT_SUBMOD_DIR
$packagepath/nestor/tools/$CHM index.hhp
cd -
rm -f $INPUT_SRC/Preface.txt
chmod 644 $packagepath/doc/$OUTPUT_SUBMOD_DIR/*.chm
mv $packagepath/doc/$OUTPUT_SUBMOD_DIR/*.chm  $packagepath/doc/
