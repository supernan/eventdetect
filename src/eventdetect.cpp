#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <sstream>
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
    TiXmlElement *pSaveNode = pSeqNode->NextSiblingElement();
    if (pSaveNode == NULL)
    {
        LOG(FATAL) << "pSaveNode is not set" << endl;
        return false;
    }
    m_sSavePath = pSaveNode->FirstChild()->Value();
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
    vector<string> sNEs;
    Split(rNESeq, sDelimit, sNEs);
    for (int i = 0; i < sNEs.size(); i++)
    {
        string sNE = sNEs[i];
        if (sNE.length() == 0)
            continue;
        if (!__IsEntityValid(sNE))
        {
            LOG(FATAL) << "NESeq is illegal out of boundry " <<sNE<< endl;
            return false;
        }
        m_vSplitSeqs.push_back(sNE);
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


CEventTree::CEventTree(const string &rConfPath)
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
    m_pNEModel = new CNEModel(m_sNEConfPath);
    if (m_pNEModel == NULL)
    {
        LOG(FATAL) << "NEModel Init Failed" << endl;
    }

    LOG(INFO) << "CEventTree Init" << endl;
    //m_sSavePath = "./tree.xml";
}


CEventTree::~CEventTree()
{
    __DestroyEventTree(m_pRootNode);
    delete m_pNEModel;
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
    cout<<"analysis end"<<endl;
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

    string sSplitEntity = m_vSplitSeqs[nEntityIdx];
    if (!__IsEntityValid(sSplitEntity))
    {
        LOG(ERROR) << "__GetEntitiesByID Failed sSplitEntity out of boundry" << sSplitEntity << endl;
        return false;
    }
    mEntityMap.clear();
    if (sSplitEntity == COUNTRY)
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
    }
    else if (sSplitEntity == LOC)
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
    }
    else if (sSplitEntity == PEOPLE)
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
    }
    else if (sSplitEntity == ORG)
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

    string sSplitEntity = m_vSplitSeqs[nEntityIdx];
    if (!__IsEntityValid(sSplitEntity))
    {
        LOG(ERROR) << "__SplitEventNode Failed sSplitEntity Error " << sSplitEntity << endl;
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
            vector<string> vNodeEntities = pRoot->m_vChildren[j]->m_mID2Entites[sSplitEntity];
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

            pChild->m_mID2Entites[sSplitEntity] = vDocEntities;
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
        e.m_sEventID = __TreeNodeEntityToString(pRoot);
        map<string, vector<string> >::iterator it;

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
        rSerialRes = "<node></node>";
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
        {
            sEntityInfo += "NULL;";
            continue;
        }
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

    stringstream sstr;
    string sEntityIdx;
    sstr << pRoot->m_nEntityIdx;
    sstr >> sEntityIdx;

    sEntityInfo = sEntityInfo + sEntityIdx + ";";

    rSerialRes = "<node><entity>" + sEntityInfo + "</entity>" + rSerialRes + "</node>";
    return;
}


string CEventTree::__TreeNodeEntityToString(eventNode *pRoot)
{
    string ret = "";
    for (int i = 0; i < m_vSplitSeqs.size(); i++)
    {
        string entity = m_vSplitSeqs[i];
        if (pRoot->m_mID2Entites[entity].empty())
        {
            ret += "NULL;";
            continue;
        }
        vector<string> entities = pRoot->m_mID2Entites[entity];
        string entityStr = "";
        for (int j = 0; j < entities.size(); j++)
        {
            if (entities[j].length() == 0)
                continue;
            if (j == entities.size()-1)
                entityStr += entities[j];
            else
            {
                entityStr += entities[j];
                entityStr += "||";
            }
        }
        ret += entityStr;
        ret += ";";
    }
    return ret;
}


map<string, vector<string> > CEventTree::__StringToEntityMap(const string &sEntity)
{
    map<string, vector<string> > ret;
    vector<string> vEntityTypes;
    string typeSep = ";";
    string entitySep = "||";
    Split(sEntity, typeSep, vEntityTypes);
    for (int i = 0; i < m_vSplitSeqs.size(); i++)
    {
        vector<string> entities;
        Split(vEntityTypes[i], entitySep, entities);
        ret[m_vSplitSeqs[i]] = entities;
    }
    return ret;
}


void CEventTree::__TreeToXMLElement(eventNode *pRoot, TiXmlElement *pElement)
{
    if (pElement == NULL)
    {
        LOG(ERROR) << "__TreeToXMLElement Failed pElement is NULL" << endl;
        return;
    }
    if (pRoot == NULL)
    {
        LOG(ERROR) << "__TreeToXMLElement Failed pRoot is NULL" << endl;
        return;
    }

    string entityStr = __TreeNodeEntityToString(pRoot);
    string entityIdStr;
    stringstream sstr;
    sstr << pRoot->m_nEntityIdx;
    sstr >> entityIdStr;

    pElement->SetAttribute("entities", entityStr.c_str());
    pElement->SetAttribute("splitentity", entityIdStr.c_str());
    if (pRoot->m_bIsLeaf)
    {
        pElement->SetAttribute("isleaf", "1");
        return;
    }

    pElement->SetAttribute("isleaf", "0");
    vector<eventNode*> vChildren = pRoot->m_vChildren;
    for (int i = 0; i < vChildren.size(); i++)
    {
        eventNode *pChild = vChildren[i];
        TiXmlElement *pChildElement = new TiXmlElement("treenode");
        __TreeToXMLElement(pChild, pChildElement);
        pElement->LinkEndChild(pChildElement);
    }
}


