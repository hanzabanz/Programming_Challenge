#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

#define MAX_COLUMNS 15
#define NO_AGE "Not given"
#define NO_ALLERGY "None"
#define FILE_NAME "inputfile1.txt"

int main(int argc, char *argv[]) {
	// variables for reading file
	char line[128];
	char* splitLine;
	const char* delim = ",";
	char* next;

	// header column indices
	int SURNAME_IND = -1;
	int FIRSTNAME_IND = -1;
	int SEX_IND = -1;
	int AGE_IND = -1;
	int BIRTHPLACE_IND = -1;
	int ALLERGIES_IND = -1;
	int counter = 0;

	// age statistics variables
	int male_avg_age = 0;
	int total_males = 0;
	int total_males_age = 0;
	int female_avg_age = 0;
	int total_females = 0;
	int total_females_age = 0;

	// patient info storage structures
	// based off what needs to be output
	multimap<string, string> male_list;
	multimap<string, string> female_list;
	multimap<string, multimap<string, string>> allergy_list;
	multimap<string, multimap<string, string>> birthplace_list;
	int p_counter = 0;
	int all_counter = 0;

	// open file
	FILE *file = fopen(FILE_NAME, "r");
	// read in header
	// check if format makes sense based on minimum length and header format
	fgets(line, 128, file);
	line[strlen(line) - 1] = '\0';
	splitLine = strtok_s(line, delim, &next);
	// find variable indices based on the headers
	while (counter < MAX_COLUMNS) {
		if (strcmp(splitLine, "Surname") == 0) {
			SURNAME_IND = counter;
		}
		else if (strcmp(splitLine, "First Name(s)") == 0) {
			FIRSTNAME_IND = counter;
		}
		else if (strcmp(splitLine, "Sex (M/F)") == 0) {
			SEX_IND = counter;
		}
		else if (strcmp(splitLine, "Age") == 0) {
			AGE_IND = counter;
		}
		else if (strcmp(splitLine, "Place of birth") == 0) {
			BIRTHPLACE_IND = counter;
		}
		else if (strcmp(splitLine, "Allergies") == 0) {
			ALLERGIES_IND = counter;
		}
		counter++;
		splitLine = strtok_s(NULL, delim, &next);
		if (splitLine == NULL || splitLine == "\n") {
			break;
		}
	}

	// make sure all headers exist
	if (SURNAME_IND == -1 || FIRSTNAME_IND == -1 || SEX_IND == -1 || AGE_IND == -1 || BIRTHPLACE_IND == -1 || ALLERGIES_IND == -1) {
		printf("Invalid file format: Missing information");
		return 1;
	}

	char* values[MAX_COLUMNS];
	char* tempSplit = "";
	char* newline;
	while (true) {
		counter = 0;
		// save patient objects for data
		// delineate based on commas
		// exceptions in files: age = '-'; birthplace = 'Not given', allergies = 'None'

		line[0] = '\0';
		fgets(line, 128, file); // read in next line
		if (line == NULL || (&line != NULL && line[0] == '\0')) {
			break;
		}
		line[strlen(line) - 1] = '\0'; // remove '\n' character
		if (line[0] == '#') { // ignore if line begins with '#'
			continue;
		}
		// begin tokenizing
		splitLine = strtok_s(line, delim, &next);
		while (counter < MAX_COLUMNS) {
			if (splitLine != NULL && splitLine[0] == '\n') { // check if end of the line
				break;
			}
			values[counter] = splitLine;
			splitLine = strtok_s(NULL, delim, &next);
			std::printf("%s\n", splitLine);
			counter++;
			if (splitLine == NULL) { // count number of allergies for this patient
				if (counter > ALLERGIES_IND && ALLERGIES_IND != -1) {
					all_counter = counter - ALLERGIES_IND;
				}
				break;
			}
		}

		string firstname = values[FIRSTNAME_IND];
		string surname = values[SURNAME_IND];
		string sex = values[SEX_IND];
		string age = values[AGE_IND];
		string birthplace = values[BIRTHPLACE_IND];

		// insert for sex
		string key = surname;
		string info = firstname + " " + surname + ", Age " + age;
		if (sex == "M") {
			male_list.insert(pair<string, string>(key, info));
			total_males++;
			try {
				male_avg_age = ((male_avg_age*total_males_age) + stoi(age)) / (total_males_age + 1);
				total_males_age++;
			}
			catch (exception e) {
				// do nothing; don't add invalid age to running average
			}
		}
		else {
			female_list.insert(pair<string, string>(key, info));
			total_females++;
			try {
				female_avg_age = ((female_avg_age*total_females_age) + stoi(age)) / (total_females_age + 1);
				total_females_age++;
			}
			catch (exception e) {
				// do nothing; don't add invalid age to running average
			}
		}

		// insert for allergies
		// key=allergy type; value=multimap of all the patients with that allergy
		for (int i = 0; i < all_counter; i++) {
			key = values[ALLERGIES_IND + i];
			if (key.compare("None") != 0) {
				if (allergy_list.count(key) <= 0) {
					multimap<string, string> temp;
					temp.insert(pair<string, string>(surname, firstname));
					allergy_list.insert(pair<string, multimap<string, string>>(key, temp));
				}
				else {
					auto search = allergy_list.find(key);
					if (search != allergy_list.end()) {
						search->second.insert(pair<string, string>(surname, firstname));
					}
				}
			}
		}

		// insert for birthplace
		// key=birthplace; value=multimap of all the patients with that birthplace
		key = birthplace;
		if (birthplace.compare("Not given") != 0) {
			if (birthplace_list.count(key) <= 0) {
				multimap<string, string> temp;
				temp.insert(pair<string, string>(surname, firstname));
				birthplace_list.insert(pair<string, multimap<string, string>>(key, temp));
			}
			else {
				auto search = birthplace_list.find(key);
				if (search != birthplace_list.end()) {
					search->second.insert(pair<string, string>(surname, firstname));
				}
			}
		}
	}

	// writing sex file
	ofstream sexFile;
	sexFile.open("sex_file.txt");
	sexFile << "There are ";
	sexFile << total_males;
	sexFile << " males patients and ";
	sexFile << total_females;
	sexFile << " female patients.\n\n";
	sexFile << "The male patients are\n";
	for (multimap<string, string>::iterator i = male_list.begin(); i != male_list.end(); i++) {
		sexFile << (*i).second;
		sexFile << "\n";
	}
	sexFile << "The average age of the male patients is ";
	sexFile << male_avg_age;
	sexFile << "\n\n";
	sexFile << "The female patients are\n";
	for (multimap<string, string>::iterator i = female_list.begin(); i != female_list.end(); i++) {
		sexFile << (*i).second;
		sexFile << "\n";
	}
	sexFile << "The average age of the female patients is ";
	sexFile << female_avg_age;
	sexFile << "\n";
	sexFile.close();

	// writing allergies file
	ofstream allergyFile;
	allergyFile.open("allergies_file.txt");
	for (multimap<string, multimap<string, string>>::iterator i = allergy_list.begin(); i != allergy_list.end(); i++) {
		multimap<string, string> x_list = (*i).second;
		allergyFile << (*i).first;
		allergyFile << ": ";
		for (multimap<string, string>::iterator x = x_list.begin(); x != x_list.end(); x++) {
			allergyFile << " ";
			allergyFile << (*x).second;
			allergyFile << " ";
			allergyFile << (*x).first;
			allergyFile << ",";
		}
		allergyFile << "\n";
	}
	allergyFile.close();

	// writing birthplace file
	ofstream birthFile;
	birthFile.open("birthplace_file.txt");
	for (multimap<string, multimap<string, string>>::iterator i = birthplace_list.begin(); i != birthplace_list.end(); i++) {
		multimap<string, string> x_list = (*i).second;
		birthFile << (*i).first;
		birthFile << ": ";
		for (multimap<string, string>::iterator x = x_list.begin(); x != x_list.end(); x++) {
			birthFile << " ";
			birthFile << (*x).second;
			birthFile << " ";
			birthFile << (*x).first;
			birthFile << ",";
		}
		birthFile << "\n";
	}
	birthFile.close();
}
