#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for f in $(find "$DIR" -name '*.h' -or -name '*.hpp' -or -name '*.cpp'); do
    clang-format -i "$f"
done
