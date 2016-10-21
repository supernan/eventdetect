#ifndef _NEMODEL_H_
#define _NEMODEL_H_

#include <iostream>
#include <vector>
#include <list>
#include "ne.h"
#include "DataType.h"
#include "glog/logging.h"

using namespace std;
using namespace ICTLAP::CORE;
using namespace ICTLAP::CORE::NE;
using namespace WeiboTopic_ICT;

namespace name_entity
{

    /*
     * \struct > neModelRes
     * \brief > record different ne result
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    struct neModelRes
    {
        vector<string> m_vPeos;
        vector<string> m_vOrgs;
        vector<string> m_vLocs;
        vector<string> m_vCountries;
        vector<string> m_vStocks;
        vector<string> m_vTimes;

    };

    /*
     * \class > CNEModel
     * \brief > name entity model
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    class CNEModel
    {
        public:
            /*
             * \fn > constructor and destructor
             * \date > 2016/10
             * \author > zhounan(zhounan@sofrware.ict.ac.cn)
             */
            CNEModel(const string &rConfPath);

            ~CNEModel();

            /*
             * \fn > ExtractNameEntity
             * \brief > extract all of name entities from text
             * \param[in] sText > text
             * \param[out] rRes > result
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool ExtractNameEntity(pstWeibo pDoc, neModelRes &rRes);


            /*
             * \fn > BatchEntityExtract
             * \brief > extract all of name entities from docs
             * \param[in] rCorpus > docs
             * \param[out] vRes > result
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool BatchEntityExtract(vector<pstWeibo> &rCorpus, vector<neModelRes> &vRes);

        private:

            /*
             * \fn > ParseNEResult
             * \brief > parse ne result
             * \param[in] rText > text
             * \param[in] res > ne parse result
             * \param[out] rModelRes > result
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __ParseNEResult(const string &rText, NE::NEres res, neModelRes &rModelRes);

        private:
            // NE api
            NEapi m_hNEModel;
    };
}



#endif
