/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/

#include <algorithm>
#include <fstream>

#include "config.h"
#include "shared.h"

using std::array;

enum{
	tar, gzip, bzip2, xz
};

/**
  * Checks if c exists in str
  * @return returns first index of c in str
  * @return SIZE_MAX if c isn't in str
  **/
size_t charAt(const char c, const string& str){
	for (size_t i = 0; i < str.length(); i++){
		if (str.at(i) == c){return i;}
	}
	return SIZE_MAX;
}

bool OpenConfig(){
	if (!is_regular_file(config)){
		error("config path is invalid \"", (config.string() + '"'));
	}
	
	if (!isRealPath(config)){
		WriteConfig();
		return false;
	}

	ReadConfig();

	return true;
}

/**
  * Outputs generic error message for bad str and exits
  * @param str bad str to be output in error
  **/
void genericError(string& str){
	if (str.empty()){error("empty str");}

	str.push_back('"');
	error("unknown str in config \"", str, str.at(0));
}

/**
  * Populate vec with entries from word, seperated by c
  **/
void vectorPushBack(const string& word, vector<path>& vec, const char c){
	uint32_t beg = 0, end = 0;

	for (; end < word.length(); end++){
		if (word.at(end) == c){
			if (beg == end){
				beg++;
				continue;
			}

			vec.emplace_back(word.substr(beg, end - beg));
			beg = end + 1;
		}
	}

	if (beg != end){
		vec.emplace_back(word.substr(beg, end - beg));
	}
}

/**
  * Returns information about compression value
  * @return [0] is compression level denoted by -#, --best, or --fast, UINT8_MAX if no level is found
  * @return [1] is non-zero if -#e, -e, or --extreme are detected
  **/
array<uint8_t, 2> getCompressionValue(string& word){
	array<uint8_t, 2> ret = {UINT8_MAX, false};

	if (word.front() == '"'){word = word.substr(1);}
	if (word.back() == '"'){
		word = word.substr(0, word.length() - 1);
	}

	if (word.length() < 2){return ret;}

	if (word.length() < 3){
		if (isdigit(word.at(1))){ret[0] = word.at(1) - 48;}

		else if (word.at(1) == 'e'){ret[1] = true;}
	}

	else if (isdigit(word.at(1)) && word.at(2) == 'e'){
		ret[0] = word.at(1) - 48;
		ret[1] = true;
	}

	else{
		if (word == "--best"){ret[0] = 9;}
		else if (word == "--fast"){ret[0] = 0;}
		else if (word == "--extreme"){ret[1] = true;}
	}

	return ret;
}

/**
  * Returns env variable for call
  * @param call is env variable without leading $ (HOME instead of $HOME)
  * @param bucket container of env calls and their results
  **/
string getEnv(const string& call, vector<array<string, 2>>& bucket){
	enum{srs, ret}; //source, return

	for (const array<string, 2>& a : bucket){
		if (a.at(srs) == call){return a.at(ret);}
	}

	bucket.push_back({call, getenv(call.c_str())});

	return bucket.back().at(ret);
}

void getMaxSize(string& word){
	//abbreviations
	const array<char, 4> a = {'K', 'M', 'G', 'B'};
	//multipliers
	const array<uint64_t, 4> m = {
			((uint64_t)1 << 10), ((uint64_t)1 << 20),
			((uint64_t)1 << 30), ((uint64_t)1 << 40)
	};

	if (word.length() < 2 || !std::all_of(word.end() - 2, word.end(), isalpha)){
		word.push_back('\"');
		error("invalid MaxSize abbreviation \"", word.c_str());
	}

	//get wordInt
	if (!std::all_of(word.begin(), (word.end() - 2), isdigit)){
		error("MaxSize multiplier contains non-integer character");
	}
	uint64_t wordInt = std::stoll(word.substr(0, word.length() - 2));

	if (!wordInt){
		error("MaxSize must have non 0 multiplier");
	}

	for (uint8_t idx = 0; idx < 4; ++idx){
		if (*(word.end() - 2) != a.at(idx)){continue;}

		maxSize = wordInt * m.at(idx);

		//check for overflow
		if (maxSize < wordInt || maxSize < m.at(idx)){
			word.push_back('\"');
			error("MaxSize multiplier causes overflow \"",word.c_str(), UINT8_MAX);
		}

		return;
	}

	string detail = {*(word.end() - 2), word.back(), '\"'};
	error("invalid MaxSize abbreviation \"", detail.c_str(), *(word.end() - 2));
}

/**
  * ReadConfig() helper function, parses line and assigns them to shared.cpp values
  * @param bucket arrays are formatted as {env variable, returned value}
  **/
