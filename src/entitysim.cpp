#include <algorithm>
#include "entitysim.h"
using namespace entity_sim;

double IEntitySimTool::JaccardSim(LRUCache &iNodeCache, vector<string> &vList2)
{
    map<string, node*> mCache = iNodeCache.getKeyMap();

    double dScore = 0.0;
    double totalScore = 0.0;
    double curScore = 0.0;
    int nCnt = 0;
    map<string, int> mMatchEntities;
    for (int i = 0; i < vList2.size(); i++)
    {
        string entity = vList2[i];
        if (mMatchEntities.find(entity) != mMatchEntities.end())
            continue;
        if (entity == "NULL")
            continue;
        nCnt += 1;
        if (mCache.find(entity) != mCache.end())
        {
            int nScore = mCache[entity]->value;
            curScore += nScore;
            mMatchEntities[entity] = 1;
        }
        else
            totalScore += 1;
    }

    map<string, node*>::iterator it;
    for (it = mCache.begin(); it != mCache.end(); ++it)
    {
        string entity = it->first;
        if (entity == "NULL")
            continue;
        int score = it->second->value;
        totalScore += score;
    }


    if (totalScore == 0 && nCnt == 0)
        return 1;
    else if (totalScore > 0 && curScore > 0)
        return curScore / totalScore;
    else
        return 0;

    /*double dScore = 0.0;
    int nLen1 = vList1.size();
    int nLen2 = vList2.size();
    int maxLen;
    if (nLen1 > nLen2)
        maxLen = nLen1;
    else
        maxLen = nLen2;
    vector<string> vIntersecRes(maxLen, "");
    vector<string> vUnionRes(nLen1 + nLen2, "");
    vector<string>::iterator it;
    it = set_intersection(vList1.begin(), vList1.end(), vList2.begin(),
                         vList2.end(), vIntersecRes.begin());
    vIntersecRes.resize(it - vIntersecRes.begin());

    it = set_union(vList1.begin(), vList1.end(), vList2.begin(),
                         vList2.end(), vUnionRes.begin());
    vUnionRes.resize(it - vUnionRes.begin());

    int nIntersec = vIntersecRes.size();
    int nUnion = vUnionRes.size();

    if (nUnion == 0)
        return dScore;
    dScore = double(nIntersec) / double(nUnion);
    return dScore;*/
}
