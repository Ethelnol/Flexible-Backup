#!/usr/bin/env bash

export maxSize=$(( ( 5 * (2**30) ) - 1 ))

export outputDir=""
export backupRoot=""
export dirBlacklist=()
export requiredScan=()
export archive=()
export archiveExt=""
export archiveOpts=()
export archiveExtensions=()

#whitelist and blacklist are assumed to be inside $backupDirRoot
export pvOpts=""
export dryRun=false
export noByteDU=false
export verbose=false
export quietLevel=0

export lastBackup=""
export lastBackupSize=0

#$1 is int - tabCount
#$2 is bool - checkSize
#$3 is str - i
#returns 0 if (input is backed up) or (any subdir of input is backed up)
#returns 1 if (input is blacklisted) or (input not backed up) or (no subdir of input is backed up)
Scan(){
	local -i tabCount=$1
	local checkSize=$2
	local i="$3"

	local isFile=false

	[[ -f "$i" ]] && isFile=true

	if (( quietLevel < 2 )); then
		if ! $isFile || (( ! quietLevel ));  then
			EchoStr $tabCount "\t"
			echo -n "$(basename "$i"): "
			echo -n "Scanning"
		fi
	fi

	IsInArr "$i" "${dirBlacklist[@]}"
	if (( $? == 1 )); then
		if (( quietLevel < 2 )); then
			if ! $isFile || (( ! quietLevel ));  then
				EchoStr 8 "\b"; echo -n "Skipping"
			fi
			if $verbose && [[ "$i" == "$outputDir" ]]
				then echo -n ", won't backup output directory"
				else echo -n ", blacklisted"
			fi
			echo ""
		fi
		return 1
	fi

	if $isFile; then Backup $tabCount $isFile 0 "$i"; return $?; fi

	IsInArr "$i" "${dirBlacklist[@]}"
	if (( $? == 2 )) || (( $(IsInArr "$i" "${requiredScan[@]}") )); then
		local -a dirEntry=()

		local j

		for j in "$i/"*; do 
			if [[ "$(basename "$j")" == "*" ]]; then continue; fi
			dirEntry+=("$j")
		done
		for j in "$i/".*; do
			if [[ "$(basename "$j")" == ".*" ]] ||\
			   [[ "$j" == "." ]] || [[ "$j" == "." ]]; then continue; fi
			dirEntry+=("$j")
		done
		
		unset j

		if (( quietLevel < 2 )); then
			if ! $isFile || (( ! quietLevel ));  then
				local -i returnInt=1

				echo -e " subentries\n"
				DeeperScan $tabCount "$checkSize" "${dirEntry[@]}" && returnInt=0
				echo ""

				return $returnInt
			fi
		fi
		
		unset isFile

		DeeperScan $tabCount "$checkSize" "${dirEntry[@]}"
		
		return $?
	fi

	local -i size=0

	if $checkSize; then
		size=$(CheckSize "$i")
		if (( size > maxSize ))
			then checkSize=true
			else checkSize=false
		fi
	fi

	if $checkSize; then
		unset checkSize
		unset isFile

		local -a dirEntry=()

		local j

		for j in "$i/"*; do 
			if [[ "$(basename "$j")" == "*" ]]; then continue; fi
			dirEntry+=("$j")
		done
		for j in "$i/".*; do
			if [[ "$(basename "$j")" == ".*" ]] ||\
			   [[ "$j" == "." ]] || [[ "$j" == "." ]]; then continue; fi
			dirEntry+=("$j")
		done
		
		unset j

		if (( quietLevel < 2 )); then
			if ! $isFile || (( ! quietLevel ));  then
				local returnBool=false

				echo -e " subentries\n"
				DeeperScan $tabCount true "${dirEntry[@]}" && returnBool=true
				echo ""

				if $returnBool
					then return 0
					else return 1
				fi
			fi
		fi
	
		DeeperScan $tabCount true "${dirEntry[@]}"

		return $?
	else
		Backup $tabCount $isFile $size "$i"
	fi
	
	return $?
}

#$1 is int - tabCount
#$2 is bool - checkSize
#$@ are arr elements
#returns 0 if (any subdir is backed up)
#returns 1 if (no subdir is backed up)
DeeperScan(){
	local -i tabCount=$1
	local checkSize=$2; shift 2

	local -i returnInt=1

	for j in "$@"; do
		if [[ "$j" == "." ]] || [[ "$j" == ".." ]]; then continue; fi
		
		# shellcheck disable=SC2086
		Scan $(( tabCount + 1 )) "$checkSize" "$j" && returnInt=0
	done

	return $returnInt
}