void ReadLine(string& line, vector<array<string, 2>>& bucket, uint8_t& comType){
	if (line.empty() || line.at(0) == '#') return;

	string word;
	size_t i;

	for (i = charAt('=', line) + 1; i < line.length(); ++i){
		if (line.at(i) == '$'){
			const size_t beg = i;

			while (++i < line.size() && isalnum(line.at(i))){}

			word += getEnv(line.substr(beg + 1, i - (beg + 1)), bucket);

			if (i == line.size()){continue;}
		}

		word.push_back(line.at(i));
	}

	if (i == SIZE_MAX){genericError(line);}

	//assign word to proper variable
	switch (line.at(0)){
		//bacRoot and blacklisted
		case 'B':{
			if (line.size() < 2){genericError(line);}

			if (line.at(1) == 'a'){
				if (!bacRoot.empty()){return;}
				bacRoot = word;
			}

			else if (line.at(1) == 'l'){vectorPushBack(word, blacklist, ',');}
			
			else{genericError(line);}

			break;
		}

			//maxSize
		case 'M':{
			if (!maxSize){getMaxSize(word);}
			return;
		}

			//whitelist
		case 'W':{
			vectorPushBack(word, whitelist, ',');
			return;
		}

			//split
		case 'S':{
			vectorPushBack(word, split, ',');
			return;
		}

			//bacDir
		case 'O':{
			if (bacDir.empty()){bacDir = path(word);}
			return;
		}

			//comType, comArgs, and collective
		case 'C':{
			if (line.length() < 12){genericError(line);}

			switch(line.at(11)){
				//comArgs
				case 'L':{
					array<uint8_t, 2> arr = getCompressionValue(word);

					if (arr[0] == UINT8_MAX){genericError(line);}

					//if (arr[0] is 0 or arr[1] is non-zero) and (comType is not xz)
					//allow for empty comType in case comType line is yet to come
					if ((arr[0] == 0 || arr[1]) && comType != 3){
						error("invalid CompressionLevel with CompressionType", "", arr[0]);
					}

					comArgs = "-" + std::to_string(arr[0]);
					if (arr[1]){comArgs += " --extreme";}

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
					vectorPushBack(word, collective, ',');
					break;
				}
			}

			return;
		}

			//unknown line
		default:{genericError(line);}
	}
}

void ReadConfig(){
	//main read process
	uint8_t comType; //id for compression type
	{
		std::ifstream ifs;
		ifs.open(config);
		if (!ifs.is_open()) error("cannot read config");

		vector<array<string, 2>> bucket{{"HOME", home.string()}};

		for (string line; getline(ifs, line);){
			ReadLine(line, bucket, comType);
		}
	}

	//fix empty comArgs and empty maxSize
	{
		//maxSize
		if (!maxSize){maxSize = (uint64_t)5 << 30;}

		if (comArgs.empty()){comArgs = ((comType == xz) ? "-0" : "-1");}

		uint16_t pos = 0, n = comArgs.length();

		if (comArgs.back() == '"'){--n;}
		if (comArgs.front() == '"'){
			++pos;
			--n;
		}

		if (n != comArgs.length()){comArgs = comArgs.substr(pos, n);}
	}

	//insert main compression call string to comArgs
	switch (comType){
		case tar:{
			comArgs.assign("");
			conExt.assign(".tar");
			return;
		}

		case gzip:{
			comArgs = "gzip " + comArgs + " --stdout";
			conExt.assign(".tar.gz");
			break;
		}

		case bzip2:{
			comArgs = "bzip2 --compress " + comArgs + " --stdout";
			conExt.assign(".tar.bz2");
			break;
		}

		case xz:{
			comArgs = "xz --compress --threads=0 " + comArgs + " --stdout";
			conExt.assign(".tar.xz");
			break;
		}

		default:{
			std::cout << "Error: Invalid compression type." << std::endl;
			exit(comType);
		}
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
				error(p, " is not a valid path");
			}
		}
		v.shrink_to_fit();
	}
	comArgs.shrink_to_fit();
}

