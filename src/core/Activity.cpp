;//
//  Activity.cpp
//  ASM
//
//  Created by 何锦龙 on 2018/8/20.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include "Activity.hpp"



Activity::~Activity() {
    for (Acton* action : actions)
        delete action;
}

Activity::Activity(const string& name, ID pos, Lmd lmd, Aft aft) {
    id = pos;
    inDegree = 0;
    activityName = name;
    lunchMode = lmd;
    affinity = aft;
}

void Activity::addActions(Acton* action) {
    actions.push_back(action);
    addDegree();
}

void Activity::mkExitAndEntranceMap(Acts& acts, Aft2ActsMap& exitMap, Aft2ActionsMap& entranceMap, Aft aft, Aft targetAft) {
    for (Acton* action : actions) {
        Activity* newAct = action -> getActivity();
        Aft newAft = newAct -> getAft();
        if (!action -> isSwitchingTaskAction(aft)) {
            Acts visited;
            visited.insert(acts.begin(), acts.end());
            if (visited.insert(newAct).second) { 
                newAct -> mkExitAndEntranceMap(visited, exitMap, entranceMap, aft, targetAft);
            }
        } else {
            if(newAft == targetAft) continue;
            exitMap[aft].insert(this);
            entranceMap[newAft].insert(action);
        }
    }
}

bool isEqual(const Acts& acts1, const Acts& acts2) {
    if (acts1.size() != acts2.size()) return false;
    for (Activity* act : acts1) {
        if (acts2.count(act) == 0) return false;
    }
    return true;
}

bool Activity::addAvailablePos(Aft aft, ID pos, ID& maxLength, Acts& visited, ContentMap& contentMap) {
    maxLength = maxLength >= pos ? maxLength : pos; 

    bool f = true;
    vector<Acts>& vec = contentMap[this][aft];
    for (Acts acts : vec) {
        if (isEqual(acts, visited)) f = false;
    }
    if (f) vec.push_back(visited);
    return (availablePos[aft].insert(pos).second | f);
}


bool Action::isSwitchingTaskAction(Aft aft) 
{
    if (getActivity() -> getAft() == aft) return false;
    if (getActivity() -> getLmd() == lmd_stk) return true;
    for (FLAG flag : getFlags())
        if (flag == NTK) return true;
    return false;
}



