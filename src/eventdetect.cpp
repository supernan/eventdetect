#include <sstream>
#include <algorithm>
#include <fstream>
#include "eventdetect.h"
#include "basictools.h"
#include "entitysim.h"

using namespace event_detect;
using namespace basic_tools;
using namespace entity_sim;


bool CEventTree::__LoadConfigFile(const string &rConfPath)
{
    TiXmlDocument *pDocument = new TiXmlDocument(rConfPath.c_str());
    pDocument->LoadFile();
    stringstream sstr;
    if (pDocument == NULL)
    {
        LOG(FATAL) << "__LoadConfigFile Failed Config file is not found" << rConfPath << endl;
        return false;
    }
    TiXmlElement *pRootElement = pDocument->RootElement();
    if (pRootElement == NULL)
    {
        LOG(FATAL) << "__LoadConfigFile Failed Root Node is NULL Config file is error" <<rConfPath<< endl;
        return false;
    }

    TiXmlElement *pDepthNode = pRootElement->FirstChildElement();
    if (pDepthNode == NULL)
    {
        LOG(FATAL) << "Depth of event tree is not set" << endl;
        return false;
    }
    sstr.clear();
    string sMaxDepth = pDepthNode->FirstChild()->Value();
    sstr << sMaxDepth;
    sstr >> m_nMaxTreeDepth;

    if (m_nMaxTreeDepth <= 0 || m_nMaxTreeDepth > MAX_NE_NUM)
    {
        LOG(FATAL) << "Max Tree Depth is Wrong" << endl;
        return false;
    }

    TiXmlElement *pNENode = pDepthNode->NextSiblingElement();
    if (pNENode == NULL)
    {
        LOG(FATAL) << "NEModel path is not set" << endl;
        return false;
    }
    m_sNEConfPath = pNENode->FirstChild()->Value();

    TiXmlElement *pSeqNode = pNENode->NextSiblingElement();
    if (pSeqNode == NULL)
    {
        LOG(FATAL) << "NE Split sequence is not set" << endl;
        return false;
    }
    string sNEs = pSeqNode->FirstChild()->Value();
    if (!__ParseEntitySeq(sNEs))
    {
        LOG(FATAL) << "__ParseEntitySeq Failed " << endl;
        return false;
    }

    LOG(INFO) << "__LoadConfigFile Succeed" << endl;
    return true;
}


bool CEventTree::__ParseEntitySeq(const string &rNESeq)
{
    if (rNESeq.length() == 0)
    {
        LOG(FATAL) << "__ParseEntitySeq Failed rNESeq is empty" << endl;
        return false;
    }

    string sDelimit = ",";
    stringstream sstr;
    sstr.clear();
    vector<string> sNEs;
    Split(rNESeq, sDelimit, sNEs);
    for (int i = 0; i < sNEs.size(); i++)
    {
        string sNE = sNEs[i];
        if (sNE.length() == 0)
            continue;
        sstr.clear();
        int nNE;
        sstr << sNE;
        sstr >> nNE;
        if (nNE < 0 || nNE > MAX_NE_NUM)
        {
            LOG(FATAL) << "NESeq is illegal out of boundry " <<nNE<< endl;
            return false;
        }
        m_vSplitSeqs.push_back(nNE);
    }

    LOG(INFO) << "__ParseEntitySeq Succeed" << endl;
    return true;
}


void CEventTree::__DestroyEventTree(eventNode *pRoot)
{
    if (pRoot == NULL)
        return;

    vector<eventNode*> vChildren = pRoot->m_vChildren;
    for (int i = 0; i < vChildren.size(); i++)
        __DestroyEventTree(vChildren[i]);

    delete pRoot;
}


CEventTree::CEventTree(const string &rConfPath, const vector<pstWeibo> &vDocs)
{
    if (!__LoadConfigFile(rConfPath))
    {
        LOG(FATAL) << "Load Configfile Failed" << endl;
    }

    m_pRootNode = new eventNode;
    if (m_pRootNode == NULL)
    {
        LOG(FATAL) << "CEventTree Failed Memory allocate Failed" << endl;
    }
    m_pRootNode->m_bIsLeaf = false;
    m_pRootNode->m_nEntityIdx = 0;
    m_pRootNode->m_nCurDepth = 1;

    m_vDocs = vDocs;
    for (int i = 0; i < m_vDocs.size(); i++)
        m_pRootNode->m_vDocIDs.push_back(i);

    m_pNEModel = new CNEModel(m_sNEConfPath);
    if (m_pNEModel == NULL)
    {
        LOG(FATAL) << "NEModel Init Failed" << endl;
    }

    LOG(INFO) << "CEventTree Init" << endl;
}


CEventTree::~CEventTree()
{
    __DestroyEventTree(m_pRootNode);
}


