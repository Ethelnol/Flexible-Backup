/** 
  * Created by Ethelnol on 17/09/2025
  **/

#include <fstream>

#include "out.h"
#include "shared.h"

constexpr uint8_t TAB = 4; //num spaces for tab
std::ofstream ofs;

void log_init(){
	ofs.open(bacDir.string() + "/flexible-backup.log");
	if (!ofs.is_open()){
		error("cannot open BackupDir/flexible-backup.log");
	}
}

void out(const uint32_t depth, const path& p, const STATUS step){
	using std::cout, std::setw, std::right, std::flush, std::endl;

	const string msg[NUM_STATES] = {
		"Scanning",
		"Backing up",
		"Already backed up",
		"Backed up",
		"Skipping",
		"Scanning deeper"
	};
	const string pStr = p.filename().string() + '\"';

	//stdout tabs or backspace
	if (step == SCANNING){
		cout << string(depth * TAB, ' ');
	}
	else{
		const string::size_type n = 3 * (pStr.length() + 21);
		string backspaces(n, '\b');
		for (auto i = 0; i < n; ++i){
			backspaces[(++i)++] = ' ';
		}
		cout << backspaces;
	}

	//stdout message
	const string::size_type space = 17 - msg[step].length();
	cout << string(space, ' ') << msg[step] << " : \"" << pStr << flush;

	//ofs tabs and message
	if (step != SCANNING && step != BACKING){
		ofs << string((depth * TAB) + space, ' ') << msg[step] << " : \"" << pStr << '\n';
		cout << '\n';
	}
}
