/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/

#include <fstream>

#include "config.h"
#include "shared.h"

using std::array;

enum{
	tar, gzip, bzip2, xz
};

/**
  * Checks if character exists in a string
  * @param c char to check for
  * @param line string to check c against
  * @return returns first index of c in line
  * @return SIZE_MAX if c isn't in line
  **/
size_t charAt(const char c, const string& line){
	for (size_t i = 0; i < line.length(); i++)
		if (line.at(i) == c) return i;
	return SIZE_MAX;
}

bool OpenConfig(const string& loc){
	path config = ((loc.empty()) ?
			(home / ".config/flexible-backup.conf") : path(loc));

	if (!isRealPath(config)){
		WriteConfig(config);
		return false;
	}

	ReadConfig(config);

	return true;
}

/**
  * Outputs generic error message for bad line and exits
  * @param line bad line to be output in error
  **/
void genericError(const string& line){
	if (line.empty())
		error("empty line", "", 1);

	error("unknown line in config \"", (line + '"').c_str(), line.at(0));
}

void vectorPushBack(const string& word, vector<path>& vec){
	uint32_t beg = 0, end = 0;
	for (; end < word.length(); end++)
		if (word.at(end) == ','){
			if (beg == end){
				beg++;
				continue;
			}

			vec.emplace_back(word.substr(beg, end - beg));
			beg = end + 1;
		}
	if (beg != end)
		vec.emplace_back(word.substr(beg, end - beg));
}

/**
  * Returns information about compression value
  * @return array[0] is compression level denoted by -#, --best, or --fast, UINT8_MAX if no level is found
  * @return array[1] is non-zero if -#e, -e, or --extreme are detected
  **/
array<uint8_t, 2> getCompressionValue(string& word){
	array<uint8_t, 2> ret{UINT8_MAX, false};

	if (word.front() == '"')
		word = word.substr(1);
	if (word.back() == '"')
		word = word.substr(0, word.length() - 1);

	if (word.length() < 2) return ret;

	if (word.length() < 3){
		if (isdigit(word.at(1)))
			ret[0] = word.at(1) - 48;

		else if (word.at(1) == 'e')
			ret[1] = true;

		return ret;
	}

	if (isdigit(word.at(1)) && word.at(2) == 'e'){
		ret[0] = word.at(1) - 48;
		ret[1] = true;

		return ret;
	}

	if (word == "--best") ret[0] = 9;
	else if (word == "--fast") ret[0] = 0;
	else if (word == "--extreme") ret[1] = true;

	return ret;
}

/**
  * ReadConfig() helper function, parses line and assigns them to shared.cpp values
  * @param bucket arrays are formatted as {env variable, returned value}
  **/
void ReadConfigHelper(string& line, vector<array<string, 2>>& bucket, uint8_t& comType){
	enum{source, result};

	if (line.empty() || line.at(0) == '#') return;

	string word;
	size_t i;

	for (i = charAt('=', line) + 1; i < line.length(); i++){
		//if (line.at(i) == '\\') continue; //idk purpose but leaving in case something breaks
		if (line.at(i) != '$'){
			word.push_back(line.at(i));
			continue;
		}

		string call; //the result of getenv(call)
		string tmpStr;

		while (++i < line.size() && isalnum(line.at(i))){
			call.push_back(line.at(i));
		}

		for (const array<string, 2>& j : bucket)
			if (j.at(source) == call){
				tmpStr = j.at(result);
				break;
			}

		if (tmpStr.empty()){
			tmpStr = getenv(call.c_str());
			if (!tmpStr.empty())
				bucket.push_back({call, tmpStr});
		}

		word += tmpStr;
		if (i < line.size()) word += line.at(i);
	}

	if (line == word){word = "";}

	if (i == SIZE_MAX){genericError(line);}

	//assign word to proper variable
	switch (line.at(0)){
		//bacRoot and blacklisted
		case 'B':{
			if (line.size() < 2){genericError(line);}

			if (line.at(1) == 'a'){bacRoot = word;}

			else if (line.at(1) == 'l'){vectorPushBack(word, blacklist);}
			
			else{genericError(line);}

			break;
		}

			//maxSize
		case 'M':{
			//abbreviations
			const array<char, 4> a = {'K', 'M', 'G', 'B'};
			//multipliers
			const array<uint64_t, 4> m = {
					((uint64_t)1 << 10), ((uint64_t)1 << 20),
					((uint64_t)1 << 30), ((uint64_t)1 << 40)
			};
			if (word.length() < 2 || !isalpha(word.back()) || !isalpha(word.at(word.length() - 2))){
				error("invalid MaxSize abbreviation \"", (word + '\"').c_str());
			}
			const string wordSfx = word.substr(word.length() - 2, 2);

			uint64_t wordInt;
			//get wordInt
			for (char c : word.substr(0, word.length() - 2)){
				if (!isdigit(c)){
					error("MaxSize multiplier contains non-integer character", "", c);
				}
				wordInt = (wordInt * 10) + (c - 48);
			}

			for (uint8_t h = 0; h < 4; h++){
				if (toupper(wordSfx.at(0)) != a.at(h)){continue;}

				maxSize = wordInt * m.at(h);

				//check for overflow
				if (wordInt != 0 && (maxSize < wordInt || maxSize < m.at(h))){
					error("MaxSize multiplier causes overflow \"",
						  (word + '\"').c_str(), UINT8_MAX);
				}

				return;
			}

			error("invalid MaxSize abbreviation \"", wordSfx.c_str(), wordSfx.at(0));
		}

			//whitelist
		case 'W':{
			vectorPushBack(word, whitelist);
			return;
		}

			//split
		case 'S':{
			vectorPushBack(word, split);
			return;
		}

			//bacDir
		case 'O':{
			bacDir = path(word);
			return;
		}

			//comType, comArgs, and collective
		case 'C':{
			if (line.length() < 12){genericError(line);}

			switch(line.at(11)){
				//comArgs
				case 'L':{
					array<uint8_t, 2> arr = getCompressionValue(word);

					if (arr[0] == UINT8_MAX) genericError(line);

					//if (arr[0] is 0 or arr[1] is non-zero) and (comType is not xz)
					//allow for empty comType in case comType line is yet to come
					if ((arr[0] == 0 || arr[1]) && comType != 3)
						error("invalid CompressionLevel with CompressionType", "", arr[0]);

					comArgs = "-" + std::to_string(arr[0]);
					if (arr[1]) comArgs += " --extreme";

					break;
				}

				//comType
				case 'T':{
					if (word.length() > 1 || !isdigit(word.at(0))){genericError(line);}
					comType = word.at(0) - 48;
					break;
				}

				//collective
				case 'a':{
					vectorPushBack(word, collective);
					break;
				}
			}

			return;
		}

			//unknown line
		default: genericError(line);
	}
}