#$1 is str
CheckSize(){
	#if (noByteDU == true)
	if $noByteDU
		then echo "$(( $(sudo du -sk "$1" | awk '{print $1}') * 1024 ))"
		else sudo du -sb "$1" | awk '{print $1}'
	fi
}

#$1 is str - i
#$@ are strs
#returns 0 if not present in arr
#returns 1 if is present in arr
#returns 2 if subentry is present in arr
IsInArr(){
	local i="$1"; shift 1

	local j=""

	for j in "$@"; do
		if   [[ "${i:0:${#j}}" == "$j" ]]; then return 1
		elif [[ "${j:0:${#i}}" == "$i" ]]; then return 2
		fi
	done

	return 0
}

#$1 is int - tabCount
#$2 is bool - isFile
#$3 is int - size
#$4 is str - i
#returns 0 if (runs backup)
#returns 1 if (does not run backup)
Backup(){
	local isFile=$2
	local i="$4"

	#if destination archive (doesn't exist) or (is older than the source)
	if NewArchiveNeeded "$i"; then
		local -i tabCount=$1

		if (( quietLevel < 2 )); then
			if ! $isFile || (( ! quietLevel ));  then
				EchoStr 8 "\b"
				echo "Backing up"
			fi
		fi


		if ! $dryRun; then
			local -i size=$3

			sudo mkdir -p "$outputDir$(dirname "$i")"
			
			#remove preexisting backups
			if [[ -d "$outputDir$i" ]]; then
				sudo rm -r "$outputDir$i"
			else
				local j="$i"
				local k

				#removes old archive, even if archive is higher in directory tree
				while [[ "$j" != "/" ]]; do
					for k in "${archiveExtensions[@]}"; do	
						if [[ -f "$outputDir$j.tar.$k" ]]; then
							sudo rm "$outputDir$j.tar.$k"
							break 2
						fi
					done
					j="$(dirname "$j")"
				done

				unset j
				unset k
			fi

			Compress "$isFile" "$size" "$i" 

			unset size
		fi

		if (( quietLevel < 2 )); then
			if ! $isFile || (( ! quietLevel ));  then
				echo -ne "\e[1A\e[K"
				if ! $dryRun; then echo -ne "\e[1A\e[K"; fi
				EchoStr $tabCount "\t"
				echo "$(basename "$i"): Backed up"
			fi
		fi
		return 0
	fi

	if (( quietLevel < 2 )); then
		if ! $isFile || (( ! quietLevel ));  then
			EchoStr 8 "\b"; echo -n "Skipping"
			if $verbose; then echo -n ", no changes detected. "; fi
			echo ""
		fi
	fi

	return 1
}

#$1 is str
#returns 0 if (destination archive does not exist) or (destination archive is older than $source)
#returns 1 if (destination archive is younger than $source)
NewArchiveNeeded(){
	[[ ! -f "$outputDir$1.tar.$archiveExt" ]] && return 0

	local -i fileTime=0
	local -i archiveTime=0
	
	fileTime=$(date -r "$1" "+%Y%m%d%H%M%S")

	archiveTime=$(date -r "$outputDir$1.tar.$archiveExt" "+%Y%m%d%H%M%S")

	if (( fileTime > archiveTime));
		then return 0
		else return 1
	fi
}

#$1 is int - isFile
#$2 is int - size
#$3 is str - i
Compress(){
	local -i isFile=$1
	local -i size=$2
	local i="$3"

	local tempOpts=""

	if (( ! size )); then size=$(CheckSize "$i"); fi

	lastBackup="$outputDir$i.tar.$archiveExt"
	lastBackupSize=$size
	totalSize=$(( totalSize + size ))

	if (( quietLevel > 0 ))
		then if $isFile || (( quietLevel > 1 )); then tempOpts="--quiet"; fi
		else tempOpts="$pvOpts"
	fi

	sudo tar --absolute-names --directory "$(sudo dirname "$i")" --create --file - "$(sudo basename "$i")" 2>/dev/null |\
	eval "pv $tempOpts --size $size" |\
	"${archive[@]}" 2>/dev/null |\
	sudo tee "$outputDir$i.tar.$archiveExt" &> /dev/null

	lastBackup=""
	lastBackupSize=0
}