void CEventTree::__XMLElementToTree(eventNode *pRoot, TiXmlElement *pElement)
{
    if (pRoot == NULL)
    {
        LOG(ERROR) << "__XMLElementToTree Failed pRoot is NULL" << endl;
        return;
    }
    if (pElement == NULL)
    {
        LOG(ERROR) << "__XMLElementToTree Failed pElement is NULL" << endl;
        return;
    }

    TiXmlAttribute *entityAttr  = pElement->FirstAttribute();
    TiXmlAttribute *splitAttr = entityAttr->Next();
    TiXmlAttribute *leafAttr = splitAttr->Next();

    string sEntity = entityAttr->Value();
    map<string, vector<string> > entityMap = __StringToEntityMap(sEntity);
    string sLeafFlg = leafAttr->Value();
    if (sLeafFlg == "1")
        pRoot->m_bIsLeaf = true;
    else
        pRoot->m_bIsLeaf = false;
    stringstream sstr;
    sstr << splitAttr->Value();
    sstr >> pRoot->m_nEntityIdx;
    pRoot->m_mID2Entites = entityMap;

    vector<TiXmlElement*> vChildElements;
    vector<eventNode*> vChildren;
    TiXmlElement* Element = pElement->FirstChildElement();
    while (Element)
    {
        eventNode *pChild = new eventNode();
        vChildren.push_back(pChild);
        vChildElements.push_back(Element);
        Element = Element->NextSiblingElement();
    }

    for (int i = 0; i < vChildren.size(); i++)
        __XMLElementToTree(vChildren[i], vChildElements[i]);
    pRoot->m_vChildren = vChildren;
}


bool CEventTree::__IsEntityValid(const string &sEntity)
{
    if (sEntity == COUNTRY)
        return true;
    else if (sEntity == PEOPLE)
        return true;
    else if (sEntity == LOC)
        return true;
    else if (sEntity == ORG)
        return true;
    else
        return false;
}


void CEventTree::__ReleaseTreeNodeDocIDs(eventNode *pRoot)
{
    if (pRoot == NULL)
        return;
    vector<eventNode*> vChildren = pRoot->m_vChildren;
    for (int i = 0; i < vChildren.size(); i++)
        __ReleaseTreeNodeDocIDs(vChildren[i]);
    vector<int>().swap(pRoot->m_vDocIDs);
    pRoot->m_vDocIDs.clear();
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

    m_vDocs = rCorpus;
    for (int i = 0; i < m_vDocs.size(); i++)
        m_pRootNode->m_vDocIDs.push_back(i);

    rEvents.clear();
    if (!__DocsEntityAnalysis())
    {
        LOG(ERROR) << "DetectEvents Failed __DocsEntityAnalysis Error" << endl;
        return false;
    }

    int nEntityIdx = 0;
    __BuildEventTree(m_pRootNode, nEntityIdx);
    __TraverseEventNode(m_pRootNode, rEvents);
    __ReleaseTreeNodeDocIDs(m_pRootNode);

    LOG(INFO) << "DetectEvents Succeed Events Number is " << rEvents.size() << endl;
    return true;
}


bool CEventTree::SaveTreeStructure()
{
    TiXmlDocument *treeDocument = new TiXmlDocument();
    TiXmlDeclaration *declaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    treeDocument->LinkEndChild(declaration);
    string sEntityArr = "";
    stringstream sstr;
    for (int i = 0; i < m_vSplitSeqs.size(); i++)
    {
        string sIdx =  m_vSplitSeqs[i];
        if (i == m_vSplitSeqs.size() - 1)
            sEntityArr += sIdx;
        else
        {
            sEntityArr += sIdx;
            sEntityArr += ",";
        }
    }
    TiXmlElement *treeElement = new TiXmlElement("tree");
    treeElement->SetAttribute("tree_entities", sEntityArr.c_str());
    TiXmlElement *RootElement = new TiXmlElement("treenode");
    __TreeToXMLElement(m_pRootNode, RootElement);
    treeElement->LinkEndChild(RootElement);
    treeDocument->LinkEndChild(treeElement);
    treeDocument->SaveFile(m_sSavePath.c_str());
    return true;
}


bool CEventTree::LoadTreeStructure()
{
    TiXmlDocument *treeDocument = new TiXmlDocument(m_sSavePath.c_str());
    treeDocument->LoadFile();
    if (treeDocument == NULL)
    {
        LOG(ERROR) << "LoadTreeStructure Failed file path is wrong " << m_sSavePath << endl;
        return false;
    }

    TiXmlElement* treeElement = treeDocument->RootElement();
    TiXmlAttribute *treeAttr = treeElement->FirstAttribute();
    string sEntityArr = treeAttr->Value();
    vector<string> vEntityIds;
    string sep = ",";
    stringstream sstr;
    Split(sEntityArr, sep, vEntityIds);
    m_vSplitSeqs.clear();
    for (int i = 0; i < vEntityIds.size(); i++)
    {
        m_vSplitSeqs.push_back(vEntityIds[i]);
    }
    TiXmlElement *RootElement = treeElement->FirstChildElement();
    if (m_pRootNode != NULL)
        delete m_pRootNode;
    m_pRootNode = new eventNode();
    __XMLElementToTree(m_pRootNode, RootElement);
    return true;
}

