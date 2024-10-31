#!/usr/bin/env bash

export conf="$HOME/.config/flexible-backup.conf"

#returns 0 if ($conf file exists)
#returns 1 if ($conf file does not exits)
CheckConf(){
	#shellcheck source="./backup"
	if [[ -f "$conf" ]];
		then ReadConf; return 0
		else MakeConf; return 1
	fi
}

ReadConf(){
	#shellcheck source="/home/ethelnol/.config/flexible-backup.conf"
	source "$HOME/.config/flexible-backup.conf"

	IFS=","

	backupRoot="$BackupRootDir"
	if [[ "${backupRoot:0:1}" != "/" ]]; then backupRoot="/$backupRoot"; fi
	if [[ "${backupRoot:0-1}" == "/" ]] && [[ "$backupRoot" != "/" ]]; then
		backupRoot="${backupRoot:0:0-1}"
	fi
	
	outputDir="$OutputLocation"
	if [[ "${outputDir:0:1}" != "/" ]]; then outputDir="/$outputDir"; fi
	if [[ "${outputDir:0-1}" == "/" ]] && [[ "$outputDir" != "/" ]]; then
		outputDir="${outputDir:0:0-1}"
	fi

	read -ra dirWhitelist <<< "$Whitelist"
	for i in "${!dirWhitelist[@]}"; do
		if [[ "${dirWhitelist[i]:0:1}" == "/" ]]; then dirWhitelist[i]="${dirWhitelist[$i]:1}"; fi
		if [[ "${dirWhitelist[i]:0-1}" == "/" ]]; then dirWhitelist[i]="${dirWhitelist[$i]:0:0-1}"; fi
	done

	read -ra dirBlacklist <<< "$Blacklist"
	for i in "${!dirBlacklist[@]}"; do
		if [[ "${dirBlacklist[i]:0:1}" == "/" ]]; then dirBlacklist[i]="${dirBlacklist[$i]:1}"; fi
		if [[ "${dirBlacklist[i]:0-1}" == "/" ]]; then dirBlacklist[i]="${dirBlacklist[$i]:0:0-1}"; fi
	done
	dirBlacklist+=("${outputDir:1}")

	read -ra requiredScan <<< "$SplinteredBackupRequired"
	for i in "${!requiredScan[@]}"; do
		if [[ "${requiredScan[i]:0:1}" == "/" ]]; then requiredScan[i]="${requiredScan[i]:1}"; fi
		if [[ "${requiredScan[i]:0-1}" == "/" ]]; then requiredScan[i]="${requiredScan[i]:0:0-1}"; fi
	done
	
	unset BackupRoot
	unset OutputLocation
	unset Whitelist
	unset Blacklist
	unset SplinteredBackupRequired
}

MakeConf(){
	sudo touch "$conf"
	ConfFormat | sudo tee "$conf" &> /dev/null
	sudo chmod 600 "$conf"
	sudo chown "$USER:$USER" "$conf"

	echo "Config file made and edited at \"$conf\""
}

ConfFormat(){
	echo "#The directory the program considers the root of the backup"
	echo "#If unknown, leave as '/'"
	echo -e "BackupRootDir=/\n"
	echo "#Files and directories that should be backed up under the BackupRoot"
	echo "#Separate entries by comma"
	echo -e "Whitelist=/home\n"
	echo "#Directories that won't be made into a single archive even if smaller than 5gb"
	echo "#Archives will be made of all the entries in the directory specifically instead of one big archive"
	echo "#Good for directories with constantly updating files like /bin"
	echo -e "SplinteredBackupRequired=/home,\$HOME\n"
	echo "#Files and directories that will be excluded"
	echo -e "Blacklist=/home/lost+found,\$HOME/.cache,\$HOME/.local/share/Trash\n"
	echo "#Specify where files should be sent after backup"
	echo -e "OutputLocation=\$HOME/flexible_backup\n"
}