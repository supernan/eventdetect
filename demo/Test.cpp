#include <iostream>
#include <fstream>
#include "eventdetect.h"

using namespace std;
using namespace event_detect;
map<string, string> g_file_map;
int g_file_idx = 0;

void output2file(vector<pstWeibo> &docs, ofstream &out)
{
    for (int j = 0; j < docs.size(); j++)
    {
        out<<docs[j]->source<<endl;
    }
    out<<"========"<<endl;
    out.close();
}

int fn_iInitWeiboDataFromFile(const char *dataPath, vector<Weibo> &weibo)
{
		//check path
	if(NULL == dataPath)
	{
		cerr<<"weibo data path error"<<endl;
		return 1;
	}

		//open file
	ifstream in(dataPath);
	if(!in)
		return 1;

		//parse data
	string record;
	int counter = 0;
	while(getline(in, record))
	{
		if(record == "")
			continue;
		Weibo new_doc;
		new_doc.id = counter;
		counter++;
		new_doc.source = record;
		weibo.push_back(new_doc);
	}
	in.close();
	return 0;
}

int main(int argc, char **argv)
{
	//test();
    FLAGS_log_dir="../logs/";
    google::InitGoogleLogging("eventdetect");
    if (argc < 2)
    {
        cout << "miss input file" << endl;
        return 1;
    }
    string confPath = "../conf/eventdetect.xml";
    vector<pstWeibo> res;
    string fpath = argv[1];
    ifstream input(fpath.c_str());
    int count = 0;
    string line;
    vector<Weibo> text;
	vector<pstWeibo> corpus;
    CEventTree model(confPath);
    while (getline(input, line))
    {
        count += 1;
        if ((count % 10000) == 0)
        {
            corpus.clear();
            vector<pstWeibo>().swap(corpus);
            for (int i = 0; i < text.size(); i++)
            {
                corpus.push_back(&text[i]);
            }
            vector<event> events;
            if (!model.DetectEvents(corpus, events))
            {
                cout<<"detect events failed" << endl;
                //return 1;
            }
            cout<<"end"<<endl;
            for (int i = 0; i < events.size(); i++)
            {
                if (events[i].m_vEventDocs.size() < 50)
                    continue;
                string eventKey = "";
                map<string, LRUCache> entites = events[i].m_EventEntitiesCache;
                map<string, LRUCache>::iterator it;
                bool bFlg = false;
                int entityCount = 0;
                for (it = entites.begin(); it != entites.end(); ++it)
                {
                    map<string, node*> curMap = it->second.getKeyMap();
                    map<string, node*>::iterator itmap;
                    for (itmap = curMap.begin(); itmap != curMap.end(); ++itmap)
                    {
                        eventKey += itmap->first;
                        eventKey += "||";
                        if (itmap->first != "NULL")
                        {
                            bFlg = true;
                            entityCount += 1;
                        }
                    }
                }
                if (!bFlg || entityCount < 2)
                    continue;
                if (g_file_map.find(eventKey) == g_file_map.end())
                {
                    stringstream sstr;
                    string sIdx;
                    sstr << g_file_idx;
                    sstr >> sIdx;
                    string path = "./data/events_";
                    path += sIdx;
                    g_file_idx += 1;
                    ofstream out(path.c_str());
                    vector<pstWeibo> docs = events[i].m_vEventDocs;
                    out << eventKey << endl;
                    output2file(docs, out);

                }
                else
                {
                    string path = g_file_map[eventKey];
                    ofstream out(path.c_str(), ios::app);
                    vector<pstWeibo> docs = events[i].m_vEventDocs;
                    output2file(docs, out);
                }
            }
            text.clear();
            vector<Weibo>().swap(text);
        }
        else
        {
            Weibo doc;
            doc.source = line;
            text.push_back(doc);
        }
    }





    /*fn_iInitWeiboDataFromFile(fpath.c_str(), text);
	vector<pstWeibo> corpus;
    for (int i = 0; i < text.size(); i++)
        corpus.push_back(&text[i]);

    CEventTree model(confPath);
    vector<event> events;
    //model.LoadTreeStructure();
    if (!model.DetectEvents(corpus, events))
    {
        cout<<"detect events failed" << endl;
        return 1;
    }
    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].m_vEventDocs.size() < 10)
            continue;
        cout<<i<<" "<<events[i].m_vEventDocs.size()<<endl;
        map<string, LRUCache> entites = events[i].m_EventEntitiesCache;
        map<string, LRUCache>::iterator it;
        bool bFlg = false;
        for (it = entites.begin(); it != entites.end(); ++it)
        {
            cout<<it->first<<endl;
            map<string, node*> curMap = it->second.getKeyMap();
            map<string, node*>::iterator itmap;
            for (itmap = curMap.begin(); itmap != curMap.end(); ++itmap)
            {
                if (itmap->first != "NULL")
                    bFlg = true;
                cout<<itmap->first<<" ";
            }
            cout<<endl;
        }
        if (!bFlg)
            continue;
        vector<pstWeibo> docs = events[i].m_vEventDocs;
        for (int j = 0; j < docs.size(); j++)
        {
            cout<<docs[j]->source<<endl;
        }
        cout<<"========"<<endl;
    }
    string newpath = "./newtree.xml";
    //model.LoadTreeStructure();
    model.SaveTreeStructure();*/
    return 0;
}

