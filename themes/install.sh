#!/bin/bash

LOCAL_THEMES="$HOME/.asciidoc/themes"
OURDIR=`dirname $(realpath $0)`
MKDIR=`which mkdir`

if [ -z "$MKDIR" ]; then
	echo "no mkdir found" 1>&2
	exit 1
fi

$MKDIR -p "$LOCAL_THEMES" || exit 1
cd "$LOCAL_THEMES"
ln -sf "$OURDIR/suse" || exit 1
