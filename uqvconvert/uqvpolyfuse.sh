#!/bin/bash

# uqvpolyfuse.sh: accepts many UQV files to fuse.
# Export POLYPARAM variable with the polyfuse parameters you would like.

#POLYPARAM="rrf -k 60 -d 1000"
POLYPARAM="combsum -n minmax -d 1000"
#POLYPARAM="rbc -p 0.995"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm -rf $DIR/tmp
rm -rf $DIR/out.txt
mkdir -p $DIR/tmp

# We want to ensure each file has a unique identifier
FCOUNTER=1


# Build nums file. If you have different topics for each UQV file, you
# absolutely MUST comment this out.
cut -d"-" -f1 $1 | sort | uniq > $DIR/nums

for uqvfile in "$@"
do
    for topic in $(cat $DIR/nums)
    do
        mkdir -p $DIR/tmp/$topic
    done

    # We want this to bucket the topic in the correct dir
    # And, we also want it to make sure it's not conflicting with the same ID
    # Which might happen if we were fusing different systems in here as well.
    echo "$DIR"
    awk -v fcounter=$FCOUNTER -v dir=$DIR '{ split($1, topic, "-"); print topic[1]" "$2" "$3" "$4" "$5" "$6 > dir"/tmp/"topic[1]"/"$1"-"fcounter; }' $uqvfile
    FCOUNTER=$(($FCOUNTER + 1))
done

for folder in $DIR/tmp/*
do
    $DIR/polyfuse $POLYPARAM $folder/* >> $DIR/out.txt
done
