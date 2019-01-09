
//  main.cpp
//  ATM
//
//  Created by 何锦龙 on 2018/8/20.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "Activity.hpp"
#include "Parse.hpp"
#include "ATM.hpp"
#include "ATMSolver.hpp"
#include <time.h>
#include <thread>
#include <regex>
#include <set>
#define ThreadCount 4 

using namespace atm;
using namespace std;


string getPathFromNuXmv(const string& smv, ID2ActionMap& map)
{
    string res = ""; 
    vector<string> states = Utility::split(smv, "-> State:");
    for (ID i = 1; i < states.size(); i++) {
        vector<string> strs = Utility::split(states[i], "c0 = v");
        if (strs.size() == 1) {
            res += "repeat\n";
            continue;
        }
        int j = 0;
        string num = "";
        while (strs[1][j] <= '9' && strs[1][j] >= '0')
            num.push_back(strs[1][j++]);
        char* end;
        int id =  static_cast<int>(strtol(num.c_str(),&end,10));
        if (id == 0) continue;
        if (id == 1) {
            res += "pop\n";
            continue;
        }
        res +=  "-->" + map[id] -> getTargetAct() -> getName() + "   " + num + " "+ to_string(map[id] -> getTargetAct() -> getID()) + "\n";
    }
    return res;
}
void getNuxmvRes(VerificationData data, int i)
{
    //cout << "thread" << i << " starts Nuxmv..." << endl;
    string fileName = "out" + to_string(i) + ".smv";
    ofstream out(fileName);
    if (out.is_open())
    {
        out << data.smv;
        out.close();
    }
    char buf_ps[2048];
    string result;
    FILE* ptr = NULL;
    regex patten_true(".*invariant.+true");
    regex patten_false(".*invariant.+false");
    string str = "./nuXmv/bin/nuXmv " + fileName;
    const char * commond = str.c_str();
    if((ptr = popen(commond, "r")) != NULL)
    {
        while(fgets(buf_ps, 2048, ptr) != NULL)
            result.append(buf_ps);
        pclose(ptr);
        ptr = NULL;
    }
    str = "rm " + fileName;
    commond = str.c_str();
    system(commond);
    //cout<<result<<endl;
    if(regex_search(result, patten_true)) *(data.value) = 0;
    else if (regex_search(result, patten_false)) {
        *(data.path) = getPathFromNuXmv(result, data.solver -> getID2ActionMap());
        *(data.value) = 1; 
    }
    else *(data.value) = -1;
}

void multiCalculation(VerificationDatas& datas, int num)
{
    cout << "start nuxmv... " << num << endl; 
    thread threads[ThreadCount];
    for(ID i = 0; i < ThreadCount; i++)
    {
        ID j = i + num * ThreadCount;
        if(j == datas.size()) break;
        //cout << "thread" << i << " is running..." << endl;
        datas[j].value = new int();
        datas[j].path = new string();
        threads[i] = thread(&getNuxmvRes, datas[j], i);
    } 
    for(ID i = 0; i < ThreadCount; i++)
    {
	ID j = i + num * ThreadCount;
	if(j == datas.size()) break;
        threads[i].join();
    }
}

void getBackPatten(VerificationDatas& datas, ofstream& outfile)
{
    outfile << "Back Patten is :" << endl;
    for (VerificationData data : datas) {
        if (*(data.value) == 1) {
            outfile << data.act -> getName() << endl;
            cout << data.act -> getName() << endl;
            outfile << *(data.path) << endl << endl;
        }
    }
    cout << endl;
}

void getLoop(VerificationDatas& datas, ofstream& outfile)
{
    outfile << "Loop is :" << endl;
    for (VerificationData data : datas) {
        if (*(data.value) == 1) {
            outfile << *(data.path) << endl << endl;
        }
    }
}

void writeFile(ofstream& outfile, string& fileName) {
    string info = fileName.replace(fileName.find("/"), 1, "_");
    //string outfileName = "res//" + info.replace(info.find("ActivityInfo"), 12, "res"); 
    string outfileName = "res" + info;
    outfile.open(outfileName, ios::app);
    outfile << outfileName << endl;
}

