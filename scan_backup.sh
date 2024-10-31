#!/usr/bin/env bash

export fiveGigabytes=$(( 5 * (2**30) ))

export outputDir=""
export backupRoot=""
export -a dirBlacklist=()
export -a requiredScan=()
export archiveType="gz"

#whitelist and blacklist are assumed to be inside $backupDirRoot
export pvOpts=""
export dryRun=false
export noByteDU=false
export verbose=false
export quietLevel=0

export lastBackup=""
export lastBackupSize=0

#param $1 is bool
#param $2 is int
#param $* are arr elements
#returns 0 if (input is backed up) or (any subdir of input is backed up)
#returns 1 if (input is blacklisted) or (input not backed up) or (no subdir of input is backed up)
Scan(){
	local checkSize=$1
	local -i tabCount=$2; shift 2
	local i="$*"

	local j

	if [[ "$PWD" == "/" ]]
		then i="/$i"
		else i="$PWD/$i"
	fi

	if (( quietLevel < 2 )); then
		if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" ]]; then
			EchoStr $tabCount "\t"
			echo -n "$(basename "$i"): "
			echo -n "Scanning"
		fi
	fi

	#if (blacklisted)
	for j in "${dirBlacklist[@]}"; do
		#${i:1:SizeOf$j} == $j
		if [[ "${i:1:${#j}}" == "$j" ]]; then
			if (( quietLevel < 2)); then
				if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" ]]; then
					EchoStr 8 "\b"; echo -n "Skipping"
				fi
				if $verbose; then echo -n ": blacklisted"; fi
				echo ""
			fi

			return 1
		fi
	done

	#if ($i is a file)
	if [[ -f "$i" ]]; then
		Backup $tabCount "$i"
		return $?
	fi

	#if (told to check size)
	#if ($checkSize == true)
	if $checkSize; then
		#if (equal or larger than 5g)
		if (( $(CheckSize "$i") > fiveGigabytes )); then
			if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" && "$quietLevel" -lt "2" ]]; then
				local returnBool=false

				echo -e " subentries\n"
				DeeperScan true $tabCount "$i" && returnBool=true
				echo ""

				if $returnBool
					then return 0
					else return 1
				fi
			fi

			DeeperScan true $tabCount "$i"
			
			return $?
		fi
	fi

	#if (has blacklisted subdir)
	for j in "${dirBlacklist[@]}"; do
		#/${j:0:SizeOf$i} == $i
		if [[ "/${j:0:$(( ${#i} - 1 ))}" == "$i" ]]; then
			if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" && "$quietLevel" -lt "2" ]]; then
				local returnBool=false

				echo -e " subentries\n"
				DeeperScan true $tabCount "$i" && returnBool=true
				echo ""

				if $returnBool
					then return 0
					else return 1
				fi
			fi

			DeeperScan true $tabCount "$i"
			
			return $?
		fi
	done

	#if (is requiredScan dir) or (has requiredScan subdir)
	for j in "${requiredScan[@]}"; do
		#/${j:0:SizeOf$i} == $i
		if [[ "/${j:0:$(( ${#i} - 1 ))}" == "$i" ]]; then
			if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" && "$quietLevel" -lt "2" ]]; then
				local returnBool=false
				
				echo -e " subentries\n"
				DeeperScan true $tabCount "$i" && returnBool=true
				echo ""

				if $returnBool
					then return 0
					else return 1
				fi
			fi

			DeeperScan true $tabCount "$i"
			
			return $?
		fi
	done

	Backup $tabCount "$i"
	return $?
}

#param $1 is bool
#param $2 is int
#param $* are arr elements
#returns 0 if (any subdir is backed up)
#returns 1 if (no subdir is backed up)
DeeperScan(){
	local checkSize=$1; shift
	local -i tabCount=$1; shift
	local i="$*"

	local j
	local returnBool=false
	local -a curdirSubdirs 

	cd "$i" || exit 2

	set -o noglob
	mapfile -t curdirSubdirs < <(sudo ls -a)

	for j in "${curdirSubdirs[@]}"; do
		if [[ "$j" == "." ]] || [[ "$j" == ".." ]]; then continue; fi
		
		if Scan "$checkSize" $((tabCount+1)) "$j"; then returnBool=true; fi

		cd "$i" || exit 2
	done
	
	if $returnBool;
		then return 0
		else return 1
	fi
}

#param $1 is str
CheckSize(){
	#if (noByteDU == true)
	if $noByteDU
		then echo "$(( $(sudo du -sk "$1" | awk '{print $1}') * 1024 ))"
		else sudo du -sb "$1" | awk '{print $1}'
	fi
}

#param $1 is int
#param $2 is str
#returns 0 if (runs backup)
#returns 1 if (does not run backup)
Backup(){
	local -i tabCount=$1
	local i="$2"
	local -i j=0

	#if destination archive (doesn't exist) or (is older than the source)
	if WriteOrNot "$i"; then
		if (( quietLevel < 2 )); then
			if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" ]]; then
				EchoStr 8 "\b"
				echo "Backing up"
			fi
		fi

		#if (archive exists)
		if [[ -f "$outputDir$i.tar.$archiveType" ]] && ! $dryRun; then
			sudo rm "$outputDir$i.tar.$archiveType"
		elif [[ -d "$outputDir$i" ]] && ! $dryRun; then
			sudo rm -r "$outputDir$i"
		fi
		
		#if ($i isn't in backupRoot)
		if [[ "$(dirname "$i")" != "$backupRoot" ]] && ! $dryRun; then
			sudo mkdir -p "$outputDir$(dirname "$i")" 2> /dev/null
		fi

		Compress "$i"

		#if ($quietLevel == 0) or ($quietLevel == 1)
		if (( quietLevel < 2 )); then
			if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" ]]; then
				echo -ne "\e[1A\e[K\e[1A\e[K"
				EchoStr $tabCount "\t"
				echo "$(basename "$i"): Backed up"
			fi
		fi
		return 0
	fi

	if (( quietLevel < 2)); then
		if [[ -f "$i" && "$quietLevel" -eq "0" ]] || [[ -d "$i" ]]; then
			EchoStr 8 "\b"; echo -n "Skipping"
			if $verbose; then echo -n ": no changes detected. "; fi
			echo ""
		fi
	fi

	return 1
}

#param $1 is str
#returns 0 if (destination archive does not exist) or (destination archive is older than $source)
#returns 1 if (destination archive is younger than $source)
WriteOrNot(){
	local source="$1"

	if [[ ! -f "$outputDir$source.tar.$archiveType" ]]
		then return 0
	fi

	local -i fileTime
	fileTime="$(date -r "$source" "+%Y%m%d%H%M%S")"

	local -i archiveTime
	archiveTime="$(date -r "$outputDir$source.tar.$archiveType" "+%Y%m%d%H%M%S")"

	if (( fileTime > archiveTime));
		then return 0
		else return 1
	fi
}

#param $1 is str
Compress(){
	local i="$1"
	local -i size; size=$(CheckSize "$i")

	totalSize=$(( totalSize + size ))
	lastBackupSize=$size

	if $dryRun; then return; fi

	lastBackup="$outputDir$i.tar.$archiveType"

	sudo tar --absolute-names --directory "$(sudo dirname "$i")" --create --file - "$(sudo basename "$i")" 2> /dev/null |\
	eval "pv $pvOpts --size $size" |\
	sudo gzip --keep --quiet --best --stdout 2> /dev/null |\
	sudo tee "$outputDir$i.tar.$archiveType" &> /dev/null

	lastBackup=""
	lastBackupSize=0
}