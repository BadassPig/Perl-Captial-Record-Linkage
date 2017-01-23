#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

using std::string;
using std::vector;
using std::ifstream;

// The score return by any matching function here might not be used directly in total score card.
const int INT_SEQ_STR_MATCH_SCORE(100);

/*
	Strip off extra characters +1 - ( ) . if @param num is phone number.
	Strip off - if @param num is zip code.	
*/
void processPhoneNumOrZip(string & num, bool numIsZip = false);

/*
	A naive generic string strip function that removes all in pattern.
	Be aware the function doesn't differentiate patter as single word or part of word.
*/
void genericStrStrip(string & str, const vector<string> & pattern);

/*
	Removed pattern is either at the end of in the middle surrounded by space.
*/
void streetStringStrip(string & str, const vector<string> & pattern);

/*
	Read CSV into memory.
*/
void readCSV(vector<vector<string>> & entries, ifstream & ifs);

/*
	Convert @param s to upper case.
*/
string toUpper(const string & s);

/*
	Split string based on delimiter
*/
void splitStr(const std::string & s, char delimiter, vector<string> & strRead);

/*
	Edit distance of 2 strings based on certain operations. Operations: insert, remove, replace.
*/
int strDistance(const string & word1, const string & word2);

/*
	Fuzzy match 2 strings, score based on number of matching substrings and it's length. Less number of matching substrings and longer matching substrings = higher score 
	Return a score with max 100
*/
int seqStrMatch(const string & word1, const string &word2);

/*
	Get match score based on edit distance
*/
bool singleStrEditDistMatch(const string & word1, const string & word2);

/*
	Similarity check of zip code, zips can have maximum 9 digit
	@return A score based zip comparison. 2 - 9-digit match, 1 - 5-digit match, 0 - other. 
*/
int zipCodeCheck(const string & zip1, const string & zip2);

/*
	A sequential string match function.
	@param s1, s2: Assuming they are company names
	@return A score:
*/
int companyNameMatch(const string & s1, const string & s2);

/*
	Try to match street address. With address number bonus, it could exceed INT_SEQ_STR_MATCH_SCORE
*/
int streetMatch(const string &s1, const string &s2);

/*
	Simple function to determine if a string is integer (not considering overflow or certain edge cases)
*/
bool isInteger(const string & s);

/*
	Print time between beginning and end of block in seconds, very rough estimate
*/
class printTimeElapsed {
public:
	printTimeElapsed() {
		time(&_start);
	}
	~printTimeElapsed() {
		time(&_end);
		std::cout << "Time elapsed " << difftime(_end, _start) << " seconds." << std::endl;
	}
private:
	time_t _start, _end;
};

/*
	Quick and dirty way of a simple test of some utility functions. Should really use unit test framework.
*/
void testUtilities();