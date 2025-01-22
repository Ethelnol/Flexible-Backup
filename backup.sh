#!/usr/bin/env bash

set -o errexit

export SOURCE=${BASH_SOURCE[0]}
export DIR=""
export missing=""

#stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script
#ty stackoverflow :P
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
	DIR="$( cd -P "$( dirname "$SOURCE" )" > /dev/null 2>&1 && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" > /dev/null 2>&1 && pwd )"

unset -v SOURCE

if [[ ! -f "$DIR/main.sh" ]]; then missing+="main.sh "; fi
if [[ ! -f "$DIR/scan_backup.sh" ]]; then missing+="scan_backup.sh "; fi
if [[ ! -f "$DIR/text_formatting.sh" ]]; then missing+="text_formatting.sh "; fi
if [[ ! -f "$DIR/conf.sh" ]]; then missing+="conf.sh "; fi


if [[ -n "$missing" ]]; then
	echo -n "Cannot find required dependenc"
	if (( $(echo "$missing" | wc -w) > 1 ))
		then echo -n "ies" 
		else echo -n "y"
	fi
	for i in $missing; do echo -n "\"$i\""; done
	echo -e "\b"

	exit "$(echo "$missing" | wc -w)"
fi
unset -v missing

source "$DIR/main.sh"

if ! SudoCheck; then exit $?; fi
unset -f SudoCheck

source "$DIR/scan_backup.sh"
source "$DIR/conf.sh"

if ! CheckConf; then exit $?; fi
unset -f CheckConf
unset -f ReadConf
unset -f MakeConf
unset -f ConfFormat

if ! CheckDependencies; then exit $?; fi
unset -f CheckDependencies

source "$DIR/text_formatting.sh"

if ! GetOpts "$@"; then exit 1; fi
unset -f GetOpts
unset -f OutputNeatly
unset -f DisplayHelp
unset -f HelpText
unset -v DIR

Start

exit $?