void ReadConfig(const path& config){
	//main read process
	uint8_t comType; //id for compression type
	{
		std::ifstream ifs;
		ifs.open(config);
		if (!ifs.is_open()) error("cannot read config");

		vector<array<string, 2>> bucket{{"HOME", home.string()}};

		for (string line; getline(ifs, line);)
			ReadConfigHelper(line, bucket, comType);
	}

	//fix empty comArgs and empty maxSize
	{
		//maxSize
		if (!maxSize){maxSize = 5 * ((uint64_t)1 << 30);}

		if (comArgs.empty()){comArgs = ((comType == xz) ? "-0" : "-1");}

		uint16_t pos = 0, n = comArgs.length();

		if (comArgs.back() == '"'){--n;}
		if (comArgs.front() == '"'){
			++pos;
			--n;
		}

		if (pos || n != comArgs.length())
		{comArgs = comArgs.substr(pos, n);}
	}

	//insert main compression call string to comArgs
	{
		string tmpStr;
		tmpStr.reserve(35 + comArgs.size());
		//35 = 26(longest compression string (xz)) + 9(--stdout)

		switch (comType){
			case tar:{
				comArgs = "";
				conExt = ".tar";
				return;
			}

			case gzip:{
				tmpStr = "gzip ";
				conExt = ".tar.gz";
				break;
			}

			case bzip2:{
				tmpStr = "bzip2 --compress ";
				conExt = ".tar.bz2";
				break;
			}

			case xz:{
				tmpStr = "xz --compress --threads=0 ";
				conExt = ".tar.xz";
				break;
			}

			default:{
				std::cout << "Error: Invalid compression type." << std::endl;
				exit(comType);
			}
		}

		tmpStr += comArgs + " --stdout";

		comArgs = tmpStr;
	}

	//check and insert backupDir to blacklist
	{
		bool hasDir = false;
		for (const path& i : blacklist){
			if (i == bacDir){
				hasDir = true;
				break;
			}
		}
		if (!hasDir){blacklist.emplace_back(bacDir);}
	}

	//validate that all path vectors contain valid paths and shrink to reduce memory
	for (vector<path>& v : vector<vector<path>>{whitelist, blacklist, split, collective}){
		for (const path& p : v){
			if (!isRealPath(p, true)){
				error(p.c_str(), " is not a valid path");
			}
		}
		v.shrink_to_fit();
	}
	comArgs.shrink_to_fit();
}

void WriteConfig(const path& conf){
	using std::endl;

	create_directories(conf);

	std::ofstream ofs;
	ofs.open(conf);
	if (!ofs.is_open()){return;}
	
	ofs << "#The directory considered the root of the backup" << endl;
	ofs << "#If unknown, leave as '/'" << endl << endl;
	ofs << "BackupRootDir=/" << endl << endl;
	ofs << "#Directories over MaxSize will have their subfiles and subdirectories checked instead" << endl;
	ofs << "#Supports unsigned integer values with double letter abbreviation from KB up to and including TB" << endl;
	ofs << "#Defaults to 5GB" << endl;
	ofs << "MaxSize=5GB" << endl << endl;
	ofs << "#Files and directories that should be backed up under the BackupRoot" << endl;
	ofs << "#Separate entries by comma" << endl;
	ofs << "Whitelist=/home" << endl << endl;
	ofs << "#Files and directories that will be excluded" << endl;
	ofs << "#Separate entries by comma" << endl;
	ofs << "Blacklist=/home/lost+found,$HOME/.cache,$HOME/.local/share/Trash" << endl << endl;
	ofs << "#Directories that won't be made into a single archive, even if smaller than MaxSize" << endl;
	ofs << "#Good for directories with constantly updating files like /bin or /home" << endl;
	ofs << "SplitBackup=/home,$HOME" << endl << endl;
	ofs << "#Directories that will be made into a single archive, even if equal to or larger than MaxSize" << endl;
	ofs << "#CollectiveBackup=" << endl << endl;
	ofs << "#Specify where to store backup" << endl;
	ofs << "OutputLocation=$HOME/flexible_backup" << endl << endl;
	ofs << "#Which compression type should be used" << endl;
	ofs << "#none=0, gzip=1, bzip2=2, xz=3" << endl;
	ofs << "CompressionType=1" << endl << endl;
	ofs << "#Set as -# where number if between 1-9, --fast, or --best" << endl;
	ofs << "#Supports -0 and --extreme flag with xz" << endl;
	ofs << "#eg \"-6 -e\"" << endl;
	ofs << "CompressionLevel=\"-6\"" << endl;

	ofs.close();
}