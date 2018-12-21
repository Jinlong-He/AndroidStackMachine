;//
//  Activity.cpp
//  ASM
//
//  Created by 何锦龙 on 2018/8/20.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include "Activity.hpp"
using namespace atm;

Activity::~Activity() {
}

void Activity::addAction(Action* action) {
    actions.insert(action);
    addDegree();
}

void Activity::mkExitAndEntranceMap(const Acts& visited, PortMap& exitMap, PortMap& entranceMap, Activity* realAct, Acts& tasks, Act2ActsMap& taskMap, Activity* mainAct, Aft& ignoreAft, VirtualActionsMap& virtualActionsMap, Act2ActionsMap& visitedActions, Act2ActionsMap& availableActions) {
    Actions workActions(actions.begin(), actions.end());
    workActions.insert(virtualActionsMap[realAct][this].begin(), virtualActionsMap[realAct][this].end());
    for (Action* action : workActions) {
        Activity* newAct = action -> getTargetAct();
        Aft newAft = newAct -> getAft();
        Aft aft = realAct -> getAft();
        if (!action -> isSwitchingTaskAction(aft)) {
            Acts newVisited;
            newVisited.insert(visited.begin(), visited.end());
            availableActions[realAct].insert(action);
            if (newVisited.insert(newAct).second) { 
                newAct -> mkExitAndEntranceMap(newVisited, exitMap, entranceMap, realAct, tasks, taskMap, mainAct, ignoreAft, virtualActionsMap, visitedActions, availableActions);
            }
        } else {
            if(newAft == ignoreAft) continue;
            tasks.insert(newAct);
            for (Activity* targetRealAct : tasks) {
                if (targetRealAct -> getAft() == newAft) {
                    taskMap[realAct].insert(targetRealAct);
                    //if (f && newAct == targetRealAct && newAct != mainAct) {
                    //    if (!action -> hasCTPFlag()) continue;
                    //}
                    exitMap[realAct].insert(new Port(this, targetRealAct, Flags(), action -> isFinishStart()));
                    entranceMap[targetRealAct].insert(new Port(newAct, realAct, action -> getFlags(), 0));
                }
            }
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

bool Activity::addAvailablePos(Activity* act, ID pos, ID& maxLength, const Acts& visited, ContentMap& contentMap) {
    maxLength = maxLength >= pos ? maxLength : pos; 
    bool f = true;
    vector<Acts>& vec = contentMap[act][this];
    for (Acts acts : vec) {
        if (isEqual(acts, visited)) f = false;
    }
    if (f) vec.push_back(visited);
    return (availablePos[act].insert(pos).second | f);
}

void Activity::addAvailablePos(Activity* realAct, ID pos) {
    availablePos[realAct].insert(pos);
}


bool Action::isSwitchingTaskAction(Aft aft) 
{
    if (getTargetAct() -> getAft() == aft) return false;
    if (getTargetAct() -> getLmd() == lmd_stk) return true;
    for (FLAG flag : getFlags())
        if (flag == NTK) return true;
    return false;
}

bool Action::isNormalAction(Aft aft, Activity* act) {
    if (isSwitchingTaskAction(aft)) return false;
    if (hasCTKFlag()) return false;
    if (hasCTPFlag()) return false;
    if (isSTPAction(act)) return false;
    return true;
}

bool Action::isSTPAction(Activity* act) {
    if (act == targetAct) {
        if (targetAct -> getLmd() == lmd_stp) return true;
        for (FLAG flag : getFlags())
            if (flag == STP) return true;
    }
    return false;
}

bool Action::hasRTFFlag() 
{
    for (FLAG flag : getFlags())
        if (flag == RTF) return !hasNTKFlag() && !hasCTPFlag();
    return false;
}

bool Action::hasCTPFlag() 
{
    for (FLAG flag : getFlags())
        if (flag == CTP) return !hasCTKFlag();
    return false;
}

bool Action::hasNTKFlag() 
{
    if (targetAct -> getLmd() == lmd_stk) return true;
    for (FLAG flag : getFlags())
        if (flag == NTK) return true;
    return false;
}

bool Action::hasCTKFlag() 
{
    for (FLAG flag : getFlags())
        if (flag == CTK) return hasNTKFlag();
    return false;
}





