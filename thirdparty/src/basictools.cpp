#include "basictools.h"

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
    bool basic_tools::Split(const string sSrc, const string sDelimit, vector<string> &vResult)
    {
	    if (0 == sSrc.size() || 0 == sDelimit.size())
		    return false;
	    vResult.clear();
	    char *source = const_cast<char*>(sSrc.c_str());
	    char *p = strtok(source, sDelimit.c_str());
	    while (p)
	    {
		    string sTemp = p;
		    vResult.push_back(sTemp);
		    p = strtok(NULL, sDelimit.c_str());
	    }
	    return true;
    }


    /*!
    *  \fn		void strip(string &str, const char *sym = "\r");
    *  \brief	strip the string with sym
    *  \param [in]	str : source text
    *  \param [out] sym : the character to remove
    *  \return	if strip sucessfully, return true, else retur  false
    *  \author	zhounan(zhounan@software.ict.ac.cn)
    *  \date	2016/10
    */
	void basic_tools::Strip(string &rStr, const char *pSym)
	{
		if (0 == rStr.size())
			return;
		string::iterator iter, start = rStr.begin(), end = rStr.end();
		for(iter = rStr.begin(); iter < rStr.end(); ++iter)
		{
			bool bMatch = false;
			for(int j = 0 ; j < strlen(pSym); ++j)
			{
				if(*iter == pSym[j])
				{
					bMatch |= true;
					break;
				}
			}
			if (!bMatch)
			{
				start = iter;
				break;
			}
		}
		if(iter == rStr.end())
		{
			rStr = "";
			return;
		}
		int i;
		for(iter = rStr.end(), i = 1; i <= rStr.size(); i++)
		{
			iter--;
			bool bMatch = false;
			for(int j = 0; j < strlen(pSym); ++j)
			{
				if(*iter == pSym[j])
				{
					bMatch |= true;
					break;
				}
			}
			if(!bMatch)
			{
				end = iter;
				break;
			}
		}
		rStr = rStr.assign(start, end+1);

		return;
	}
