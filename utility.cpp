#include "stdafx.h"
#include "utility.h"
#include <algorithm>
#include <assert.h>

using namespace std;

const vector<string> VEC_COMPANY_NAME_STRIP = { "'S", "LLC", ",", "INC", "." , "& "};
const vector<string> VEC_ADDRESS_STRIP = { "ST", "PKWY", "DR", "BLVD", "HWY", "AVE", "RD", "STREET", "#", "HIGHWAY" };

void processPhoneNumOrZip(string & num, bool numIsZip) {
	if (num.empty())
		return;
	vector<string> rmStrs = !numIsZip ? vector<string>({ "+1", ".", " ", "-", "(", ")" }) : vector<string>({ "-" });
	genericStrStrip(num, rmStrs);
}

void genericStrStrip(string & str, const vector<string> & pattern) {
	if (str.empty())
		return;
	for (const auto & i : pattern) {
		while (str.find(i) != string::npos) {
			str.erase(str.find(i), i.length());
		}
	}
	while (str.back() == ' ')
		str.erase(str.length() - 1, 1);
}

void streetStringStrip(string & str, const vector<string> & pattern) {
	if (str.empty())
		return;
	for (const auto & i : pattern) {
		string ending(" ");
		ending.append(i);
		if (str.length() > ending.length() && str.substr(str.length() - ending.length()) == ending) {
			str = str.substr(0, str.length() - ending.length());
			break;
		}
		string middle(" ");
		middle.append(i).append(" ");
		if (str.find(middle) != string::npos) {
			str.erase(str.find(middle), middle.length() - 1);	// leave a space
			break;
		}
	}
	while (str.back() == ' ')
		str.erase(str.length() - 1, 1);
}

void readCSV(vector<vector<string>> & entries, ifstream & ifs) {
	string line;
	while (getline(ifs, line)) {
		stringstream ss(line);
		string word;
		vector<string> lineContent;
		while (getline(ss, word, '|'))
			lineContent.push_back(word);
		entries.push_back(lineContent);
	}
}

string toUpper(const string & s) {
	string ret;
	locale loc;
	for (auto & i : s) {
		ret.push_back(toupper(i, loc));
	}
	return ret;
}

void splitStr(const std::string & s, char delimiter, vector<string> & strRead) {
	strRead.clear();
	stringstream ss(s);
	string word;
	vector<string> str1Content, str2Content;
	while (getline(ss, word, delimiter)) {
		if (!word.empty())
			strRead.push_back(word);
	}
}

int strDistance(const string & word1, const string & word2) {
	if (word1.empty())
		return word2.length();
	if (word2.empty())
		return word1.length();
	const int M(word1.size());
	const int N(word2.size());
	vector<vector<int>> helper(M + 1, vector<int>(N + 1, 0));
	// word1[0...i]->word2[0...j] = helper[i, j]
	// return helper[M, N]
	for (int i = 1; i <= N; ++i)
		helper[0][i] = i;
	for (int i = 1; i <= M; ++i)
		helper[i][0] = i;
	for (int i = 1; i <= M; ++i) {
		for (int j = 1; j <= N; ++j) {
			if (word1[i - 1] == word2[j - 1])
				helper[i][j] = helper[i - 1][j - 1];
			else {
				helper[i][j] = min(min(helper[i - 1][j] + 1, helper[i][j - 1] + 1), helper[i - 1][j - 1] + 1);
			}
		}
	}

	return helper[M][N];
}

int seqStrMatch(const string & word1, const string &word2) {
	// both word1, word2 should be upper case
	if (word1.empty() || word2.empty())
		return false;
	vector<vector<bool>> helper(word1.length(), vector<bool>(word2.length(), false));
	for (int i = 0; i < int(word1.length()); ++i) {
		for (int j = 0; j < int(word2.length()); ++j) {
			helper[i][j] = word1[i] == word2[j];
		}
	}
	int currentStreak(0);
	vector<int> totalStreak;
	for (int i = 0; i < int(word1.length()); ) {
		for (int j = 0; j < int(word2.length()); ) {
			while (i < int(word1.length()) && j < int(word2.length()) && helper[i][j]) {
				i++;
				j++;
				currentStreak++;
			}
			if (currentStreak > 1) {
				totalStreak.push_back(currentStreak);
				currentStreak = 0;
			}
			j++;
		}
		i++;
	}
	// the fewer the totalStreak and the higher each element, the more likely string match
	int minLength = min(int(word1.length()), int(word2.length()));
	int total(0);
	for (auto & i : totalStreak)
		total += i;
	int score = total * INT_SEQ_STR_MATCH_SCORE * pow(0.9, totalStreak.size() - 1) / minLength;
	return score;
}

bool singleStrEditDistMatch(const string & word1, const string & word2) {
	if (word1.empty() || word2.empty())
		return false;
	int diff = strDistance(word1, word2);
	int strLengthDiff = abs(int(word1.length()) - int(word2.length()));
	diff -= strLengthDiff;
	int minStrLength = min(word1.length(), word2.length());
	// Match 75%
	return diff * 4 <= minStrLength;
}

