// MerchantFind.cpp : Defines the entry point for the console application.
//

/* Created by: Yi Long
	01/2017

	The program is an attempt at record linkage. It's created for a take home assignment from Pearl Captital. The input files are 2 CSV files both with 
	company information like name, phone number, zip code, etc. The goal is be able to link the record from both files while there is no unique identifier present
	in both files. The approached adopted here is Deterministic record linkage or rules-based record linkage. It simply picks a few columns that are present 
	in both files and fuzzy match each respectively. The one record with highest score (and also above threshold) will be considered a match. 
	For the time allocated on this project, I only picked Phone, Company Name, Zip code and Street Address as identifiers. The problem with rules-based record linkage
	is 1. It often needs to be preprocessed based on knowledge of data. Decrease of data quality leads noise in final score.
	2. Rules are also based on knowledge of data. Even a small increase of complexity of data can result in large increase in the number of rules.
	For future work, might be interested to move to probabilistic record linkage or machine learning.
	wiki reading: https://en.wikipedia.org/wiki/Record_linkage#Deterministic_record_linkage
*/

#include "stdafx.h"
#include <unordered_map>
#include <algorithm>
#include "utility.h"
#include <assert.h>
#include <map>

using namespace std;

const int INT_PHONE_SCORE(50);
const int INT_ZIP_SCORE(50);
const int INT_COMPANY_SCORE(40);	//name
const int INT_STREET_SCORE(40);	//name
const int INT_MIN_SCORE_1(80);	// minimum score for match, if phone, zip, company matches are performed.

