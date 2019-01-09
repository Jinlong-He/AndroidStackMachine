//
//  Parse.cpp
//  ASM
//
//  Created by 何锦龙 on 2018/8/22.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include "Parse.hpp"
using namespace atm;


Parse::Parse(const string& infoFileName, const string& transFileName)
{
    readInfoFile(infoFileName); 
    readTransitionGragh(transFileName);
}
void Parse::readInfoFile(const string& fileName)
{
    vector<string> strs;
    string str;
    ifstream file(fileName);
    unordered_map<string, Aft> str2Aft;
    if(!file) return;
    while(!file.eof())
    {
        getline(file, str);
        if(!str.empty())
            strs.push_back(str);
    }
    ID num = ceil(log2((strs.size() - 2) / 3 + 1));
    int id = 2;
    bool flag = false;
    ID aftId = 1;
    for(ID i = 0; i < strs.size(); i++)
    {
        if(strs[i] == "#MAIN")
        {
            flag = true;
            continue;
        }
        if(strs[i] == "NIAM#")
        {
            flag = false;
            continue;
        }
        string name = Utility::split(strs[i++], ":")[1];
        Lmd lmd;
        if(Utility::split(strs[i], ":")[1] == "std")
            lmd = lmd_std;
        else if(Utility::split(strs[i], ":")[1] == "stk")
            lmd = lmd_stk;
        else if(Utility::split(strs[i], ":")[1] == "stp")
            lmd = lmd_stp;
        else if(Utility::split(strs[i], ":")[1] == "sit")
            lmd = lmd_sit;
        else exit(1);
        Aft aft;
        string aftStr = "";
        vector<string> strVec = Utility::split(strs[++i], ":");
        if(strVec.size() > 1)
        {
            if (str2Aft.count(strVec[1]) == 0)
                str2Aft[strVec[1]] = "b" + to_string(aftId++);
            aftStr = str2Aft[strVec[1]]; 
        }
        if(aftStr.length() == 0 || aftStr == " ")
            //aft = "epsilon" +  ReplaceAll(name, ".", "");
            aft = "e" + to_string(aftId++);
        else
            aft = aftStr;
        Activity* act = new Activity(name, id++, lmd, aft);
        str2ActMap[name] = act;
        activities.insert(act);
        afts.insert(aft);
        if (flag) {
            mainActivity = act;
            mainAft = aft;
        }
    }
    file.close();
}
void Parse::readTransitionGragh(const string& fileName)
{
    vector<string> strs;
    string str;
    ifstream file(fileName);
    if(!file) return;
    while(!file.eof())
    {
        getline(file, str);
        if(!str.empty())
            strs.push_back(str);
    }
    ID id = 1;
    for(ID i = 0; i < strs.size(); i++)
    {
        vector<string> strVec = Utility::split(strs[i++], "-->");
        if(str2ActMap.count(strVec[0]) == 0 || str2ActMap.count(strVec[1]) == 0)
            continue;
        Activity* sAct = str2ActMap[strVec[0]];
        Activity* tAct = str2ActMap[strVec[1]];
        strVec.clear();
        if(strs[i].find(':') != strs[i].size() - 1)
            strVec = Utility::split(Utility::split(strs[i], ":")[1], " ");
        Flags flags;
        bool finish = false;
        for(string str : strVec)
        {
            if(str.find("FLAG_ACTIVITY_NEW_TASK") != -1)
                flags.push_back(NTK);
            else if(str.find("FLAG_ACTIVITY_CLEAR_TASK") != -1)
                flags.push_back(CTK);
            else if(str.find("FLAG_ACTIVITY_CLEAR_TOP") != -1)
                flags.push_back(CTP);
            else if(str.find("FLAG_ACTIVITY_REORDER_TO_FRONT") != -1)
                flags.push_back(RTF);
            else if(str.find("FLAG_ACTIVITY_SINGLE_TOP") != -1)
                flags.push_back(STP);
            else if(str.find("FLAG_ACTIVITY_MULTIPLE_TASK") != -1)
                flags.push_back(MTK);
            else if(str.find("FLAG_ACTIVITY_TASK_ON_HOME") != -1)
                flags.push_back(TOH);
            else if(str.find("finish") != -1)
                finish = true;
            else continue;
        }
        tAct -> addDegree();
        Action* action = new Action(sAct, tAct, flags, finish, id++);
        actions.insert(action);
        sAct -> addAction(action);
    }
    
    file.close();
}