void WriteConfig(){
	create_directories(config);

	std::ofstream ofs;
	ofs.open(config);
	if (!ofs.is_open()){return;}
	
	const vector<string> text = {
			"#The directory considered the root of the backup",
			"#If unknown, leave as '/'",
			"",
			"BackupRootDir=/",
			"",
			"#Directories over MaxSize will have their subfiles and subdirectories checked instead",
			"#Supports unsigned integer values with double letter abbreviation from KB up to and including TB",
			"#Defaults to 5GB",
			"MaxSize=5GB",
			"",
			"#Files and directories that should be backed up under the BackupRoot",
			"#Separate entries by comma",
			"Whitelist=/home",
			"",
			"#Files and directories that will be excluded",
			"#Separate entries by comma",
			"Blacklist=/home/lost+found,$HOME/.cache,$HOME/.local/share/Trash",
			"",
			"#Directories that won't be made into a single archive, even if smaller than MaxSize",
			"#Good for directories with constantly updating files like /bin or /home",
			"SplitBackup=/home,$HOME",
			"",
			"#Directories that will be made into a single archive, even if equal to or larger than MaxSize",
			"#CollectiveBackup=",
			"",
			"#Specify where to store backup",
			"OutputLocation=$HOME/flexible_backup",
			"",
			"#Which compression type should be used",
			"#none=0, gzip=1, bzip2=2, xz=3",
			"CompressionType=1",
			"",
			"#Set as -# where number if between 1-9, --fast, or --best",
			"#Supports -0 and --extreme flag with xz",
			"#eg \"-6 -e\"",
			"CompressionLevel=\"-6\"",
	};
	for (const string& i : text){ofs << i << std::endl;}

	ofs.close();
}

//help text
void outputHelp(){
	enum{arg, txt};

	const vector<string> header = {
			"Flexible-Backup",
			"Backup files using tar and a chosen compressor",
			"",
			"Options:",
	};
	const uint8_t l_body = 26; //largest body string
	const vector<array<string, 2>> body = {
			{"-c, --config=PATH", "Load config information from PATH"},
			{"-h, --help", "Display this message"},
			{"-r PATH, --root=PATH", "Set BackupRootDir to PATH"},
			{"-o PATH, --output=PATH", "Set OutputLocation to PATH"},
			{"--maxSize=NB", "Set max size a directory can be where N is a number and B is KB to TB"},
			{"-B PATH, --blacklist=PATH", "Add PATH to blacklist"},
			{"-W PATH, --whitelist=PATH", "Add PATH to Whitelist"},
			{"-S PATH, --splitlist=PATH", "Add PATH to SplitBackup"},
			{"-C PATH, --collective=PATH", "Add PATH to CollectiveBackup"},
			//{"--compressLevel=N", "Set CompressionLevel to N"},
			//{"--compressType=N", "Set CompressionType to N"}
	};

	for (const string& i : header){std::cout << i << std::endl;}
	for (const array<string, 2>& arr : body){
		std::cout << "  " << std::setw(l_body) << std::left << arr.at(0) << "  " << arr.at(1) << std::endl;
	}
}

bool GetArgsHelper(const string& arg, const array<array<string, 9>, 2>& options){
	bool multiChar;
	if (arg.size() < 2){return false;}

	if (arg.at(0) != '-'){return false;}

	uint8_t i = 0;
	string argRoot, o;

	//single character format (-h)
	if (arg.at(1) != '-'){
		argRoot = arg.at(1);
		o = arg.substr(3, arg.size() - 3);
		multiChar = false;
	}
	//multi character format (--help)
	else{
		size_t eq = charAt('=', arg);
		argRoot = arg.substr(2, eq - 2);
		o = arg.substr(eq + 1, arg.size() - (eq + 1));
		multiChar = true;
	}

	if (o.empty()){return false;}

	for (; i < 11; i++){
		if (options[multiChar][i] == argRoot){break;}
	}

	switch(i){
		case 0:{
			config.assign(o);
			break;
		} //config
		case 1:{
			outputHelp();
			exit(0);
		} //help
		case 2:{
			bacRoot.assign(o);
			break;
		} //backup root
		case 3:{
			bacDir.assign(o);
			break;
		} //backup dir
		case 4:{
			getMaxSize(o);
			break;
		} //maxSize
		case 5:{
			blacklist.emplace_back(o);
			break;
		} //blacklist
		case 6:{
			whitelist.emplace_back(o);
			break;
		} //whitelist
		case 7:{
			split.emplace_back(o);
			break;
		} //split
		case 8:{
			collective.emplace_back(o);
			break;
		} //collective
		case 9:{
			comArgs = o;
			break;
		} //compressLevel
		case 10:{
			break;
		} //compressType
		default:{return false;}
	}

	return true;
}

void GetArgs(const int argc, char* argv[]){
	const array<array<string, 9>, 2> options = {
			"c", "h", "r",
			"o", "", "B",
			"W", "S", "C",
			//"", "",
			"config", "help", "root",
			"output", "maxSize", "blacklist",
			"whitelist", "splitlist", "collective",
			//"compressLevel", "compressType"
	};

	for (int i = 1; i < argc; i++){
		if (!GetArgsHelper(string(argv[i]), options)){
			error("Invalid argument, ", argv[i]);
		}
	}
}