bool CEventTree::__DocsEntityAnalysis()
{
    if (m_pNEModel == NULL)
    {
        LOG(ERROR) << "__DocsEntityAnalysis Error NEModel is not initialized" << endl;
        return false;
    }
    if (m_vDocs.empty())
    {
        LOG(WARNING) << "__DocsEntityAnalysis WARNING docs is empty" << endl;
        return true;
    }

    m_vNERes.clear();
    if (!m_pNEModel->BatchEntityExtract(m_vDocs, m_vNERes))
    {
        LOG(ERROR) << "__DocsEntityAnalysis Error BatchEntityExtract Failed" << endl;
        return false;
    }
    LOG(INFO) << "__DocsEntityAnalysis Succeed" << endl;
    return true;
}


bool CEventTree::__GetEntitiesByID(vector<int> &vDocIDs, const int &nEntityIdx, map<int, vector<string> > &mEntityMap)
{
    if (nEntityIdx < 0 || nEntityIdx >= m_vSplitSeqs.size())
    {
        LOG(ERROR) << "__GetEntitiesByID Failed nEntityIdx out of boundry" << nEntityIdx << endl;
        return false;
    }

    int nSplitEntity = m_vSplitSeqs[nEntityIdx];
    if (nSplitEntity < 0 || nSplitEntity >= MAX_NE_NUM)
    {
        LOG(ERROR) << "__GetEntitiesByID Failed nSplitEntity out of boundry" << nSplitEntity << endl;
        return false;
    }
    mEntityMap.clear();
    switch (nEntityIdx)
    {
        case COUNTRY:
        {
            for (int i = 0; i < vDocIDs.size(); i++)
            {
                int nID = vDocIDs[i];
                if (nID >= m_vNERes.size())
                {
                    LOG(ERROR) << "__GetEntitiesByID Error nID out of range" << endl;
                    continue;
                }
                mEntityMap[nID] = m_vNERes[nID].m_vCountries;
            }
            break;
        }
        case LOC:
        {
            for (int i = 0; i < vDocIDs.size(); i++)
            {
                int nID = vDocIDs[i];
                if (nID >= m_vNERes.size())
                {
                    LOG(ERROR) << "__GetEntitiesByID Error nID out of range" << endl;
                    continue;
                }
                mEntityMap[nID] = m_vNERes[nID].m_vLocs;
            }
            break;
        }
        case PEOPLE:
        {
            for (int i = 0; i < vDocIDs.size(); i++)
            {
                int nID = vDocIDs[i];
                if (nID >= m_vNERes.size())
                {
                    LOG(ERROR) << "__GetEntitiesByID Error nID out of range" << endl;
                    continue;
                }
                mEntityMap[nID] = m_vNERes[nID].m_vPeos;
            }
            break;
        }
        case ORG:
        {
            for (int i = 0; i < vDocIDs.size(); i++)
            {
                int nID = vDocIDs[i];
                if (nID >= m_vNERes.size())
                {
                    LOG(ERROR) << "__GetEntitiesByID Error nID out of range" << endl;
                    continue;
                }
                mEntityMap[nID] = m_vNERes[nID].m_vOrgs;
            }
            break;
        }
    }
    return true;
}


bool CEventTree::__SplitEventNode(eventNode *pRoot, int nEntityIdx)
{
    if (pRoot == NULL)
    {
        LOG(WARNING) << "__SplitEventNode Failed root is NULL" << endl;
        return false;
    }
    if (nEntityIdx < 0 || nEntityIdx >= m_vSplitSeqs.size())
    {
        LOG(ERROR) << "__SplitEventNode entityID out of boundry " << nEntityIdx << endl;
        return false;
    }
    vector<int> vDocIDs = pRoot->m_vDocIDs;
    map<int, vector<string> > mDocID2Entity;
    if (!__GetEntitiesByID(vDocIDs, nEntityIdx, mDocID2Entity))
    {
        LOG(ERROR) << "__SplitEventNode Failed __GetEntitiesByID Error" << endl;
        return false;
    }

    int nSplitEntity = m_vSplitSeqs[nEntityIdx];
    if (nSplitEntity < 0 || nSplitEntity >= MAX_NE_NUM)
    {
        LOG(ERROR) << "__SplitEventNode Failed nSplitEntity Error " << nSplitEntity << endl;
        return false;
    }

    for (int i = 0; i < vDocIDs.size(); i++)
    {
        int nDocID = vDocIDs[i];
        vector<string> vDocEntities = mDocID2Entity[nDocID];
        if (vDocEntities.empty()) // if no entities set it to NULL important
            vDocEntities.push_back("NULL");

        bool bMatch = false;
        for (int j = 0; j < pRoot->m_vChildren.size(); j++)
        {
            vector<string> vNodeEntities = pRoot->m_vChildren[j]->m_mID2Entites[nSplitEntity];
            if (vNodeEntities.empty())
                continue;
            double dScore = IEntitySimTool::JaccardSim(vNodeEntities, vDocEntities);
            if (dScore >= THRESHOLD)
            {
                bMatch = true;
                pRoot->m_vChildren[j]->m_vDocIDs.push_back(nDocID);
                break;
            }
        }
        if (!bMatch)
        {
            eventNode *pChild = new eventNode;
            pChild->m_mID2Entites = pRoot->m_mID2Entites;

            pChild->m_mID2Entites[nSplitEntity] = vDocEntities;
            pChild->m_nEntityIdx = nEntityIdx + 1;
            pChild->m_nCurDepth = pRoot->m_nCurDepth + 1;
            pChild->m_vDocIDs.push_back(nDocID);
            pChild->m_bIsLeaf = false;
            pRoot->m_vChildren.push_back(pChild);
        }
    }
    return true;
}


