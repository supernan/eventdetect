#ifndef _EVENT_DETECT_H_
#define _EVENT_DETECT_H_

#include <vector>
#include <map>
#include <iostream>
#include "tinyxml.h"
#include "tinystr.h"
#include "glog/logging.h"
#include "nemodel.h"
#include "DataType.h"
using namespace std;
using namespace name_entity;
using namespace WeiboTopic_ICT;

#define MAX_NE_NUM 104
#define COUNTRY 100
#define LOC 101
#define PEOPLE 102
#define ORG 103
#define NONE 4

#define THRESHOLD 0.5

namespace event_detect
{

    /*
     * \struct > event
     * \brief > event struct
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    struct event
    {
        // docs
        vector<pstWeibo> m_vEventDocs;

        // event entites
        map<int, vector<string> > m_EventEntitiesMap;
    };

    /*
     * \struct > eventNode
     * \brief > event tree node
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    struct eventNode
    {
        // is the node leaf
        bool m_bIsLeaf;

        // entities of this node
        map<int, vector<string> > m_mID2Entites;

        // children node
        vector<eventNode*> m_vChildren;

        // split entity ID need to be initialize to -1
        int m_nEntityIdx;

        // current node depth
        int m_nCurDepth;

        // docs in this tree node
        vector<int> m_vDocIDs;
    };

    /*
     * \class > CEventTree
     * \brief > event detect tree leaf node is event docs
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    class CEventTree
    {
        public:
            /*
             * \fn > constructor and destructor
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            CEventTree(const string &rConfPath, const vector<pstWeibo> &vDocs);

            ~CEventTree();


            /*
             * \fn > DetectEvents
             * \brief > detect events from corpus
             * \param[in] rCorpus > docs
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool DetectEvents(vector<pstWeibo> &rCorpus, vector<event> &rEvents);


            /*
             * \fn > SaveTreeStructure
             * \brief > save tree model
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool SaveTreeStructure(string path);


            /*
             * \fn > LoadTreeStructure
             * \brief > load tree model
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool LoadTreeStructure();


        private:

            /*
             * \fn > __LoadConfigFile
             * \brief > load config files
             * \param[in] rConfPath > config file path
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __LoadConfigFile(const string &rConfPath);


            /*
             * \fn > __ParseEntitySeq
             * \brief > parse name entity sequence
             * \param[in] rNESeq > split entity sequence
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __ParseEntitySeq(const string &rNESeq);


            /*
             * \fn > __DestroyEventTree
             * \brief > release event tree
             * \param[in] pRoot > root node to destroy
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __DestroyEventTree(eventNode *pRoot);


            /*
             * \fn > __DocsEntityAnalysis
             * \brief > extract name entities from docs
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __DocsEntityAnalysis();


            /*
             * \fn > __GetEntitiesByID
             * \brief > get docs entities by entity_id
             * \param[in] vDocIDs > IDs of docs
             * \param[in] nEntityIdx > entity index
             * \param[out] entities > docs entities
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __GetEntitiesByID(vector<int> &vDocIDs, const int &nEntityIdx, map<int, vector<string> > &mEntityMap);


            /*
             * \fn > __SplitEventNode
             * \brief > split node depend on entity
             * \param[in] pRoot > root node to build
             * \param[in] nEntityIdx > entity index to split
             * \ret bool > whether function succeed
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            bool __SplitEventNode(eventNode *pRoot, int nEntityIdx);

            /*
             * \fn > __BuildEventTree
             * \brief > build event tree
             * \param[in] pRoot > root node to build
             * \param[in] nEntityIdx > entity to split
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __BuildEventTree(eventNode *pRoot, int nEntityIdx);


            /*
             * \fn > __TraverseEventNode
             * \brief > traverse event node get events
             * \param[in] pRoot > root node to build
             * \param[out] rEvents > events result
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __TraverseEventNode(eventNode *pRoot, vector<event> &rEvents);


            /*
             * \fn > __TreeSerialization
             * \brief > serialize the tree to string
             * \param[in] pRoot > root of tree
             * \param[out] rSerialRes > serialize result
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __TreeSerialization(eventNode *pRoot, string &rSerialRes);


            /*
             * \fn > __TreeToXMLElement
             * \brief > parse tree to xml file
             * \param[in] pRoot > root of tree
             * \param[out] pElement > xml node
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __TreeToXMLElement(eventNode *pRoot, TiXmlElement *pElement);


            /*
             * \fn > __XMLElementToTree
             * \brief > parse xml file to tree
             * \param[in] pRoot > root of tree
             * \param[out] pElement > xml node
             * \ret void
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            void __XMLElementToTree(eventNode *pRoot, TiXmlElement *pElement);


            /*
             * \fn > __TreeNodeEntityToString
             * \brief > make node   entities to string format
             * \param[in] pRoot > root
             * \ret string > result
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            string __TreeNodeEntityToString(eventNode *pRoot);


            /*
             * \fn > __StringToEntityMap
             * \brief > make string format to entities map
             * \param[in] sEntity > string to change
             * \ret string > result
             * \date > 2016/10
             * \author > zhounan(zhounan@software.ict.ac.cn)
             */
            map<int, vector<string> > __StringToEntityMap(const string &sEntity);


        private:

            // max depth of tree
            int m_nMaxTreeDepth;

            // save path
            string m_sSavePath;

            // root of event tree
            eventNode *m_pRootNode;

            // nemodel conf path
            string m_sNEConfPath;

            // nemodel pointer
            CNEModel *m_pNEModel;

            // split sequence
            vector<int> m_vSplitSeqs;

            // docs of event tree
            vector<pstWeibo> m_vDocs;

            // ne result of docs
            vector<neModelRes> m_vNERes;

    };
}


#endif
