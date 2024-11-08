#!/usr/bin/env bash

export fiveGigabytes=$(( 5 * (2**30) ))

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

#param $1 is bool
#param $2 is int
#param $* are arr elements
#returns 0 if (input is backed up) or (any subdir of input is backed up)
#returns 1 if (input is blacklisted) or (input not backed up) or (no subdir of input is backed up)
Scan(){
	local checkSize=$1
	local -i tabCount=$2; shift 2
	local i="$*"

	local -i isFile
	local j

	#using -d instead of -f so (( ! isFile )) naturally reads as (( not isFile ))
	[[ -d "$i" ]]
	isFile=$?

	if (( quietLevel < 2 )); then
		if (( ! isFile )) || (( isFile && ! quietLevel )); then
			EchoStr $tabCount "\t"
			echo -n "$(basename "$i"): "
			echo -n "Scanning"
		fi
	fi

	#if (blacklisted)
	for j in "${dirBlacklist[@]}"; do
		#${i:1:SizeOf$j} == $j
		if [[ "${i:0:${#j}}" == "$j" ]]; then
			if (( quietLevel < 2)); then
				if (( ! isFile )) || (( isFile && ! quietLevel )); then
					EchoStr 8 "\b"; echo -n "Skipping"
				fi
				if $verbose; then
					if [[ "$i" == "$outputDir" ]]
						then echo -n ", won't backup output directory"
						else echo -n ", blacklisted"
					fi
				fi
				echo ""
			fi
			return 1
		fi
	done

	if (( isFile )); then Backup $tabCount "$i" $isFile; return $?; fi

	#if (has blacklisted subdir)
	for j in "${dirBlacklist[@]}"; do
		#/${j:0:SizeOf$i} == $i
		if [[ "${j:0:${#i}}" == "$i" ]]; then
			if (( quietLevel < 2)); then
				if (( ! isFile )) || (( isFile && ! quietLevel )); then
					local returnBool=false

					echo -e " subentries\n"
					DeeperScan "$checkSize" $tabCount "$i" && returnBool=true
					echo ""

					if $returnBool
						then return 0
						else return 1
					fi
				fi
			fi
			
			unset isFile
			unset j

			DeeperScan "$checkSize" $tabCount "$i"
			
			return $?
		fi
	done

	#if (is requiredScan dir) or (has requiredScan subdir)
	for j in "${requiredScan[@]}"; do
		#/${j:0:SizeOf$i} == $i
		if [[ "${j:0:${#i}}" == "$i" ]]; then
			if (( quietLevel < 2)); then
				if (( ! isFile )) || (( isFile && ! quietLevel )); then
					local returnBool=false
					
					echo -e " subentries\n"
					DeeperScan "$checkSize" $tabCount "$i" && returnBool=true
					echo ""

					if $returnBool
						then return 0
						else return 1
					fi
				fi
			fi

			unset isFile
			unset j

			DeeperScan "$checkSize" $tabCount "$i"
			
			return $?
		fi
	done

	local -i size=0

	if $checkSize; then
		size=$(CheckSize "$i")
		if (( size > fiveGigabytes ))
			then checkSize=true
			else checkSize=false
		fi
	fi

	if $checkSize; then
		unset checkSize
		unset isFile
		unset j

		if (( quietLevel < 2)); then
			if (( ! isFile )) || (( isFile && ! quietLevel )); then
				local returnBool=false

				echo -e " subentries\n"
				DeeperScan true $tabCount "$i" && returnBool=true
				echo ""

				if $returnBool
					then return 0
					else return 1
				fi
			fi
		fi
	
		DeeperScan true $tabCount "$i"

		return $?
	else
		Backup $tabCount "$i" $size $isFile
	fi
	
	return $?
}

#param $1 is bool
#param $2 is int
#param $3 is str
#returns 0 if (any subdir is backed up)
#returns 1 if (no subdir is backed up)
DeeperScan(){
	set -o noglob
	mapfile -t curdirSubdirs < <(sudo ls -a "$i")

	for j in "${curdirSubdirs[@]}"; do
		if [[ "$j" == "." ]] || [[ "$j" == ".." ]]; then continue; fi
		
		Scan "$checkSize" $(( tabCount + 1 )) "$i/$j" && returnBool=true
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
#param $3 is int
#param $4 is int
#returns 0 if (runs backup)
#returns 1 if (does not run backup)
Backup(){
	local i="$2"
	local isFile=$4

	#if destination archive (doesn't exist) or (is older than the source)
	if WriteOrNot "$i"; then
		local -i tabCount=$1

		if (( quietLevel < 2 )); then
			if (( ! isFile )) || (( isFile && ! quietLevel )); then
				EchoStr 8 "\b"
				echo "Backing up"
			fi
		fi

		if ! $dryRun; then
			local -i size=$3

			#if ($i isn't in backupRoot)
			if [[ ! -d "$outputDir$(dirname "$i")" ]]; then
				sudo mkdir -p "$outputDir$(dirname "$i")" 2> /dev/null
			elif [[ -d "$outputDir$i" ]]; then
				sudo rm -r "$outputDir$i"
			elif [[ -f "$outputDir$i.tar.$archiveExt" ]]; then
				sudo rm "$outputDir$i.tar.$archiveExt"
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

			Compress "$i" "$isFile" "$size"
		fi

		if (( quietLevel < 2 )); then
			if (( ! isFile )) || (( isFile && ! quietLevel )); then
				echo -ne "\e[1A\e[K"
				if ! $dryRun; then echo -ne "\e[1A\e[K"; fi
				EchoStr $tabCount "\t"
				echo "$(basename "$i"): Backed up"
			fi
		fi
		return 0
	fi

	if (( quietLevel < 2)); then
		if (( ! isFile )) || (( isFile && ! quietLevel )); then
			EchoStr 8 "\b"; echo -n "Skipping"
			if $verbose; then echo -n ", no changes detected. "; fi
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

	if [[ ! -f "$outputDir$source.tar.$archiveExt" ]]; then
		return 0
	fi

	local -i fileTime
	fileTime="$(date -r "$source" "+%Y%m%d%H%M%S")"

	local -i archiveTime
	archiveTime="$(date -r "$outputDir$source.tar.$archiveExt" "+%Y%m%d%H%M%S")"

	if (( fileTime > archiveTime));
		then return 0
		else return 1
	fi
}

#param $1 is str
#param $2 is int
#param $3 is int
Compress(){
	local i="$1"
	local -i isFile=$2
	local -i size=$3
	local tempOpts=""

	if (( ! size )); then size=$(CheckSize "$i"); fi

	lastBackupSize=$size
	totalSize=$(( totalSize + size ))
	lastBackup="$outputDir$i.tar.$archiveExt"

	if (( quietLevel > 1 )); then
		tempOpts="--quiet"
	elif (( quietLevel )) && (( isFile )); then
		tempOpts="--quiet"
	else
		tempOpts="$pvOpts"
	fi

	sudo tar --absolute-names --directory "$(sudo dirname "$i")" --create --file - "$(sudo basename "$i")" 2>/dev/null |\
	eval "pv $tempOpts --size $size" |\
	"${archive[@]}" 2>/dev/null |\
	sudo tee "$outputDir$i.tar.$archiveExt" &> /dev/null

	lastBackup=""
	lastBackupSize=0
}