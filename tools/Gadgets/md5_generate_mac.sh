#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

SOURCE_BIN="$1"
SOURCE_DIR=`dirname $SOURCE_BIN`
SOURCE_BASE_NAME=`basename  $SOURCE_BIN`

SOURCE_NAME=`echo $SOURCE_BASE_NAME|cut -d '.' -f1`
VERSION_FILE="$2"
# echo "source bin: "$SOURCE_BIN
# echo "source dir: "$SOURCE_DIR
# echo "source base name: "$SOURCE_BASE_NAME
# echo "source name: "$SOURCE_NAME
# if called by cmake, should cut 1-32
# if called by windows bat, should cut 2-33
MD5_STR=$(python3 "${SCRIPT_DIR}/rename_with_md5.py" "$SOURCE_BIN")

# echo $MD5_STR

FILE_CONTENT=
for LINE in `head -n 6 $VERSION_FILE`
do
    FILE_CONTENT=$FILE_CONTENT%$LINE
done
#echo $FILE_CONTENT

MAJOR=`echo $FILE_CONTENT | awk -F '%' '{print $4}'`
MINOR=`echo $FILE_CONTENT | awk -F '%' '{print $7}'`
REVISION=`echo $FILE_CONTENT | awk -F '%' '{print $10}'`
BUILDNUM=`echo $FILE_CONTENT | awk -F '%' '{print $13}'`
GCID=`echo $FILE_CONTENT | awk -F '%' '{print $19}' | cut -c 3-`
#CUSTOMER_NAME=`echo $FILE_CONTENT | awk -F '%' '{print $16}'`
# echo $MAJOR
# echo $MINOR
# echo $REVISION
# echo $BUILDNUM
# echo $GCID
# echo $MD5_STR
# echo $CUSTOMER_NAME

IMAGE_NAME=$SOURCE_DIR/$SOURCE_NAME-v$MAJOR.$MINOR.$REVISION.$BUILDNUM-$GCID-$MD5_STR.bin
# echo "image name: "$IMAGE_NAME
# echo "SOURCE_BIN: "$SOURCE_BIN
mv $SOURCE_BIN $IMAGE_NAME
#rm -f "./bin/"$1".bin"
#git checkout $VERSION_FILE
