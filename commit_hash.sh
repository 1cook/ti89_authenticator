#!/bin/bash
echo "#define COMMIT_HASH \"" > bin/commit_hash.h
git show -s --format=%H | head -c 20 >> bin/commit_hash.h
echo "...\"" >> bin/commit_hash.h