int zipCodeCheck(const string & zip1, const string & zip2) {
	if (zip1.empty() || zip2.empty() || (zip1.length() != 9 && zip1.length() != 5) || (zip2.length() != 9 && zip2.length() != 5))
		return 0;
	if (zip1 == zip2)
		return zip1.length() == 9 ? 2 : 1;
	else if (zip1.substr(0, 5) == zip2.substr(0, 5))
		return 1;
	else
		return 0;
}

int companyNameMatch(const string & s1, const string & s2)
{
	if (s1.empty() || s2.empty())
		return INT_SEQ_STR_MATCH_SCORE;
	int ret(0);
	string str1(toUpper(s1)), str2(toUpper(s2));
	// Special tweek. Strip offs
	//vector<string> toStrip = { "'S", "LLC", ",", "INC", ".", "&"};
	genericStrStrip(str1, VEC_COMPANY_NAME_STRIP);
	genericStrStrip(str2, VEC_COMPANY_NAME_STRIP);

	return seqStrMatch(str1, str2);
}

int streetMatch(const string &s1, const string &s2)
{
	if (s1.empty() || s2.empty())
		return 0;
	int ret(0);
	string str1(toUpper(s1)), str2(toUpper(s2));
	// Special tweek. Strip offs
	vector<string> toStrip = VEC_ADDRESS_STRIP;
	streetStringStrip(str1, toStrip);
	streetStringStrip(str2, toStrip);

	vector<string> s1Content, s2Content;
	// The first string of street is often the street number, which should be given more weight.
	string firstStr1 = str1.substr(0, str1.find(" "));
	string firstStr2 = str2.substr(0, str2.find(" "));
	if (isInteger(firstStr1) && isInteger(firstStr2)) {
		if (firstStr1 == firstStr2) {
			int s = seqStrMatch(str1.substr(str1.find(" ") + 1), str2.substr(str2.find(" ") + 1));
			return s + 15;
		}
		else
			return 0;
	}
	return seqStrMatch(str1, str2);
}

bool isInteger(const string & s) {
	if (s.empty())
		return false;
	int i(0);
	if (s[0] == '-')
		i = 1;
	while (i < int(s.length())) {
		if (!isdigit(s[i]))
			return false;
		i++;
	}
	return true;
}

void testUtilities()
{
	// TODO More test cases
	string num("+1(201)356-0909");
	processPhoneNumOrZip(num);
	assert(num == "2013560909");
	num = ("201 356 0909");
	processPhoneNumOrZip(num);
	assert(num == "2013560909");
	num = ("201.356.0909");
	processPhoneNumOrZip(num);
	assert(num == "2013560909");

	assert(toUpper("jldafupeou--98q88jlkj") == "JLDAFUPEOU--98Q88JLKJ");
	assert(toUpper("bob's 123tttbdf") == "BOB'S 123TTTBDF");

	string company("BOB'S MAKE & BAKE, INC");
	genericStrStrip(company, VEC_COMPANY_NAME_STRIP);
	assert(company == "BOB MAKE BAKE");

	string address("130 WEST ST");
	streetStringStrip(address, VEC_ADDRESS_STRIP);
	assert(address == "130 WEST");
	address = "HWY 325"; 
	streetStringStrip(address, VEC_ADDRESS_STRIP);
	assert(address == "HWY 325");
	address = "1325 MORGAN STREET APT 5";
	streetStringStrip(address, VEC_ADDRESS_STRIP);
	assert(address == "1325 MORGAN APT 5");

	vector<string> result;
	splitStr("a b  c  ", ' ', result);
	assert(result.size() == 3 && result[2] == "c");
	splitStr("    ", ' ', result);
	assert(result.size() == 0);

	assert(seqStrMatch("DOCUMENT", "DOCS") > 70);
	assert(seqStrMatch("INTERCOSTAL", "INTRACOSTAL") > 70);
	assert(seqStrMatch("ASIAN BLOSSOM", "BLOSSOM ASIAN") > 70);
	assert(strDistance("INTECOSTAL", "INTRACOSTAL") == 2);
	assert(seqStrMatch("SMILESAVERS PEDIATRIC DENTISTRY", "SMILE SAVERS PEDIATRIC") > 70);
	assert(seqStrMatch("SMILESAVERS", "SMILE") > 70);
	assert(seqStrMatch("130 WEST", "130 SOUTH") < 70);
	//assert(seqStrMatch("SMILESAVERS PEDIATRIC DENTISTRY", "SMILE SAVERS PEDIATRIC") > 70);

	assert(companyNameMatch("Bob's Furniture", "BOB FURNITURE") == INT_SEQ_STR_MATCH_SCORE);
	assert(companyNameMatch("Blossom Asian", "Asian Blossom") > 70);
	assert(companyNameMatch("Document", "DOCS") > 70);
	assert(companyNameMatch("Leapfrog Document Services Inc", "LEAPFROG DOCS") > 70);

	assert(streetMatch("130 WEST ST", "130 West Street") > 70);
	assert(streetMatch("130 WEST ST", "130 South St") < 70);
	assert(streetMatch("130 WEST ST", "130 South St") < 70);
	assert(streetMatch("1679 US HIGHWAY 395 N # C", "1679 US 395 N # C") > 70);
	assert(streetMatch("2236 NW 82ND AVE", "2236 82ND") > 70);
}