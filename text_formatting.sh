#!/usr/bin/env bash

export dryRun=false
export noByteDU=false

export lastBackup=""
export lastBackupSize=0
export totalSize=0
export decimalOfTotalSize=0
export sigFigs=2

#$1 is int - num
#$2 is str - str
EchoStr(){
	local -i num=$1
	local str="$2"

	local -i i
	
	for (( i = 0; i < num; i++ )); do echo -ne "$str"; done
}

#$1 is bool - centered
#$2 is int - left
#if $1; then $3 is int
#$* are arr elements - arr
OutputNeatly(){
	local centered=$1
	local -i left=$2; shift 2
	local -a arr=()

	local -i maxArea=0
	local outputStr=""
	local tempStr=""
	local -i spaceInt=0

	local -i i=0
	local -i j=0

	if $centered; then
		maxArea=$(( COLUMNS - ( left * 2 ) ))
	else
		maxArea=$(( COLUMNS - ( left + $1 ) )); shift
		spaceInt=$left
	fi

	IFS=' ' read -ra arr <<< "$*"
	if (( maxArea < 45 )); then maxArea=$COLUMNS; fi

	while (( i < ${#arr[@]} )); do
		tempStr="${arr[$i]}"

		#while (i < arrLen) and ( (outputStrLen + 1 + tempStrLen) < maxArea )
		while (( i < ${#arr[@]} )) && (( ( ${#outputStr} + 1 + ${#tempStr} ) < maxArea )); do
			outputStr+="$tempStr "
			i=$(( i + 1 ))
			tempStr="${arr[$i]}"
		done

		if [[ -n "$outputStr" ]]; then
			outputStr=$(echo -e "$outputStr\b")

			if $centered; then
				spaceInt=$(( ( ( left * 2 ) + maxArea - ${#outputStr} ) / 2 ))
			fi

			for (( j=0; j<spaceInt; j++ )); do echo -n " "; done
			echo "$outputStr"
			
			outputStr=""
		fi
	done
}

DisplayHelp(){
		OutputNeatly true 5 "Flexible Backup"
	echo ""
		OutputNeatly true 5 "$(HelpText "description")"
			
	echo -e "Options:\n"

	echo "    -h, --help:"
		OutputNeatly false 10 5 "$(HelpText "help")"
	echo ""
	echo "    -q, --quiet=NUM:"
		OutputNeatly false 10 5 "$(HelpText "quiet")"		
	echo ""
	echo "    -s, --simulate, --dry-run, --just-print:"
		OutputNeatly false 10 5 "$(HelpText "simulate")"
	echo ""
	echo "    -v, --verbose:"
		OutputNeatly false 10 5 "$(HelpText "verbose")"	
	echo ""
}

#$1 is str
HelpText(){
	case "$1" in
		"description")
			echo -n "Creates backups more representative of the system "
			echo -n "at time of backup by only backing up updated files"
			echo -n " and directories"
			;;
		"help")
			echo -n "Displays this message."
			;;
		"quiet")
			echo -n "Silences program in levels increased by passing mu"
			echo -n "ltiple instances of '-q' and/or '--quiet=NUM'. Lev"
			echo -n "el 1 silences scanning/skipping message for non-di"
			echo -n "rectory source, message when source is blacklisted"
			echo -n ", and announcement of when source is about to be b"
			echo -n "acked up. Level 2 or higher silences every message"
			echo -n " and causes the program to make no output. Cannot "
			echo -n "be passed with -v or --verbose."
			;;
		"simulate")
			echo -n "Disable backing up and removal of outdated package"
			echo -n "s, while still giving an estimated readout of size"
			echo -n " of the backup"
			;;
		"verbose")
			echo -n "Increases verbosity of output. Gives bitrate, tota"
			echo -n "l amount transferred, and timer with the compressi"
			echo -n "on progress bar and states reason for why specific"
			echo -n " source was skipped. Can not be passed with -q or "
			echo -n "--quiet=NUM."
			;;
	esac
}

GetTotalSizeBackedUp(){
	local -i i=0
	local -i int=0
	local units=""

	if $noByteDU; then int=1; fi

	if (( lastBackupSize )); then totalSize=$(( totalSize - lastBackupSize )); fi

	while (( totalSize >= 1024 )); do
		decimalOfTotalSize=$(( totalSize % 1024 ))
		totalSize=$(( totalSize / 1024 ))
		int=$(( int + 1 ))
		if (( int == 3 )); then break; fi
	done

	decimalOfTotalSize=${decimalOfTotalSize:0:$sigFigs}

	case $int in
		0) units=""       ;;
		1) units="kilo"   ;;
		2) units="mega"   ;;
		3) units="giga"   ;;
		4) units="tera"   ;;
		5) units="peta"   ;;
		6) units="exa"    ;;
		*) units="alotta" ;;
	esac

	echo -n "Roughly $totalSize"
	if (( decimalOfTotalSize )); then
		echo -n ".${decimalOfTotalSize:0:$sigFigs}"

		for (( i = 0; i < $(( sigFigs - ${#decimalOfTotalSize} )); i++ )); do
			echo -n "0"
		done

	fi

	echo -n " $units""byte"
	if (( totalSize + decimalOfTotalSize > 1 )) || (( ! totalSize + decimalOfTotalSize )); then
		echo -n "s"
	fi
	if $dryRun; then echo -n " would have been"; fi
	echo " backed up"
}