int main(int argc, const char * argv[]) {
    cout << "reading..." << endl;
    ofstream successfile;
    ofstream allfile;
    ofstream mostTrue;
    ofstream truefile;
    ofstream falsefile;
    successfile.open("success.txt", ios::app);
    allfile.open("all.txt", ios::app);
    mostTrue.open("mostTrue.txt", ios::app);
    truefile.open("true.txt", ios::app);
    falsefile.open("f.txt", ios::app);
    string commond = argv[1];
    string infoFile = argv[2];
    string transFile = argv[3];
    Parse parse(infoFile, transFile);
    char* end;
    int window =  static_cast<int>(strtol(argv[4],&end,10));
    if (window == 0) window = 100;
    ofstream outfile;
    string fileName = argv[2];
    writeFile(outfile, fileName);
    cout << fileName << endl;
    allfile << fileName << endl;
    allfile.close();
    if (commond == "-b") {
        ATM a (parse);
        a.mkConfig("", window);
        cout << a.getRes() << endl;
        if (a.getRes() == 1)
            truefile << fileName << endl;
        if (a.getRes() > 0)
            mostTrue << fileName << endl;
        if (a.getRes() == 0)
            falsefile<< fileName << endl;
        if (!a.isBounded())
            successfile << fileName << endl;
        truefile.close();
        mostTrue.close();
        falsefile.close();
        ATMSolver solver(&a);
        solver.init();
        if (! a.isBounded()) {
            VerificationDatas datas;
            solver.pre4GetLoop(datas);
            for (ID i = 0; i <= datas.size() / ThreadCount; i++)
                multiCalculation(datas, i);
            getLoop(datas, outfile);
        }
        exit(1);
        VerificationDatas datas;
        string targetName = argv[5];
        for (Activity* act : solver.getActivities()) {
            if (targetName.find(act -> getName()) != -1) {
                cout << act -> getName() << endl;
                outfile << act -> getName() << endl;
                solver.pre4BackPatten(datas, act);
                break;
            }
        }
        for (ID i = 0; i <= datas.size() / ThreadCount; i++)
            multiCalculation(datas, i);
        getBackPatten(datas, outfile);
        successfile << argv[2] << endl;
        successfile.close();
        outfile.close();
    } 
    //else {
    //    if (parse.getAfts().size() == 1) outfile << "one stack" << endl;
    //    VerificationDatas verificationdatas;
    //    vector<ATM*> asms;
    //    for (Aft  aft : parse.getAfts()) {
    //        if (aft != parse.getMainAft()) {
    //            Parse parse(argv[2], argv[3]);
    //            ATM* a = new ATM(parse);
    //            asms.push_back(a);
    //            a -> mkConfig(aft, window, flag);
    //            ATMSolver* solver = new ATMSolver();
    //            solver -> loadVars(a);
    //            VerificationDatas datas;
    //            if (commond == "-dbh") {
    //                solver -> preDetection4BackHijacking(datas, aft);
    //            } else if (commond == "-dth") {
    //                solver -> preDetection4TskReparentingHijacking(datas, aft);
    //            }
    //            verificationdatas.insert(verificationdatas.end(), datas.begin(), datas.end());
    //        }
    //    }
    //    if (verificationdatas.size() > 0) {
    //        cout << "there are " << verificationdatas.size() << " threads" << endl;
    //        for (ID i = 0; i <= verificationdatas.size() / ThreadCount; i++)
    //            multiCalculation(verificationdatas, i);
    //        getRiskLevel(verificationdatas, outfile);
    //    } else {
    //        outfile << "None" << endl;
    //    }
    //    successfile << argv[2] << endl;
    //    successfile.close();
    //    outfile.close();
    //    for (ATM* a : asms)
    //        delete a;
    //}
    return 0;
}

//
//

//
//

//

//
//void getMaxLength(VerificationDatas& datas, ofstream& outfile)
//{
//    for(VerificationData data : datas)
//        if(*(data.value) == 0)
//        {
//            //cout << "max Length is : " << data.length << endl;
//            outfile << "max Length is : " << data.length - 1 << endl;
//            return;
//        }
//    //cout << "max Length is : " << datas[datas.size() - 1].length << endl;
//    outfile << "max Length is : " << datas[datas.size() - 1].length << endl;
//    outfile << *(datas[datas.size() - 1].path) << endl;
//}
//
//void getRiskLevel(VerificationDatas& datas, ofstream& outfile)
//{
//    unordered_set<Activity*> ll0;
//    unordered_set<Activity*> ll1;
//    unordered_set<Activity*> ll2;
//    unordered_set<string> l0;
//    unordered_set<string> l1;
//    unordered_set<string> l2;
//    unordered_map<Aft, unordered_set<Activity*> > aft2DatasMap;
//    unordered_map<Activity*, string> act2StrMap;
//    for(VerificationData data : datas)
//    {
//        act2StrMap[data.act] = data.act -> getName() + ": \n" + *(data.path);
//        if(*(data.value) == 0)
//        {
//            ll0.insert(data.act);
//            continue;
//        }
//        aft2DatasMap[data.aft].insert(data.act);
//    }
//    for(auto it = aft2DatasMap.begin(); it != aft2DatasMap.end(); it++)
//    {
//        if(it -> second.size() == 1)
//            ll2.insert(it -> second.begin(), it -> second.end());
//        if(it -> second.size() > 1)
//            ll1.insert(it -> second.begin(), it -> second.end());
//    }
//
//    for(Activity* act : ll1)
//        if(ll2.count(act) == 0)
//            l1.insert(act2StrMap[act]);
//
//    for(Activity* act : ll0)
//        if(ll2.count(act) == 0 && ll1.count(act) == 0)
//            l0.insert(act2StrMap[act]);
//
//    for(Activity* act : ll2)
//        l2.insert(act2StrMap[act]);
//
//    //cout << "Hijacking Level0 : " << endl;
//    outfile << "Hijacking Level0 : " <<  l0.size() <<endl;
//    for(string str : l0)
//        //cout << act -> getName() << " , ";
//        outfile << str << endl;
//    //cout << endl << "Hijacking Level1 : " << endl;
//    outfile << endl << "Hijacking Level1 : " << l1.size() << endl;
//    for(string str : l1)
//        //cout << act -> getName() << " , ";
//        outfile << str << endl;
//    //cout << endl << "Hijacking Level2 : " << endl;
//    outfile << endl << "Hijacking Level2 : " << l2.size() << endl;
//    for(string str : l2)
//        //cout << act -> getName() << " , ";
//        outfile << str << endl;
//    //cout << endl;
//    outfile << endl << endl;
//}
//
//
//
//int max(int a, int b)
//{
//    return a > b ? a : b;
//}
//

//
//
//

