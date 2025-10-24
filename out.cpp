/** 
  * Created by Caitlyn Briggs on 17/09/2025
  **/

#include <algorithm>
#include <fstream>

#include "out.h"
#include "shared.h"

const uint8_t TAB = 4; //num spaces for tab
std::ofstream ofs;

enum STATUS{
	scanning, backing, archived, ended
};

void log_init(){
	ofs.open(bacDir.string() + "/flexible-backup.log");
	if (!ofs.is_open()){
		error("cannot open BackupDir/flexible-backup.log");
	}
}

/**
  * Returns "msg : \"p\"" to be output to log file
  * @pre msg_len is less than 18, largest msg is "Already backed up"
  **/
char* message(const char* msg, uint8_t msg_len, const path& p){
	if (!msg || msg_len > 17){return nullptr;}

	uint8_t msg_buffer = 17;

	//length of p, msg_buffer, 2 spaces, colon, 2 quotes, and NUL
	const uint32_t ret_len = p.string().length() + 23;
	char* ret = new char[ret_len];

	//uniform chars
	std::fill_n(ret, msg_buffer - msg_len, ' ');
	ret[17] = ' ';
	ret[18] = ':';
	ret[19] = ' ';
	ret[20] = '\"';
	ret[ret_len - 2] = '\"';
	ret[ret_len - 1] = '\0';

	//fill ret with msg
	while (msg_len){
		ret[--msg_buffer] = msg[--msg_len];
	}

	//fill ret with p
	msg_buffer = 21;
	for (char c : p.string()){
		ret[msg_buffer++] = c;
	}

	return ret;
}

void outChars(std::basic_ostream<char>& out, uint32_t len, char c){
	char ret[len + 1];
	std::fill_n(ret, len, c);
	ret[len] = '\0';

	out << ret;
}

/**
  * Outputs (depth * TABS) spaces, msg, and p depending on step
  * @param msg_len number of chars in msg such that msg has range of [0, msg_len - 1]
  * @param step [0] for scanning, [1] for skipping/scanning deeper, [2] for backing up, [3] for (already) backed up
  **/
void out(uint32_t depth, const char* msg, uint8_t msg_len, const path& p, STATUS step){
	//stdout tabs or backspace
	if (step == scanning){
		outChars(std::cout, depth * TAB, ' ');
	}
	else{
		outChars(std::cout, p.string().length() + 22, '\b');
	}

	//stdout message
	char* output = message(msg, msg_len, p);
	std::cout << output << std::flush;

	//ofs tabs and message
	if (step == archived || step == ended){
		outChars(ofs, depth * TAB, ' ');
		ofs << output << '\n';
		std::cout << std::endl;
	}

	delete[] output;
}

void Scanning(uint32_t depth, const path& p){
	out(depth, "Scanning", 8, p, scanning);
}

void Backing(uint32_t depth, const path& p){
	out(depth, "Backing up", 10, p, backing);
}

void Backed(uint32_t depth, const path& p){
	out(depth, "Backed up", 9, p, archived);
}

void A_Backed(uint32_t depth, const path& p){
	out(depth, "Already backed up", 17, p, archived);
}

void Skipping(uint32_t depth, const path& p){
	out(depth, "Skipping", 8, p, ended);
}

void Deeper(uint32_t depth, const path& p){
	out(depth, "Scanning deeper", 15, p, ended);
}
