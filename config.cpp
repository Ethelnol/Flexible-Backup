/** 
  * Created by Ethelnol on 18/07/2025
  **/

#include <algorithm>
#include <array>
#include <fstream>

#include "config.h"
#include "shared.h"

using std::array;

enum compression{
	tar   = 0,
	gzip  = 1,
	bzip2 = 2,
	xz    = 3
};

bool OpenConfig(){
	if (!is_regular_file(config)){
		error("config path is invalid \"", (config.string() + '"'));
	}

	if (!exists(config)){
		WriteConfig();
		return false;
	}

	ReadConfig();

	return true;
}

/**
  * Outputs generic error message for bad string and exits
  * @param str bad string to be output in error
  **/
void genericError(string& str){
	if (str.empty()){error("empty str");}

	str.push_back('"');
	error("unknown str in config \"", str, str.front());
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
  * Removes leading/tailing \" from <str>
  **/
void removeQuotes(string& str){
	string::size_type pos = 0, n = str.length();

	if (str.front() == '\"'){
		++pos;
		--n;
	}
	if (str.back() == '\"'){
		--n;
	}

	if (n != str.length()){
		str = str.substr(pos, n);
	}
}

/**
  * Returns information about compression value
  * @post returned array must be deallocated after function call
  * @return [0] is compression level denoted by -#, --best, or --fast, UINT8_MAX if no level is found
  * @return [1] is non-zero if -#e, -e, or --extreme are detected
  **/
array<uint8_t, 2> getCompressionValue(string& word){
	array<uint8_t, 2> ret = {
		UINT8_MAX,
		false
	};

	removeQuotes(word);

	//invalid argument passed
	if (word.length() < 2){
		return ret;
	}

	switch (word.length()){
		//invalid length
		case 0: case 1:{
			break;
		}

		//"-#" or "-e" argument
		case 2:{
			if (isdigit(word.at(1))){
				ret[0] = word.at(1) - 48;
			}
			else if (word.at(1) == 'e'){
				ret[1] = true;
			}
			break;
		}

		//"-#e" argument
		case 3:{
			if (isdigit(word.at(1)) && word.at(2) == 'e'){
				ret[0] = word.at(1) - 48;
				ret[1] = true;
			}

			break;
		}

		//long argument
		default:{
			if (word == "--best"){
				ret[0] = 9;
			}
			else if (word == "--fast"){
				ret[0] = 0;
			}
			else if (word == "--extreme"){
				ret[1] = true;
			}

			break;
		}
	}

	return ret;
}

/**
  * Returns env variable for call
  * @param call is env variable without leading $ (HOME instead of $HOME)
  * @param bucket container of env calls and their results
  **/
string getEnv(const string& call, vector<array<string, 2>>& bucket){
	enum{
		var, //source
		ret  //return
	};

	for (const array<string, 2>& str : bucket){
		if (str[var] == call){return str[ret];}
	}
	bucket.push_back({call, getenv(call.c_str())});

	return bucket.back()[ret];
}

/**
  * ReadLine() helper function, parses <word> and calculates value of <maxSize>
  * @param word string to be parsed
  **/
void getMaxSize(string& word){
	//abbreviations
	const char a[4] = {'K', 'M', 'G', 'B'};
	//multipliers
	const uint64_t m[4] = {((uint64_t)1 << 10), ((uint64_t)1 << 20),
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
	const uint64_t wordInt = std::stoll(word.substr(0, word.length() - 2));

	if (!wordInt){
		error("MaxSize must have non 0 multiplier");
	}

	for (uint8_t idx = 0; idx < 4; ++idx){
		if (*(word.end() - 2) != a[idx]){continue;}

		maxSize = wordInt * m[idx];

		//check for overflow
		if (maxSize < wordInt || maxSize < m[idx]){
			word.push_back('\"');
			error("MaxSize multiplier causes overflow \"",word.c_str(), UINT8_MAX);
		}

		return;
	}

	const string detail = {*(word.end() - 2), word.back(), '\"'};
	error("invalid MaxSize abbreviation \"", detail.c_str(), *(word.end() - 2));
}

/**
  * ReadConfig() helper function, parses line and assigns them to shared.cpp values
  * @param bucket string* is formatted as {env variable, returned value}
  * @param comType type of compression to be used for archives
  **/
void ReadLine(string& line, vector<array<string, 2>>& bucket, compression& comType){
	if (line.empty() || line.front() == '#'){
		return;
	}

	string word;
	size_t i;

	for (i = line.find_first_of('=') + 1; i < line.length(); ++i){
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
	switch (line.front()){
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
					const array<uint8_t, 2> arr = getCompressionValue(word);

					if (arr[0] == UINT8_MAX){genericError(line);}

					//if (arr[0] is 0 or arr[1] is non-zero) and (comType is not xz)
					//allow for empty comType in case comType line is yet to come
					if ((arr[0] == 0 || arr[1]) && comType != 3){
						error("invalid CompressionLevel with CompressionType", "");
						exit(1); //unnecessary since error will exit but silences IDE warnings
					}

					comArgs = "-" + std::to_string(arr[0]);
					if (arr[1]){comArgs += " --extreme";}

					break;
				}

				//comType
				case 'T':{
					if (word.length() > 1 || !isdigit(word.front())){genericError(line);}
					comType = static_cast<compression>(word.front() - '0');
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

/**
  * ReadConfig() helper function, sets <comArgs> and <com_ext>
  * @param com_type type of compression to be expected
  **/
void setCommandVars(const compression com_type){
	//fix empty comArgs
	if (com_type == tar){
		comArgs.clear();
		conExt = ".tar";
		return;
	}

	if (comArgs.empty()){
		const char val = static_cast<char>('0' + (com_type != xz));
		comArgs = {'-', val, '\0'};
	}
	else{
		removeQuotes(comArgs);
	}

	//insert main compression call string to comArgs
	switch (com_type){
		case gzip:{
			comArgs = "gzip " + comArgs + " --stdout";
			conExt = ".tar.gz";
			break;
		}

		case bzip2:{
			comArgs = "bzip2 --compress " + comArgs + " --stdout";
			conExt = ".tar.bz2";
			break;
		}

		case xz:{
			comArgs = "xz --compress --threads=0 " + comArgs + " --stdout";
			conExt = ".tar.xz";
			break;
		}

		default:{
			std::cout << "Error: Invalid compression type." << std::endl;
			exit(com_type);
		}
	}

	comArgs.shrink_to_fit();
}

void ReadConfig(){
	//read config and assign values to global vars
	{
		std::ifstream ifs;
		ifs.open(config);
		if (!ifs.is_open()){error("cannot read config");}

		vector<array<string, 2>> bucket{{"HOME",home.string()}};

		compression com_type; //id for compression type
		for (string line; getline(ifs, line);){
			ReadLine(line, bucket, com_type);
		}
		setCommandVars(com_type);
	}

	//fix empty maxSize
	if (!maxSize){
		//5 * 2^30, 5'368'709'120
		maxSize = 0b1'0100'0000'0000'0000'0000'0000'0000'0000;
	}

	//check and insert <bacDir> to <blacklist>
	if (std::none_of(
		blacklist.begin(),
		blacklist.end(),
		[&](const path& p)->bool{return p == bacDir;}
	)){
		blacklist.push_back(bacDir);
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
	for (const string& i : text){ofs << i << '\n';}

	ofs.close();
}

//help text
void outputHelp(){
	std::cout << "FlexibleBackup\nBackup files using tar and a chosen compressor\n\nOptions:\n";

	const vector<string> body = {
			"  -c, --config=PATH", "Load config information from PATH",
			"  -h, --help", "Display this message",
			"  -r PATH, --root=PATH", "Set BackupRootDir to PATH",
			"  -o PATH, --output=PATH", "Set OutputLocation to PATH",
			"  --maxSize=NB", "Set max size a directory can be where N is a number and B is KB to TB",
			"  -B PATH, --blacklist=PATH", "Add PATH to blacklist",
			"  -W PATH, --whitelist=PATH", "Add PATH to Whitelist",
			"  -S PATH, --splitlist=PATH", "Add PATH to SplitBackup",
			"  -C PATH, --collective=PATH", "Add PATH to CollectiveBackup"
			//"  --compressLevel=N", "Set CompressionLevel to N",
			//"  --compressType=N", "Set CompressionType to N"
	};

	for (uint32_t i = 0; i < body.size(); i = i + 2){
		std::cout << std::setw(30) << std::left
		          << body.at(i) << body.at(i + 1) << '\n';
	}
}

bool GetArgsHelper(const string& arg, const array<array<string, 9>, 2>& options){
	if (arg.size() < 2 || arg.front() != '-'){return false;}

	bool multiChar;
	string argRoot, o;

	//single character format (-h)
	if (arg.at(1) != '-'){
		argRoot = arg.at(1);
		o = arg.substr(3, arg.length() - 3);
		multiChar = false;
	}
	//multi character format (--help)
	else{
		const uint32_t eq = arg.find_first_of('=', 2);
		argRoot = arg.substr(2, eq - 2);
		o = arg.substr(eq + 1, arg.size() - (eq + 1));
		multiChar = true;
	}

	if (o.empty()){return false;}

	uint8_t i = 0;
	for (; i < 11; i++){
		if (options[multiChar][i] == argRoot){break;}
	}

	switch(i){
		case 0:{
			config = o;
			break;
		} //config
		case 1:{
			outputHelp();
			exit(0);
		} //help
		case 2:{
			bacRoot = o;
			break;
		} //backup root
		case 3:{
			bacDir = o;
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

void GetArgs(const int argc, const char* argv[]){
	const array<array<string, 9>, 2> options = {
			"c", "h", "r",
		   "o", "", "B",
		   "W", "S", "C",
			"config", "help", "root",
			"output", "maxSize", "blacklist",
			"whitelist", "splitlist", "collective"
	};

	for (int i = 1; i < argc; i++){
		if (!GetArgsHelper(string(argv[i]), options)){
			error("Invalid argument, ", argv[i]);
		}
	}
}
