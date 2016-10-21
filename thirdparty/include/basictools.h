#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
using namespace std;

namespace basic_tools
{
    /*!
    *  \fn		bool split(const string src, const char *delimit, vector<string> &result);
    *  \brief	split string with delimit, the relation between delimit is OR
    *  \param [in]	src : source text
    *  \param [in] delimit : seperate character
    *  \param [out] result : split result
    *  \return	if split sucessfully, return true, else retur  false
    *  \author	zhounan(zhounan@software.ict.ac.cn)
    *  \date	2016/10
    */
    bool Split(const string sSrc, const string sDelimit, vector<string> &vResult);


    /*!
    *  \fn		void strip(string &str, const char *sym = "\r");
    *  \brief	strip the string with sym
    *  \param [in]	str : source text
    *  \param [out] sym : the character to remove
    *  \return	if strip sucessfully, return true, else retur  false
    *  \author	zhounan(zhounan@software.ict.ac.cn)
    *  \date	2016/10
    */
	void Strip(string &rStr, const char *pSym);
}
