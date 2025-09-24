#!/bin/sh

if [ $# -gt 0 ]; then
    FILE="$1"
    shift
    if [ -f "$FILE" ]; then
        INFO="$(head -n 1 "$FILE")"
    fi
else
    echo "Usage: $0 <filename>"
    exit 1
fi

if command -v git >/dev/null 2>&1; then
    # Optionally allow specifying the repo dir via GIT_REPO_DIR (useful in containers)
    GIT_DIR_TO_USE="${GIT_REPO_DIR:-.}"
    if git -C "$GIT_DIR_TO_USE" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        # clean 'dirty' status of touched files that haven't been modified
        git -C "$GIT_DIR_TO_USE" diff >/dev/null 2>/dev/null 

        # get a string like "v0.6.0-66-g59887e8-dirty"
        DESC="$(git -C "$GIT_DIR_TO_USE" describe --dirty 2>/dev/null)"

        # get a string like "2012-04-10 16:27:19 +0200"
        TIME="$(git -C "$GIT_DIR_TO_USE" log -n 1 --format="%ci" 2>/dev/null)"
    fi
fi

# Fallbacks when git data isn't available (e.g., .git not mounted in container)
if [ -z "$DESC" ] && [ -n "$BUILD_DESC" ]; then
    DESC="$BUILD_DESC"
fi
if [ -z "$TIME" ]; then
    if [ -n "$BUILD_DATE" ]; then
        TIME="$BUILD_DATE"
    else
        TIME="$(date -u +"%Y-%m-%d %H:%M:%S +0000")"
    fi
fi

if [ -n "$DESC" ]; then
    NEWINFO="#define BUILD_DESC \"$DESC\""
else
    NEWINFO="// No build information available"
fi

# only update build.h if necessary
if [ "$INFO" != "$NEWINFO" ]; then
    echo "$NEWINFO" >"$FILE"
    echo "#define BUILD_DATE \"$TIME\"" >>"$FILE"
fi