void CEventTree::__BuildEventTree(eventNode *pRoot, int nEntityIdx)
{
    if (pRoot == NULL)
        return;
    if (pRoot->m_vDocIDs.empty())
        return;
    if (nEntityIdx >= m_vSplitSeqs.size())
    {
        pRoot->m_bIsLeaf = true;
        return;
    }

   if (!__SplitEventNode(pRoot, nEntityIdx))
   {
       LOG(WARNING) << "__BuildEventTree Failed __SplitEventNode Error" << endl;
       return;
   }

   for (int i = 0; i < pRoot->m_vChildren.size(); i++)
   {
       __BuildEventTree(pRoot->m_vChildren[i], nEntityIdx + 1);
   }
}


void CEventTree::__TraverseEventNode(eventNode *pRoot, vector<event> &rEvents)
{
    if (pRoot == NULL)
        return;

    if (pRoot->m_bIsLeaf)
    {
        event e;
        vector<int> vDocIDs = pRoot->m_vDocIDs;
        vector<pstWeibo> vEventDocs;
        for (int i = 0; i < vDocIDs.size(); i++)
        {
            int nIdx = vDocIDs[i];
            if (nIdx < 0 || nIdx >= m_vDocs.size())
            {
                LOG(ERROR) << "__TraverseEventNode Error nIdx out of range" << endl;
                continue;
            }
            vEventDocs.push_back(m_vDocs[nIdx]);
        }
        e.m_vEventDocs = vEventDocs;
        e.m_EventEntitiesMap = pRoot->m_mID2Entites;
        map<int, vector<string> >::iterator it;

        bool bEmpty = true;
        for (it = pRoot->m_mID2Entites.begin(); it != pRoot->m_mID2Entites.end(); ++it)
        {
            if (!it->second.empty() && it->second[0] != "NULL")
            {
                bEmpty = false;
                break;
            }
        }
        if (!bEmpty)
            rEvents.push_back(e);
        return;
    }

    vector<eventNode*> vChildren = pRoot->m_vChildren;
    for (int i = 0; i < vChildren.size(); i++)
    {
        __TraverseEventNode(vChildren[i], rEvents);
    }

}


void CEventTree::__TreeSerialization(eventNode *pRoot, string &rSerialRes)
{
    if (pRoot == NULL)
        return;
    if (pRoot->m_bIsLeaf)
    {
        rSerialRes = "()";
        return;
    }

    vector<eventNode*> vChildren = pRoot->m_vChildren;
    vector<string> vChildRes(vChildren.size(), "");
    for (int i = 0; i < vChildren.size(); i++)
        __TreeSerialization(vChildren[i], vChildRes[i]);
    sort(vChildRes.begin(), vChildRes.end());
    for (int i = 0; i < vChildRes.size(); i++)
        rSerialRes += vChildRes[i];
    string sEntityInfo = "";
    for (int i = 0; i < m_vSplitSeqs.size(); i++)
    {
        vector<string> entities = pRoot->m_mID2Entites[m_vSplitSeqs[i]];
        if (entities.empty())
            continue;
        for (int j = 0; j < entities.size(); j++)
        {
            if (j == entities.size()-1)
                sEntityInfo += entities[j];
            else
            {
                sEntityInfo += entities[j];
                sEntityInfo += ",";
            }
        }
        sEntityInfo += ";";
    }
    rSerialRes = "([" + sEntityInfo + "]" + rSerialRes + ")";
    return;
}


bool CEventTree::DetectEvents(vector<pstWeibo> &rCorpus, vector<event> &rEvents)
{
    if (rCorpus.empty())
    {
        LOG(WARNING) << "DetectEvents WARNING Corpus is empty" << endl;
        return true;
    }

    if (m_pRootNode == NULL)
    {
        LOG(ERROR) << "DetectEvents Failed root node is NULL" << endl;
        return false;
    }

    rEvents.clear();
    if (!__DocsEntityAnalysis())
    {
        LOG(ERROR) << "DetectEvents Failed __DocsEntityAnalysis Error" << endl;
        return false;
    }

    int nEntityIdx = 0;
    __BuildEventTree(m_pRootNode, nEntityIdx);
    __TraverseEventNode(m_pRootNode, rEvents);

    LOG(INFO) << "DetectEvents Succeed Events Number is " << rEvents.size() << endl;
    return true;
}


bool CEventTree::SaveTreeModel()
{
    ofstream output(m_sSavePath.c_str());
    string sModelStr;
    __TreeSerialization(m_pRootNode, sModelStr);
    output << sModelStr;
    output.close();
    return true;
}
