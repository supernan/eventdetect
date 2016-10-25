#ifndef _NEMODEL_H_
#define _NEMODEL_H_

#include <iostream>
#include <vector>
#include <list>
#include "ne.h"
#include "DataType.h"
#include "thpool.h"
#include "glog/logging.h"

using namespace std;
using namespace ICTLAP::CORE;
using namespace ICTLAP::CORE::NE;
using namespace WeiboTopic_ICT;

namespace name_entity
{

    class CNEModel;

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
     * \fn > ExtractEntityThreadFunc
     * \brief > extract entity thread function
     * \param arg > thread func arg
     * \ret void
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    void* ExtractEntityThreadFunc(void *arg);


    /*
     * \fn > Lock
     * \brief > lock the mutex when multi-thread function is running
     * \param [in] m_iNEModel > the object need to lock
     * \ret void
     * \date  > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    void Lock(CNEModel *m_iNEModel);


    /*
     * \fn > UnLock
     * \brief > unlock the mutex when multi-thread function is running
     * \param [in] m_iNEModel > the object need to unlock
     * \ret void
     * \date  > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    void UnLock(CNEModel *m_iNEModel);


    /*
     * \struct > neModelThreadParam
     * \brief > all params used by thread function
     * \date  > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    struct neModelThreadParam
    {
        vector<pstWeibo> *m_pCorpus;
        vector<neModelRes> *m_pRes;
        CNEModel *m_pNEModel;
        NEapi m_API;
        int m_nStart;
        int m_nEnd;
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
             * \pram[in] pAPI > nemodel api
             * \param[in] sText > text
             * \param[out] rRes > result
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool ExtractNameEntity(NEapi pAPI, pstWeibo pDoc, neModelRes &rRes);


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

        public:

            // mutex
            pthread_mutex_t m_iModelMutex;

        private:
            // NE api
            vector<NEapi> m_vNEHandlers;
            //NEapi m_hNEModel;
    };
}



#endif