int main(int argc, char *argv[])
{
	testUtilities();

	if (argc != 3) {
		cout << "Please provide PCBF.csv file and UCC Filings.csv file respectively" << endl;
		return -1;
	}
	ifstream fPCBF(argv[1]), fUCCfile(argv[2]);
	if (!fPCBF.is_open()) {
		cout << "Can't open " << argv[1] << endl;
		return -1;
	}
	if (!fUCCfile.is_open()) {
		cout << "Can't open " << argv[2] << endl;
		return -1;
	}

	vector<vector<string>> PCBFentries;	// PCBF entries are unique
	unordered_map<string, int> PCBFDbaIndexMap; // A map from PCBF dba name to PCBFentries index
	unordered_map<string, string> PCBFphoneMerchant;	// phone number to merchant name (dba) map

	unordered_map<string, int> merchantUCCFilings;	// Our ultimate goal, a company name <-> UCC filing count map
	int totalMatchedUCCRecord(0);

	// Important column index, we only care about a handlful columns. If there are too many will use a data structure.
	int intPCBFdba(0), intPCBFphone(0), intPCBFZip, intPCBFStreet; // column is 0 based, PCBFentries[x][PCBFdba] will give you Dba
	int intUCCcompany(0), intUCCPhone(0), intUCCZip, intUCCStreet;
	string line;
	// Process PCBF header
	{
		getline(fPCBF, line);
		stringstream ss(line);
		string word;
		int c(0);
		while (getline(ss, word, '|')) {
			if (word == "Dba")
				intPCBFdba = c;
			else if (word == "Phone")
				intPCBFphone = c;
			else if (word == "BillingPostalCode")
				intPCBFZip = c;
			else if (word == "BillingStreet")
				intPCBFStreet = c;
			c++;
		}
	}
	assert(intPCBFdba > 0 && intPCBFphone > 0 && intPCBFZip > 0 && intPCBFStreet > 0);

	// Process PCBF unique merchant entries
	while (getline(fPCBF, line)) {
		stringstream ss(line);
		string word;
		PCBFentries.push_back(vector<string>());
		while (getline(ss, word, '|')) {
			// Might need to make word upper case here
			int currentColumn = PCBFentries.back().size();
			if (currentColumn == intPCBFphone || currentColumn == intPCBFZip) {
				// process phone or zip, remove +1 . - etc
				processPhoneNumOrZip(word, currentColumn == intPCBFZip);
			}
			PCBFentries.back().push_back(word);
		} // Read PCBF row finished
		string merchant(PCBFentries.back()[intPCBFdba]), phonNum(PCBFentries.back()[intPCBFphone]), zip(PCBFentries.back()[intPCBFZip]);
		merchantUCCFilings[merchant] = 0;
		if (phonNum.length() == 10)
			PCBFphoneMerchant[phonNum] = merchant;
		else
			cout << "Merchant " << merchant << " has erroneous phone number: " << phonNum << endl;
			
		//if (zip.length() == 9 || zip.length() == 5)
		//	PCBFZipMerchant[zip] = merchant;
		//else
		//	cout << "Merchant " << merchant << " has erroneous zip code: " << zip << endl;
		//PCBFDbaIndexMap[toUpper(merchant)] = PCBFentries.size() - 1;
		PCBFDbaIndexMap[merchant] = PCBFentries.size() - 1;

	}	// / while (getline(fPCBF, line))
	fPCBF.close();

	// Process UCC header
	{
		getline(fUCCfile, line);
		stringstream ss(line);
		string word;
		int c(0);
		while (getline(ss, word, '|')) {
			if (word == "IUSA COMPANY NAME")
				intUCCcompany = c;
			else if (word == "IUSA PHONE NUMBER")
				intUCCPhone = c;
			else if (word == "IUSA MAIL ZIP MERGE")
				intUCCZip = c;
			else if (word == "IUSA MAILING ADDRESS")
				intUCCStreet = c;
			c++;
		}
	}
	assert(intUCCPhone > 0 && intUCCZip > 0 && intUCCcompany > 0 && intUCCStreet > 0);

	string lastCheckUCCId, lastCheckPCBFName;	// UCC are sorted, so likely one line has same merchant as previous one. In this case if previous one found a match, no need to check again.
	while (getline(fUCCfile, line)) {
		stringstream ss(line);
		string word;
		vector<string> uccRow;
		bool phoneMatch(false);
		int companyMatchScore(-1);
		while (getline(ss, word, '|')) {
			// Might need to make word upper case
			int currentColumn = uccRow.size();
			if (currentColumn == intUCCPhone || currentColumn == intUCCZip) {
				// process phone, remove +1 . - etc
				processPhoneNumOrZip(word, currentColumn == intUCCZip);
			}
			uccRow.push_back(word);
		} // Read UCC row finished.

		// Now search for match
		int score(0);	// phone: 50, zip: 50/35, companyName: <= 50
		string strUCCPhone(uccRow[intUCCPhone]), strUCCZip(uccRow[intUCCZip]), strUCCcompany(uccRow[intUCCcompany]), strUCCStreet(uccRow[intUCCStreet]);
		if (strUCCPhone.empty() && strUCCcompany.empty())	// Bad record.
			continue;
		if (!lastCheckPCBFName.empty() && lastCheckUCCId == strUCCcompany) {
			merchantUCCFilings[lastCheckPCBFName] ++;
			totalMatchedUCCRecord++;
			//cout << "Just checked UCC company " << strUCCcompany << ", no need to perform check again." << endl;
			continue;
		}
		//cout << "Read UCC company " << strUCCcompany << endl;
		// Phone number is a strong match, use phone number map to speed things up rather than iterate through all PCBF entries
		unordered_map<string, string>::const_iterator it = PCBFphoneMerchant.find(strUCCPhone);
		if (it != PCBFphoneMerchant.end()) {
			score += INT_PHONE_SCORE;
			int entryIndex = PCBFDbaIndexMap[it->second];
			string strPCBFMerchantZip = PCBFentries[entryIndex][intPCBFZip];
			string strPCBFMerchantName = PCBFentries[entryIndex][intPCBFdba];
			int zipCodeCheckResult = zipCodeCheck(strUCCZip, strPCBFMerchantZip);
			int zipScore(0);
			if (zipCodeCheckResult == 2)
				zipScore += INT_ZIP_SCORE;
			else
				zipScore += 0 ? 0 : (INT_ZIP_SCORE / 2);
			score += zipScore;
			int companyNameScore = companyNameMatch(strPCBFMerchantName, strUCCcompany) * INT_COMPANY_SCORE / INT_SEQ_STR_MATCH_SCORE;
			score += companyNameScore;
			if (score > INT_MIN_SCORE_1) {
				merchantUCCFilings[strPCBFMerchantName] ++;
				totalMatchedUCCRecord++;
				lastCheckUCCId = strUCCcompany;
				lastCheckPCBFName = strPCBFMerchantName;
			}
			else {
				//cout << "Phone numbers match for " << strPCBFMerchantName << ", but total score not hight enough."
				//	<< " UCC company name " << uccRow[intUCCcompany] << endl;
				cout << strPCBFMerchantName << ": phone - " << INT_PHONE_SCORE << ", zip - " << zipScore << ", name - " << companyNameScore
					<< ", UCC name: " << uccRow[intUCCcompany] << endl;
			}
		}
		else {
			// Iterate through all entries, can skip the ones that already found a match.
			//printTimeElapsed t;
			int maxScore(0), msZip(0), msName(0), msStreet(0);
			string bestMatchName;
			for (const auto & i : merchantUCCFilings) {
				if (i.second == 0) {
					score = 0;
					vector<string> & pcbfRow = PCBFentries[PCBFDbaIndexMap[i.first]];
					string strPCBFMerchantName(pcbfRow[intPCBFdba]), strPCBFMerchantZip(pcbfRow[intPCBFZip]), strPCBFStreet(pcbfRow[intPCBFStreet]);
					int zipCodeCheckResult = zipCodeCheck(strUCCZip, strPCBFMerchantZip);
					int zipScore = zipCodeCheckResult * INT_ZIP_SCORE / 2;
					score += zipScore;
					int companyNameScore = companyNameMatch(strPCBFMerchantName, strUCCcompany) * INT_COMPANY_SCORE / INT_SEQ_STR_MATCH_SCORE;
					int streetScore = streetMatch(strPCBFStreet, strUCCStreet) * INT_STREET_SCORE / INT_SEQ_STR_MATCH_SCORE;
					score += streetScore;
					if (score > maxScore) {
						maxScore = score;
						bestMatchName = strPCBFMerchantName;
						msZip = zipScore;
						msName = companyNameScore;
						msStreet = streetScore;
					}
				}
			}
			if (!bestMatchName.empty()) {
				cout << strUCCcompany << " matched to " << bestMatchName << " with score: " << maxScore << ", zip score: " << msZip << ", name score: " << msName
					<< ", address score: " << msStreet << endl;
				if (score > 50) {
					merchantUCCFilings[bestMatchName] ++;
					totalMatchedUCCRecord++;
				}
				lastCheckUCCId = strUCCcompany;
				lastCheckPCBFName = bestMatchName;
			}
		}
	}	// / while (getline(fUCCfile, line))
	//cout << "Total matched UCC record: " << totalMatchedUCCRecord << ", remaining UCC record " << endl;
	fUCCfile.close();

	// show top 10
	multimap<int, string> countCompanyMap;
	for (auto & i : merchantUCCFilings) {
		if (i.second != 0)
			countCompanyMap.insert(make_pair(i.second, i.first));
		if (countCompanyMap.size() > 10)
			countCompanyMap.erase(countCompanyMap.begin());
	}

	cout << endl;
	cout << "Total matched " << totalMatchedUCCRecord << " records in UCC." << endl;
	cout << "Top 10 filings: " << endl;
	for (multimap<int, string>::const_reverse_iterator it = countCompanyMap.rbegin(); it != countCompanyMap.rend(); ++it)
		cout << it->second << ": " << it->first << endl;

    return 0;
}

/*
	Record processing
*/
