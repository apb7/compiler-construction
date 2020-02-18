#!/bin/bash
for i in {1..$3 }; do cat $1 >> $2; done
# chmod u+x generate_files.sh
# generate_files.sh file1 file2 4711
