#include <algorithm>
#include "entitysim.h"
using namespace entity_sim;

double IEntitySimTool::JaccardSim(vector<string> &vList1, vector<string> &vList2)
{
    double dScore = 0.0;
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
    return dScore;
}
