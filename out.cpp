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
		cout << string(pStr.length() + 21, '\b');
	}

	//stdout message
	cout << setw(17) << right << msg[step] << " : \"" << p << flush;

	//ofs tabs and message
	if (step == BACKED_UP || step == SKIPPING || step == DEEPER){
		const int32_t space = (depth * TAB) + 17;
		ofs << setw(space) << right << msg << " : \"" << p << '\n';
		cout << endl;
	}
}
