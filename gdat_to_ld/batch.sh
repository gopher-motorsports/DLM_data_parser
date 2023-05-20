#!/bin/sh

dir=$1
if [ ! -d $dir ]; then
    echo "$dir is not a directory"
    exit
fi

for filepath in $dir/*; do
    filename=$(basename $filepath)
    echo "CONVERTING $filename"
    ./ld_converter $filepath
done

echo "Done."