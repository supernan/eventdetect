#include <iostream>
#include <fstream>
#include "eventdetect.h"

using namespace std;
using namespace event_detect;

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
    vector<Weibo> text;
    vector<pstWeibo> res;
    string fpath = argv[1];
    fn_iInitWeiboDataFromFile(fpath.c_str(), text);
	vector<pstWeibo> corpus;
    for (int i = 0; i < text.size(); i++)
        corpus.push_back(&text[i]);

    CEventTree model(confPath, corpus);
    vector<event> events;
    if (!model.DetectEvents(corpus, events))
    {
        cout<<"detect events failed" << endl;
        return 1;
    }

    for (int i = 0; i < events.size(); i++)
    {
        cout<<i<<" "<<events[i].m_vEventDocs.size()<<endl;
        map<int, vector<string> > entites = events[i].m_EventEntitiesMap;
        map<int, vector<string> >::iterator it;
        for (it = entites.begin(); it != entites.end(); ++it)
        {
            cout<<it->first<<endl;
            for (int j = 0; j < it->second.size(); j++)
                cout<<it->second[j]<<" ";
            cout<<endl;
        }
        vector<pstWeibo> docs = events[i].m_vEventDocs;
        for (int j = 0; j < docs.size(); j++)
        {
            cout<<docs[j]->source<<endl;
        }
        cout<<"========"<<endl;
    }
    return 0;
}

