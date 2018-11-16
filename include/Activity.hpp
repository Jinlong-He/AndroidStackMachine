//
//  Activity.hpp
//  ASM
//
//  Created by 何锦龙 on 2018/8/20.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef Activity_hpp
#define Activity_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <math.h>

using namespace std;
namespace asm {
    class Activity;
    class Action;
    enum Lmd {lmd_std, lmd_stk, lmd_stp, lmd_sit};
    enum FLAG {CTK, CTP, NTK, RTF, STD, MTK, TOH, FIN};
    typedef string Aft;
    typedef size_t ID;
    typedef vector<FLAG> Flags;
    typedef unordered_set<ID> IDs;
    typedef unordered_set<Aft> Afts;
    typedef unordered_set<Activity*> Acts;
    typedef unordered_set<Action*> Actions;

    typedef unordered_map<Aft, IDs> AvailablePos;
    typedef unordered_map<Aft, Acts> Aft2ActsMap;
    typedef unordered_map<Aft, vector<Acts> > Aft2ActsVecMap;
    typedef unordered_map<Aft, Actions> Aft2ActionsMap;
    typedef unordered_map<Activity*, Actions> Act2ActionsMap;
    typedef unordered_map<Aft, Act2ActionsMap> VirtualActionsMap;
    typedef unordered_map<Activity*, Aft2ActsVecMap> ContentMap;


    /// \breif Activity in the Android system.
    class Activity
    {
    private:
        ID id;                      ///< the uniqe identity of each Activity.
        ID degree;                  ///< the sum of indegree and outdegree for an Activity in the ATG.
        Aft affinity;               ///< the afinity attribute for an Activity.
        Lmd lunchMode;              ///< the lunch mode for an Activity.
        string activityName;        ///< the name for an Activity.
        Actions actions;            ///< the actions in the ATG for an Activity.
        AvailablePos availablePos;  ///< the available positions in a task for an Activity.
    public:
        
        Activity() {}
        ~Activity();
        Activity(const string& name, ID i, Lmd lmd = lmd_std, Aft aft = "");
        
        ID getID() {return id;}
        ID getDegree() {return degree;}
        Aft getAft() {return affinity;}
        Lmd getLmd() {return lunchMode;}
        string getName() {return activityName;}
        Actions& getActions() {return actions;}
        AvailablePos& getAvailablePos() {return availablePos;}

        void addDegree() {degree++;}
        void addActions(Action* action);
        bool addAvailablePos(Aft aft, ID pos, ID& maxLength, Acts& visited);
        void mkExitAndEntranceMap(Acts& acts, Aft2ActsMap& exitMap, Aft2ActionsMap& entranceMap, Aft aft, Aft targetAft);
        
        void outputAvailablePos()
        {
            output();
            for(auto it = availablePos.begin(); it != availablePos.end(); it++)
            {
                cout << it -> first << " : ";
                for(ID id : it -> second)
                    cout<< id <<" " ;
                cout << endl;
            }
            cout << endl;
        }

        void output()
        {
            cout<<activityName<<" "<<lunchMode<<" "<<affinity<<endl;
        }

    };

    class Action
    {
    private:
        Activity* activity; ///< Activity in an action.
        Flags flags;        ///< the falgs set in an acton.
        ID id;              ///< the uniqe identity for an action
    public:
        Action() {activity = nullptr;};
        Action(Activity* act, Flags& fs, ID i)
        {
            activity = act;
            flags.assign(fs.begin(), fs.end());
            id = i;
        }
        Activity* getActivity() {return activity;}
        Flags& getFlags() {return flags;}
        ID getID() {return id;}

        bool isSwitchingTaskAction(Aft aft);
    };

    class Task
    {
    private:
        Aft aft;            ///<the affinity for this task which is same with realAct's affinity.
        Activity* realActivity;  ///<the Activity which create this task.   
    public:
        Task() {realActivity = nullptr;}
        Task(Aft a, Activity* realAct) {aft = a, realActivity = realAct;}

        getAft() {return aft;}
        getRealActivity() {return realActivity;}
    };

}

#endif /* Activity_hpp */
