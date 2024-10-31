#!/usr/bin/env bash

set -o errexit

declare SOURCE=${BASH_SOURCE[0]}
declare DIR=""
declare returnInt=0

#stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script
#ty stackoverflow :P
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
	DIR="$( cd -P "$( dirname "$SOURCE" )" > /dev/null 2>&1 && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" > /dev/null 2>&1 && pwd )"

if [[ ! -f "$DIR/main.sh" ]]; then
	echo "Cannot find main file, cancelling"
	exit 1
fi

source "$DIR/main.sh"
source "$DIR/scan_backup.sh"
source "$DIR/conf.sh"
source "$DIR/text_formatting.sh"

tput civis

SudoCheck &&\
CheckDependancies &&\
CheckConf &&\
GetOpts "$@" &&\
Start
returnInt=$?

tput cnorm

exit $returnInt