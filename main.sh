#!/usr/bin/env bash

export outputDir=""
export backupRoot=""
export -a dirWhitelist=()

#whitelist and blacklist are assumed to be inside $backupDirRoot
export pvOpts=""
export dryRun=false
export noByteDU=false
export verbose=false
export quietLevel=0

#returns 0 if (whitelisted input is backed up) or (subdir of whitelisted input is backed up)
#returns 1 if (no whitelisted input is backed up) or (no subdir of whitelisted input is backed up)
Start(){
	local i
	local returnBool=false

	cd "$backupRoot" || exit 2

	#if ($OSTYPE is [macOS] )
	if [[ "${OSTYPE:0:6}" == "darwin" ]]; then noByteDU=true; fi

	for i in "${dirWhitelist[@]}"; do
		if Scan true 0 "$i"; then returnBool=true; fi
		
		cd "$backupRoot" || exit 2
	done

	if (( quietLevel < 2 )); then echo ""; fi
	echo -n "Backup complete"
	if $returnBool; then
		echo ""; sudo chown -R "$USER:$USER" "$outputDir"
		GetTotalSizeBackedUp

		tput cnorm
		return 0
	else
		echo ": No backups made"

		tput cnorm
		return 1
	fi
}

#returns 0 if (no options are passed) or (only valid options are passed)
#returns 1 if ("-h" or "--help" is passed) or (an unknown option is passed)
#param $* are arr elements
GetOpts(){
	local i
	local -i j

	local -a tempArr=()
	local -a optsArr=()
	
	read -ra tempArr <<< "$*"

	for i in "${tempArr[@]}"; do
		if [[ "$i" =~ ^[-]{2,} ]]; then
			while [[ "$i" =~ ^[-]{3} ]]; do
				i="${i:1}"
			done
			optsArr+=("$i")

		elif [[ "$i" =~ ^[-]{1,} ]] && (( ${#i} > 2 )); then
			for (( j = 1; j < ${#i}; j++ )); do
				optsArr+=("-${i:$j:1}")
			done
		else
			optsArr+=("$i")
		fi

	done

	for i in "${optsArr[@]}"; do
		case $i in
			-h|--help)
				DisplayHelp
				return 1
				;;
			-q|--quiet*)
				if (( ${#i} == 2 )); then
					quietLevel=$(( quietLevel + 1 ))
				elif [[ "${i:7}" =~ ^=[0-9]{1,}$ ]]; then
					quietLevel=$(( quietLevel + ${i:8} ))
				else
					echo -n "Option \"--quiet=\" "
					if [[ "${i:7:1}" != "=" ]] || [[ -z "${i:8}" ]]
						then echo "requires input"
						else echo "only takes integer numbers -- \"${i:8}\""
					fi
					return 1
				fi

				;;
			-s|--simulate|--just-print|--dry-run)
				dryRun=true
				;;
			-v|--verbose)
				verbose=true
				;;
			*)
				if [[ "$i" == "-" ]]; then
					echo "Option not provided -- \"-?\""
				elif [[ "$i" =~ ^[-]{1,} ]]; then
					echo "Option not recognised -- \"$i\""
				else
					echo "Argument not recognised -- \"$i\""
				fi

				return 1
				;;
		esac
	done

	#if ($quietLevel > 0) and ($verbose == true)
	if (( quietLevel )) && $verbose; then
		echo "Cannot pass -v|--verbose and -q|--quiet=NUM"
		return 1
	fi

	if (( quietLevel > 1 ))
		then pvOpts="--quiet"
	elif $verbose
		then pvOpts="--format '%t %e %p %b %r'"
		else pvOpts="--format '%e %p'"
	fi

	return 0
}

#returns number of missing dependencies
CheckDependancies(){
	#if (specific binary does not exist)
	if [[ ! -f /usr/bin/tar ]]; then missing+="tar "; fi
	if [[ ! -f /usr/bin/gzip ]]; then missing+="gzip "; fi
	if [[ ! -f /usr/bin/pv ]]; then missing+="pv "; fi

	#if (binaries are missing) or (local files are missing)
	if [[ -n "$missing" ]]; then
		echo -n "Missing dependenc"
		if (( $(echo "$missing" | wc -w) > 1 ))
			then echo -n "y: "
			else echo -n "ies: "
		fi
		for i in $missing; do echo -n "\"$i\" "; done
		echo -e "\b"
	fi

	return "$(echo "$missing" | wc -w)"
}

#returns 0 if (user is root) or (user executes sudo)
#returns 1 if (user cannot execute sudo) or (user does not execute sudo)
SudoCheck(){
	if [[ "$(whoami)" != "root" ]]; then
		sudo -l &> /dev/null && sudo echo -n "" && return 0

		echo -e "Error: requires sudo privileges\n"; return 1
	fi
	return 0
}

# shellcheck disable=SC2317
SIGINT_Trap(){
	echo "Program canceled"
	if [[ -n "$lastBackup" ]] && ! $dryRun; then
		if [[ -f "$lastBackup" ]]; then
			echo -e "\tCleaning up \"$lastBackup\""
			sudo rm "$lastBackup"
		fi
		echo "Finishing up..."
	fi

	sudo chown -R "$USER:$USER" "$outputDir"
	
	GetTotalSizeBackedUp

	tput cnorm
}

#has program handle SIGINT itself
trap 'SIGINT_Trap;exit 1' SIGINT