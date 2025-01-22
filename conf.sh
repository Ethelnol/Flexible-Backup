#!/usr/bin/env bash

export conf="$HOME/.config/flexible-backup.conf"

#returns 0 if ($conf file exists)
#returns 1 if ($conf file does not exits)
CheckConf(){
	if [[ -f "$conf" ]]
		then ReadConf
		else MakeConf
	fi
	return $?
}

#returns 1 if ($CompressionType from conf is not valid) or ($CompressionLevel from conf is not valid)
ReadConf(){
	export backupRoot=""
	export outputDir=""

	export dirWhitelist=()
	export dirBlacklist=()
	export requiredScan=()

	export archive=()
	export archiveExt=""
	export archiveExtensions=()

	local -i i=0

	source "$conf"

	IFS=","

	backupRoot="$BackupRootDir"
	if [[ "${backupRoot:0-1}" == "/" ]]; then backupRoot="${backupRoot:0:0-1}"; fi
	if [[ "${backupRoot:0:1}" != "/" ]]; then backupRoot="/$backupRoot"; fi
	
	outputDir="$OutputLocation"
	if [[ "${outputDir:0-1}" == "/" ]]; then outputDir="${outputDir:0:0-1}"; fi
	if [[ "${outputDir:0:1}" != "/" ]]; then outputDir="/$outputDir"; fi

	archive=("sudo")
	case $CompressionType in
		0)
			archive+=("gzip")
			archiveExt="gz"
			archiveExtensions=("bz2" "xz")
			;;
		1)
			archive+=("bzip2" "--compress")
			archiveExt="bz2"
			archiveExtensions=("gz" "xz")
			;;
		2)
			archive+=("xz" "--compress" "--threads=0")
			archiveExt="xz"
			archiveExtensions=("gz" "bz2")
			;;
		*)
			echo "Compression type not recognized -- '$CompressionType'"
			return 1
			;;
	esac

	case $CompressionLevel in
		--best | --fast | -[1-9])
			;;			
		-[1-9]e | -[1-9]\ -e | -[1-9]\ --extreme )
			if (( CompressionType != 2 )); then
				echo "Cannot use extreme compression, only valid with xz"
				return 1
			fi
			;;
		"")
			CompressionLevel="-6"
			echo "CompressionLevel not set, defaulting to \"-6\""
			;;
		* )
			echo "Invalid option -- '$CompressionLevel'"
			return 1
			;;
	esac
	archive+=("$CompressionLevel" "--keep" "--stdout")

	read -ra dirWhitelist <<< "$Whitelist"
	for i in "${!dirWhitelist[@]}"; do
		if [[ "${dirWhitelist[i]:0:1}" != "/" ]]; then dirWhitelist[i]="/${dirWhitelist[i]}"; fi
		if [[ "${dirWhitelist[i]:0-1}" == "/" ]]; then dirWhitelist[i]="${dirWhitelist[i]:0:0-1}"; fi
	done

	read -ra dirBlacklist <<< "$Blacklist"
	for i in "${!dirBlacklist[@]}"; do
		if [[ "${dirBlacklist[i]:0:1}" != "/" ]]; then dirBlacklist[i]="/${dirBlacklist[i]}"; fi
		if [[ "${dirBlacklist[i]:0-1}" == "/" ]]; then dirBlacklist[i]="${dirBlacklist[i]:0:0-1}"; fi
	done
	dirBlacklist+=("$outputDir")

	read -ra requiredScan <<< "$SplinteredBackupRequired"
	for i in "${!requiredScan[@]}"; do
		if [[ "${requiredScan[i]:0:1}" != "/" ]]; then requiredScan[i]="/${requiredScan[i]}"; fi
		if [[ "${requiredScan[i]:0-1}" == "/" ]]; then requiredScan[i]="${requiredScan[i]:0:0-1}"; fi
	done
	
	unset BackupRoot
	unset OutputLocation
	unset CompressionType
	unset CompressionLevel
	unset Whitelist
	unset Blacklist
	unset SplinteredBackupRequired

	return 0
}

MakeConf(){
	if touch "$conf"; then
		ConfFormat > "$conf"
		echo "Created config file at \"$conf\""
		return 1
	else
		echo "Could not create config file, make sure \"$(dirname "$conf")\" can be written to"
		return 2
	fi
}

ConfFormat(){
	echo    "#The directory the program considers the root of the backup"
	echo    "#If unknown, leave as '/'"
	echo -e "BackupRootDir=/\n"
	echo    "#Files and directories that should be backed up under the BackupRoot"
	echo    "#Separate entries by comma"
	echo -e "Whitelist=/home\n"
	echo    "#Directories that won't be made into a single archive even if smaller than 5gb"
	echo    "#Archives will be made of all the entries in the directory specifically instead of one big archive"
	echo    "#Good for directories with constantly updating files like /bin"
	echo -e "SplinteredBackupRequired=/home,\$HOME\n"
	echo    "#Files and directories that will be excluded"
	echo    "#If no leading '/' is provided, then a leading '/' will be assumed"
	echo    "#Separate entries by comma"
	echo -e "Blacklist=/home/lost+found,\$HOME/.cache,\$HOME/.local/share/Trash\n"
	echo    "#Specify where files should be sent after backup"
	echo -e "OutputLocation=\$HOME/flexible_backup\n"
	echo    "#Which compression type should be used"
	echo    "#gzip=0, bzip2=1, xz=2"
	echo -e "CompressionType=0\n"
	echo    "#Set as number between 1-9, --fast, or --best"
	echo    "#Supports -[1-9]e, -e, or --extreme when using xz"
	echo    "#eg CompressionLevel=\"-6 -e\""
	echo    "CompressionLevel=\"-6\""
}