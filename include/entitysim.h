#ifndef _ENTITY_SIM_H_
#define _ENTITY_SIM_H_

#include <set>
#include <vector>
#include <iostream>
using namespace std;

namespace entity_sim
{
    /*
     * \class > IEntitySimTool
     * \brief > Entity List similarity interface
     * \date > 2016/10
     * \author > zhounan(zhounan@software.ict.ac.cn)
     */
    class IEntitySimTool
    {
        /*
        * \fn > JaccardSim
        * \brief > compare entity list with jaccard
        * \param[in] vList1 > List1
        * \param[in] vList2 > List2
        * \param[out] dScore > sim score
        * \ret bool > whether fucntion succeed
        * \date > 2016/10
        * \author > zhounan(zhounan@software.ict.ac.cn)
        */
        public:
            static double JaccardSim(vector<string> &vList1, vector<string> &vList);
    };
}

#endif
