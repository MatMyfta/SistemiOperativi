#!/bin/bash

ALLOWED_HEADERS=(
assert.h ctype.h errno.h fcntl.h float.h limits.h math.h pthread.h signal.h
stddef.h stdio.h stdlib.h string.h sys/ipc.h sys/msg.h sys/shm.h sys/stat.h
sys/types.h sys/wait.h time.h unistd.h termios.h
)

# transform array into string where each element is separated by | (e.g. "assert.h|ctype.h|...")
HEADERS_PATTERN=$(echo ${ALLOWED_HEADERS[@]} | tr " " "|")
grep --color -E "#.*include.*<.*>" -r src/ | grep -v -E "#.*include.*<(${HEADERS_PATTERN})>"

if [[ $? -eq 1 ]]; then
  echo "Everything OK"
fi
