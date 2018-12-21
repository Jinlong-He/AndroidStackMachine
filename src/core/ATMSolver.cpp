//
//  ATMSolver.cpp
//  ATM
//
//  Created by 何锦龙 on 2018/8/23.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include "ATMSolver.hpp"
#include "utility/Utility.hpp"
using namespace atm;

void ATMSolver::initActVars() {
    ID id = 1;
    Values stateValues;
    stateValues.push_back(nullValue);
    for (Activity* act : getActivities()) {
        Value* value = mkValue(id);
        id2ActMap[id++] = act;
        act2ValueMap[act] = value;
        stateValues.push_back(value);
    }
    for (ID i = 0; i < getTaskNum(); i++) {
        for (ID j = 1; j <= getTaskLength(); j++) {
            ID sId = i * (getTaskLength() + 1) + j;
            stateVars[sId] = mkStateVar(sId, stateValues, nullValue);
        }
    }
}

void ATMSolver::initTaskVars() {
    Values taskValues;
    ID id = 0;
    for (auto& mapPair : getRealActMap()) {
        taskValues.clear();
        taskValues.push_back(nullValue);
        for (Activity* realAct : mapPair.second) {
            taskValues.push_back(act2ValueMap[realAct]);
        }
        ID sId = id * (getTaskLength() + 1);
        aft2IDMap[mapPair.first] = id++;
        stateVars[sId] = mkStateVar(sId, taskValues, nullValue);
    }
}

void ATMSolver::initCharVars() {
    Values charValues;
    charValues.push_back(nullValue);
    ID id = 2; 
    for (Action* action : getActions()) {
        Value* value = mkValue(id);
        id2ActionMap[id++] = action;
        action2ValueMap[action] = value;
        charValues.push_back(value);
    }
    popValue = mkValue(1);
    charValues.push_back(popValue);
    charVars.push_back(mkCharVar(0, charValues));
    charVars.resize(1);
}

void ATMSolver::initOrderVars() {
    Values orderValues;
    orderValues.push_back(nullValue);
    Order datas;
    for (ID i = 1; i <= getTaskNum(); i++) {
        datas.push_back(i);
    }
    Order nullOrder;
    for (ID i = 0; i < getTaskNum(); i++)
        nullOrder.push_back(0);
    order2ValueMap[nullOrder] = nullValue;
    Utility::Permutation_Order_Null(datas, (ID)0, orders);
    ID id = 1;
    for (Order& order : orders) {
        //for (ID i = 0; i < order.size(); i++) {
        //    if (order[i] == 100) order[i] = 0;
        //}
        Value* value = mkValue(id);
        id2OrderMap[id++] = order;
        order2ValueMap[order] = value;
        orderValues.push_back(value);
    }
    ID sId = stateVarNum - 1;
    stateVars[sId] = mkStateVar(sId, orderValues, nullValue);
    stateVars.resize(stateVarNum);
}
void ATMSolver::init() {
    ID n = getTaskNum();
    ID l = getTaskLength();
    stateVarNum = (l + 1) * n + 1;
    StateVar* tempStateVar = nullptr;
    for (ID i = 0; i < stateVarNum; i++)
        this -> stateVars.push_back(tempStateVar);
    nullValue = mkValue(0);
    initActVars();
    initTaskVars();
    initOrderVars();
    initCharVars();
    startMain();
    startActions();
    mkOrderTrans();
    pop();
    //for (auto& mapPair : atm -> getLoopMap()) {
    //    mkLoopConfiguration(mapPair.first, mapPair.second);
    //    break;
    //}
}

Condition* ATMSolver::mkCondition(Condition* condition) {
    Condition* cpCondition = new Condition(*condition);
    conditions.push_back(cpCondition);
    return cpCondition;
}
Condition* ATMSolver::mkCondition(Activity* realAct, Action* action, ID i, ID j, bool order) {
    Activity* act = action -> getSourceAct();
    Condition* condition = new Condition();
    conditions.push_back(condition);
    condition -> mkCharAtomic(charVars[0], action2ValueMap[action]);
    condition -> mkStateAtomic(getStateVar(i, j), act2ValueMap[act]);
    condition -> mkStateAtomic(getStateVar(i), act2ValueMap[realAct]);
    if (order)
        mkOrderAtomic(condition, i);
    if (j < getTaskLength())
        condition -> mkStateAtomic(getStateVar(i, j + 1), nullValue);
    return condition;
}

void ATMSolver::Push(Condition* condition, Action* action, ID i, ID j, bool fin) {
    Activity* targetAct = action -> getTargetAct(); 
    if (fin) {
        getStateVar(i, j - 1) -> mkTransition(condition, act2ValueMap[targetAct]);
    } else {
        if (j > getTaskLength())
            return;
        getStateVar(i, j) -> mkTransition(condition, act2ValueMap[targetAct]);
    }
}

void ATMSolver::ClrTop(Condition* condition, Activity* realAct, Action* action, ID i, ID j, ID m) {
    Activity* targetAct = action -> getTargetAct(); 
    for (ID n : targetAct -> getAvailablePos()[realAct]) {
        if (n <= m || n >= j) continue;
        condition -> mkStateAtomic(getStateVar(i, n), act2ValueMap[targetAct], 0);
    }
    for (ID n = m + 1; n <= j; n++) {
        getStateVar(i, n) -> mkTransition(condition, nullValue);
    }
}

void ATMSolver::ClrTop(Activity* realAct, Action* action, ID i, ID j) {
    Condition* condition = mkCondition(realAct, action, i, j);
    ClrTop(condition, realAct, action, i, j);
}

void ATMSolver::ClrTop(Condition* condition, Activity* realAct, Action* action, ID i, ID j) {
    Activity* targetAct = action -> getTargetAct();
    Condition* stdCondition = mkCondition(condition);
    for (ID m : targetAct -> getAvailablePos()[realAct]) {
        if (m >= j) continue;
        stdCondition -> mkStateAtomic(getStateVar(i, m), act2ValueMap[targetAct], 0);
        Condition* ctpCondition = mkCondition(condition);
        ctpCondition -> mkStateAtomic(getStateVar(i, m), act2ValueMap[targetAct]);
        ClrTop(ctpCondition, realAct, action, i, j, m);
    }
    Push(stdCondition, action, i, j + 1, action -> isFinishStart());
}

void ATMSolver::Finish(Condition* condition, Action* action, ID i, ID j) {
    getStateVar(i, j) -> mkTransition(condition, nullValue);
}

void ATMSolver::getNewOrder(Order& order, Order& newOrder, ID id) {
    if (order[id] == 0) {
        for (ID i = 0; i < order.size(); i++) {
            if (id != i && order[i] == 0) {
                newOrder.push_back(0);
            } else {
                newOrder.push_back(order[i] + 1);
            }
        }
    } else {
        for (ID i = 0; i < order.size(); i++) {
            if (order[i] < order[id] && order[i] != 0) {
                newOrder.push_back(order[i] + 1);
            } else if (i == id){
                newOrder.push_back(1);
            } else {
                newOrder.push_back(order[i]);
            }
        }
    }
}

void ATMSolver::getNewPopOrder(Order& order, Order& newOrder, ID id) {
    for (ID i = 0; i < order.size(); i++) {
        if (order[i] > order[id]) {
            newOrder.push_back(order[i] - 1);
        } else if (order[i] == order[id]){
            newOrder.push_back(0);
        } else {
            newOrder.push_back(order[i]);
        }
    }
}

void ATMSolver::SwitchTask(Condition* condition, Action* action, Order& order) {
    Aft aft = action -> getTargetAct() -> getAft();
    ID id = aft2IDMap[aft];
    Order newOrder;
    getNewOrder(order, newOrder, id);
    getStateVar() -> mkTransition(condition, order2ValueMap[newOrder]);
}

void ATMSolver::New(Condition* condition, Action* action, ID i, ID j, bool push) {
    Aft targetAft = action -> getTargetAct() -> getAft();
    ID m = aft2IDMap[targetAft];
    for (Order& order : orders) {
        if (order[i] != 1) continue;
        Condition* orderCondition = mkCondition(condition);
        orderCondition -> mkStateAtomic(getStateVar(), order2ValueMap[order]);
        SwitchTask(orderCondition, action, order);
        if (action -> isFinishStart()) {
            Finish(orderCondition, action, i, j);
        }
        if (order[m] == 0) {
            Push(orderCondition, action, m, 1);
        } else {
            for (ID n = 1; n < getTaskLength(); n++) {
                Condition* cpCondition = mkCondition(orderCondition);
                cpCondition -> mkStateAtomic(getStateVar(m, n), nullValue, 0);
                cpCondition -> mkStateAtomic(getStateVar(m, n + 1), nullValue);
                if (push) {
                    Push(cpCondition, action, m, n + 1);
                } else {
                    for (Activity* realAct : getRealActMap()[targetAft]) {
                        ClrTop(cpCondition, realAct, action, m, n);
                    }
                }
            }
        }
    }
}

void ATMSolver::mkOrderTrans() {
    for (Order& order : orders) {
        for (ID id = 0; id < order.size(); id++) {
            if (order[id] > 0) {
                Order newOrder;
                getNewPopOrder(order, newOrder, id);
                Condition* condition = mkCondition();
                condition -> mkCharAtomic(charVars[0], nullValue);
                condition -> mkStateAtomic(getStateVar(), order2ValueMap[order]);
                condition -> mkStateAtomic(getStateVar(id, 1), nullValue);
                getStateVar() -> mkTransition(condition, order2ValueMap[newOrder]);
            }
        }
    }
}

//void ATMSolver::ClrTsk(Condition* condition, Action* action, ID i, ID j) {
//    Activity* targetAct = action -> getActivity(); 
//    for (ID m = 1; m <= j; j++) {
//        getStateVar(i, m) -> addTransition(new Transition(condition, nullValue));
//    }
//    getStateVar(i, 0) -> addTransition(new Transition(condition, act2ValueMap[targetAct]));
//}

void ATMSolver::startMain() {
    Condition* condition = new Condition();
    Aft mainAft = getMainActivity() -> getAft();
    ID mainID = (aft2IDMap[mainAft]);
    condition -> mkStateAtomic(getStateVar(mainID), nullValue); 
    condition -> mkStateAtomic(getStateVar(mainID, 1), nullValue); 
    condition -> mkStateAtomic(getStateVar(), nullValue);
    condition -> mkCharAtomic(charVars[0], action2ValueMap[getMainAction()]);
    getStateVar(mainID) -> mkTransition(condition, act2ValueMap[getMainActivity()]);
    getStateVar(mainID, 1) -> mkTransition(condition, act2ValueMap[getMainActivity()]);
    ID init[getAfts().size()];
    for (Aft aft : getAfts()) {
        if (aft == mainAft) {
            init[aft2IDMap[aft]] = 1;
        } else {
            init[aft2IDMap[aft]] = 0;
        }
    }
    Order order(init, init + getAfts().size());
    getStateVar() -> mkTransition(condition, order2ValueMap[order]);
}

void ATMSolver::pop() {
    for (Order& order : orders) {
        for (ID i = 0; i < order.size(); i++) {
            if (order[i] == 1) {
                for (ID j = 1; j <= getTaskLength(); j++) {
                    Condition* condition = mkCondition();
                    condition -> mkStateAtomic(getStateVar(), order2ValueMap[order]);
                    condition -> mkStateAtomic(getStateVar(i, j), nullValue, 0);
                    condition -> mkCharAtomic(charVars[0], popValue);
                    if (j < getTaskLength())
                        condition -> mkStateAtomic(getStateVar(i, j + 1), nullValue);
                    getStateVar(i, j) -> mkTransition(condition, nullValue);
                }
            }
        }
    }
}

void ATMSolver::startActions() {
    for (Action* action : getActions()) {
        if (action == atm -> getMainAction()) continue;
        if (action -> hasNTKFlag()) {
            if (action -> hasCTPFlag()) {
                startNewCTP(action);
            } else {
                startNewSTD(action);
            }
        } else {
            if (action -> hasCTPFlag()) {
                startCTP(action);
            } else {
                startSTD(action);
            }
        }
    }
}


void ATMSolver::startSTD(Action* action) {
    Activity* targetAct = action -> getTargetAct(); 
    Activity* act = action -> getSourceAct();
    Aft targetAft = targetAct -> getAft();
    for (Aft aft : getAfts()) {
        ID i = aft2IDMap[aft];
        for (Activity* realAct : getRealActMap()[aft]) {
            for (ID j : act -> getAvailablePos()[realAct]) {
                Condition* condition = mkCondition(realAct, action, i , j);
                Push(condition, action, i, j + 1, action -> isFinishStart());
            }
        }
    }
}

void ATMSolver::startCTP(Action* action) {
    Activity* targetAct = action -> getTargetAct(); 
    Activity* act = action -> getSourceAct();
    if (act == targetAct) return;
    Aft targetAft = targetAct -> getAft();
    for (Aft aft : getAfts()) {
        ID i = aft2IDMap[aft];
        for (Activity* realAct : getRealActMap()[aft]) {
            for (ID j : act -> getAvailablePos()[realAct]) {
                ClrTop(realAct, action, i, j);
            }
        }
    } 
}

//void ATMSolver::startCTK(Activity* act, Action* action) {
//    Activity* targetAct = action -> getActivity(); 
//    Aft targetAft = targetAct -> getAft();
//    for (ID i = 0; i < getTaskNum(); i++) {
//        for (ID j : act -> getAvailablePos()[targetAft]) {
//            Condition* condition = mkCondition(act, targetAft, action, i , j);
//            ClrTsk(stdCondition, action, i, j);
//        }
//    }
//}

void ATMSolver::startNewSTD(Action* action) {
    Activity* act = action -> getSourceAct();
    Activity* targetAct = action -> getTargetAct(); 
    //cout << targetAct -> getName() << endl;
    Aft targetAft = targetAct -> getAft();
    //cout << targetAft << endl;
    for (Aft aft : getAfts()) {
        ID i = aft2IDMap[aft];
        for (Activity* realAct : getRealActMap()[aft]) {
            for (ID j : act -> getAvailablePos()[realAct]) {
                //cout << targetAct -> getName() << endl;
                if (aft == targetAft) {
                    if (realAct == targetAct && targetAct != getMainActivity())
                        continue;
                    Condition* condition = mkCondition(realAct, action, i , j);
                    Push(condition, action, i, j + 1, action -> isFinishStart());
                } else {
                    Condition* condition = mkCondition(realAct, action, i , j, 0);
                    New(condition, action, i, j, 1);
                }
            }
        }
    }
}

void ATMSolver::startNewCTP(Action* action) {
    Activity* act = action -> getSourceAct();
    Activity* targetAct = action -> getTargetAct(); 
    Aft targetAft = targetAct -> getAft();
    for (Aft aft : getAfts()) {
        ID i = aft2IDMap[aft];
        for (Activity* realAct : getRealActMap()[aft]) {
            for (ID j : act -> getAvailablePos()[realAct]) {
                if (aft == targetAft) {
                    ClrTop(realAct, action, i, j);
                } else {
                    Condition* condition = mkCondition(realAct, action, i , j, 0);
                    New(condition, action, i, j, 0);
                }
            }
        }
    }
}

void ATMSolver::mkLoopConfiguration(Activity* realAct, Acts& loopActs) {
    configuration.clear();
    Aft aft = realAct -> getAft();
    vector<vector<Activity*> > permutation;
    Utility::Permutation(loopActs, permutation);
    ID i = aft2IDMap[aft];
    for (auto& loop : permutation) {
        for (ID j : loop[0] -> getAvailablePos()[realAct]) {
            if (j > getTaskLength() - loop.size() + 1) continue;
            StateAtomics stateAtomics;
            for (ID m = 0; m < loop.size(); m++) {
            stateAtomics.push_back(new StateAtomic(getStateVar(i, j + m), act2ValueMap[loop[m]]));
            }
            this -> configuration.push_back(stateAtomics);
        }
    }
}

void ATMSolver::mkBackPattenConfiguration(Activity* act, Activity* bAct) {
    configuration.clear();
    Value* actValue = act2ValueMap[act];
    Value* bActValue = act2ValueMap[bAct];
    for (Aft aft : getAfts()) {
        ID i = aft2IDMap[aft];
        for (Activity* realAct : getRealActMap()[aft]) {
            for (ID j : act -> getAvailablePos()[realAct]) {
                if (j > 1) {
                    StateAtomics stateAtomics;
                    stateAtomics.push_back(new StateAtomic(getStateVar(i, j), actValue));
                    stateAtomics.push_back(new StateAtomic(getStateVar(i, j - 1), bActValue));
                    configuration.push_back(stateAtomics);
                } else {
                    for (Order& order : orders) {
                        if (order[i] > 0) {
                            for (ID m = 0; m < order.size(); m++) {
                                if (order[m] == order[i] + 1) {
                                    for (ID n = 1; n <= getTaskLength(); n++) {
                                        StateAtomics stateAtomics;
                                        stateAtomics.push_back(new StateAtomic(getStateVar(i, 1), actValue));
                                        stateAtomics.push_back(new StateAtomic(getStateVar(), order2ValueMap[order]));

                                        stateAtomics.push_back(new StateAtomic(getStateVar(m, n), bActValue));
                                        if (n < getTaskLength()) {
                                            stateAtomics.push_back(new StateAtomic(getStateVar(m, n + 1), nullValue));
                                        }
                                        configuration.push_back(stateAtomics);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void ATMSolver::pre4BackPatten(VerificationDatas& datas, Activity* act) {
    for (Activity* bAct : getActivities()) {
       mkBackPattenConfiguration(act, bAct);
       string smv = getPreSMV() + getINVARSPEC();
       int* value = nullptr;
       string* path = nullptr;
       VerificationData data = {bAct, "", smv, value, path, this};
       datas.push_back(data);
    }
}






//string replaceAll(std::string str, const std::string& from, const std::string& to) {
//    size_t start_pos = 0;
//    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
//        str.replace(start_pos, from.length(), to);
//        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
//    }
//    return str;
//}
//
//bool ASMSolver::isSTD(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == NTK) return false;
//            else if(flag == CTK) return false;
//            else if(flag == CTP) return false;
//            else if(flag == RTF) return false;
//        }
//        return true;
//    }
//}
//
//bool ASMSolver::isCTP(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == NTK) return false;
//        }
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == CTP) return true;
//        }
//        return false;
//    }
//}
//
//bool ASMSolver::isRTF(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == CTP || flag == NTK) return false;
//        }
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == RTF) return true;
//        }
//        return false;
//    }
//}
//
//bool ASMSolver::isNTK(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getAft() == targetAft) return false;
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == CTK) return false;
//            else if(flag == CTP) return false;
//            else if(flag == RTF) return false;
//        }
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == NTK) return true;
//        }
//        return false;
//    }
//}
//
//bool ASMSolver::isSTK(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getAft() == targetAft) return false;
//    if(act -> getLmd() == lmd_stk)
//    {
//        for(FLAG flag : transition -> getFlags())
//            if(flag == CTK) return false;
//        return true;
//    }
//    else return false;
//}
//
//bool ASMSolver::isRTF_NTK(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getAft() == targetAft) return false;
//    int count = 0;
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == RTF) count++;
//            else if(flag == NTK) count++;
//            else if(flag == CTK || flag == CTP) count--;
//        }
//        if(count == 2)
//            return true;
//    }
//    return false;
//}
//
//bool ASMSolver::isCTP_NTK(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getAft() == targetAft) return false;
//    int count = 0;
//    if(act -> getLmd() == lmd_stk) return false;
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == CTP) count++; 
//            else if(flag == NTK) count++;
//            else if(flag == CTK) count--;
//        }
//        if(count == 2)
//            return true;
//    }
//    return false;
//}
//
//bool ASMSolver::isCTK_NTK(Transition* transition)
//{
//    Activity* act = transition -> getActivity();
//    if(act -> getAft() == targetAft) return false;
//    int count = 0;
//    if(act -> getLmd() == lmd_stk)
//    {
//        for(FLAG flag : transition -> getFlags())
//            if(flag == CTK) return true;
//        return false;
//    }
//    else
//    {
//        for(FLAG flag : transition -> getFlags())
//        {
//            if(flag == CTK) count++;
//            else if(flag == NTK) count++;
//        }
//        if(count == 2)
//            return true;
//    }
//    return false;
//}
//
//Condition::Condition(const Condition& condition)
//{
//    trueStateConditions.assign(condition.trueStateConditions.begin(), condition.trueStateConditions.end());
//    falseStateConditions.assign(condition.falseStateConditions.begin(), condition.falseStateConditions.end());
//    charCondition.first = condition.charCondition.first;
//    charCondition.second = condition.charCondition.second;
//    trueAftConditions.assign(condition.trueAftConditions.begin(), condition.trueAftConditions.end());
//    falseAftConditions.assign(condition.falseAftConditions.begin(), condition.falseAftConditions.end());
//    targetAct = condition.targetAct;
//    targetState = condition.targetState;
//    targetAftVar = condition.targetAftVar;
//    valid = condition.valid;
//    length = condition.length;
//}
//
//
//ASMSolver::ASMSolver()
//{
//    charVar = nullptr;
//    nullAct = nullptr;
//    mainActivity = nullptr;
//    nullTrans = nullptr;
//    nullAft = "";
//}
//
//ASMSolver::~ASMSolver()
//{
//    for(AftVar* aftVar : aftVars)
//        delete aftVar;
//    for(ID i = 0; i < stateVarsVec.size(); i++)
//        for(ID j = 0; j < stateVarsVec[i].size(); j++)
//            delete stateVarsVec[i][j];
//    delete charVar;
//}
//
//void ASMSolver::setMaxLength(ID length)
//{
//    stackLength = length;
//}
//
//void ASMSolver::loadVars(ASM* a)
//{
//    mainActivity = a -> getMainActivity();
//    mainAft = a -> getMainAft();
//    targetAft = a -> getTargetAft();
//    activities.assign(a -> getActivities().begin(), a -> getActivities().end());
//    transitions.assign(a -> getTransitions().begin(), a -> getTransitions().end());
//    transitions.insert(transitions.end(), a -> getMainTransitions().begin(), a -> getMainTransitions().end());
//    mainTransitions.assign(a -> getMainTransitions().begin(), a -> getMainTransitions().end());
//    for (Activity* act : activities) {
//        id2ActMap[act -> getID()] = act;
//    }
//    for (Transition* trans : transitions) {
//        id2TransMap[trans -> getID()] = trans;
//    }
//    afts.assign(a -> getAfts().begin(), a -> getAfts().end());
//    mainActivities.assign(a -> getMainActivities().begin(), a -> getMainActivities().end());
//    init(a -> getActNum(), a -> getStackNum(), a -> getStackLength() + 1);
//    cout << "stack number is: " << stackNum <<"; stack max length is: " << stackLength <<endl;
//    startMain(a -> getFlag());
//    for (Activity* activity : activities) {
//        Transitions& transitions = activity -> getTransitions();
//        if (transitions.size() > 0) {
//            for (Transition* transition : transitions) {
//                if (isSTD(transition)) startStd(activity, transition, FLAG(-1));
//                else if (isRTF(transition)) startStdWithRTF(activity, transition);
//                else if (isCTP(transition)) startStdWithCTP(activity, transition);
//                else if (isNTK(transition)) startStdWithNTK(activity, transition);
//                else if (isSTK(transition)) startStk(activity, transition);
//                else if (isCTK_NTK(transition)) startActWithCTK_NTK(activity, transition);
//                else if (isCTP_NTK(transition)) startStdWithCTP_NTK(activity, transition);
//                else if (isRTF_NTK(transition)) startStdWithRTF_NTK(activity, transition);
//                else {}
//            }
//        }
//    }
//    smv = getVarStrSimple() + getInitStrSimple() + getCodeStrSimple(); 
//}
//
//void ASMSolver::init(ID actNum, ID aftNum, ID length)
//{
//    int count = 0;
//    stackLength = length;
//    stackNum = aftNum;
//    stateVarsVec.resize(aftNum);
//    for(ID i = 0; i < aftNum; i++)
//    {
//        stateVarsVec[i].resize(stackLength);
//        for(ID j = 0; j < stackLength; j++)
//            stateVarsVec[i][j] = new StateVar(count++);
//    }
//    aftVars.resize(stackNum);
//    count = 0;
//    for(ID i = 0; i < aftNum; i++)
//        aftVars[i] = new AftVar(count++);
//    count = 0;
//    charVar = new CharVar(0);
//    nullAct = new Activity("null", 0);
//    Flags flags;
//    nullTrans = new Transition(nullAct, flags, 0);
//    nullAft = "e";
//    afts.push_back(nullAft);
//    activities.push_back(nullAct);
//}
//
//void ASMSolver::startMain(bool flag)
//{
//    for (Transition* transition : mainTransitions) {
//        Condition* condition = new Condition(stackLength);
//        condition -> setCharCondition(charVar, transition);
//        condition -> addTrueCondition(stateVarsVec[0][0], nullAct);
//        Condition* copyCondition = new Condition(*condition);
//        copyCondition -> setTargetAft(mainAft);
//        aftVars[0] -> addCondition(copyCondition);
//        condition -> setTargetAct(transition -> getActivity());
//        stateVarsVec[0][0] -> addCondition(condition);
//    }
//} 
//
//void ASMSolver::startStd(Activity* act, Transition* transition, FLAG flag)
//{
//    Activity* targetAct = transition -> getActivity(); 
//    Aft targetAft = targetAct -> getAft();
//    for (ID i = 0; i < stackNum; i++) {
//        for (Aft aft : afts) {
//            for (ID j : act -> getAvailablePos()[aft]) {
//                if (j == stackLength - 1) continue;
//                Condition* condition = new Condition(stackLength);
//                condition -> setCharCondition(charVar, transition);
//                condition -> addTrueCondition(stateVarsVec[i][j], act);
//                condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                if (i < stackNum - 1)
//                condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                //for(ID k : targetAct -> getAvailablePos()[targetAft])//there is no instance of target.
//                //{
//                //    if(k >= j) continue;
//                //    condition -> addFalseCondition(stateVarsVec[i][k], targetAct);
//                //}
//                condition -> setTargetAct(targetAct);
//                if (Activity::isFIN(transition)) {
//                    stateVarsVec[i][j] -> addCondition(condition);
//                } else {
//                    stateVarsVec[i][j + 1] ->addCondition(condition);
//                }
//            }
//        }
//    }
//}
//
//
//void ASMSolver::startStdWithCTP(Activity* act, Transition* transition)
//{
//    //startStd(act, transition, CTP);
//    Activity* targetAct = transition -> getActivity();
//    for (ID i = 0; i < stackNum; i++) {
//        for (Aft aft : afts) {
//            for (ID j : act -> getAvailablePos()[aft]) {
//                for (ID k : targetAct -> getAvailablePos()[aft]) {
//                    if (k >= j) continue; 
//                    for (ID l = j; l > k; l--) {
//                        Condition* condition = new Condition(stackLength);
//                        condition -> setCharCondition(charVar, transition);
//                        condition -> addTrueCondition(stateVarsVec[i][j], act);
//                        condition -> addTrueCondition(stateVarsVec[i][k], targetAct);
//                        if (j < stackLength - 1)
//                            condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                        if (i < stackNum - 1)
//                            condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                        condition -> setTargetAct(nullAct);
//                        stateVarsVec[i][l] -> addCondition(condition);
//                    }
//                }
//                if (j == stackLength - 1) continue;
//                Condition* condition = new Condition(stackLength);
//                condition -> setCharCondition(charVar, transition);
//                condition -> addTrueCondition(stateVarsVec[i][j], act);
//                if (j < stackLength - 1)
//                    condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                if (i < stackNum - 1)
//                    condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                for (ID l : targetAct -> getAvailablePos()[aft]) {
//                    if (l >= j) continue;
//                    condition -> addFalseCondition(stateVarsVec[i][l], targetAct);
//                }
//                condition -> setTargetAct(targetAct);
//                if (Activity::isFIN(transition)) {
//                    stateVarsVec[i][j] -> addCondition(condition);
//                } else {
//                    stateVarsVec[i][j + 1] ->addCondition(condition);
//                }
//
//            }
//        }
//    }
//}
//
//void ASMSolver::startStdWithRTF(Activity* act, Transition* transition)
//{
//    //startStd(act, transition, RTF);
//    Activity* targetAct = transition -> getActivity();
//    for(ID i = 0; i < stackNum; i++)
//    {
//        for(Aft aft : afts) 
//        for(ID j : act -> getAvailablePos()[aft])
//        {
//            for(ID k : targetAct -> getAvailablePos()[aft])
//            {
//                if(k >= j) continue;
//                Condition* condition = new Condition(stackLength);
//                condition -> setCharCondition(charVar, transition);
//                condition -> addTrueCondition(stateVarsVec[i][j], act);
//                condition -> addTrueCondition(stateVarsVec[i][k],targetAct);
//                if(j < stackLength - 1)
//                condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                if(i < stackNum - 1)
//                condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                if (Activity::isFIN(transition)) {
//                    for(ID l = k; l < j - 1; l++)
//                    {
//                        Condition* copyCondition = new Condition(*condition);
//                        copyCondition -> setTargetState(stateVarsVec[i][l + 1]);
//                        stateVarsVec[i][l] -> addCondition(copyCondition);
//                    }
//                    Condition* copyCondition = new Condition(*condition);
//                    copyCondition -> setTargetState(stateVarsVec[i][k]);
//                    stateVarsVec[i][j - 1] -> addCondition(copyCondition);
//                    Condition* nullCondition = new Condition(*condition);
//                    nullCondition -> setTargetAct(nullAct);
//                    stateVarsVec[i][j] -> addCondition(nullCondition);
//                } else {
//                    for(ID l = k; l < j; l++)
//                    {
//                        Condition* copyCondition = new Condition(*condition);
//                        copyCondition -> setTargetState(stateVarsVec[i][l + 1]);
//                        stateVarsVec[i][l] -> addCondition(copyCondition);
//                    }
//                    Condition* copyCondition = new Condition(*condition);
//                    copyCondition -> setTargetState(stateVarsVec[i][k]);
//                    stateVarsVec[i][j] -> addCondition(copyCondition);
//                }
//            }
//        }
//    }    
//}
//
//
//void ASMSolver::newTskNoIst(Activity* act, Transition* transition, FLAG flag)
//{
//    Activity* targetAct = transition -> getActivity();
//    Aft targetAft = targetAct -> getAft();
//    for (ID i = 0; i < stackNum; i++) {
//        for (Aft aft : afts) {
//            for (ID j : act -> getAvailablePos()[aft]) {
//                //top stack is targetStack
//                if (aft == targetAft) {
//                    Condition* condition =  new Condition(stackLength);
//                    condition -> addTrueAftCondition(aftVars[i], targetAft);
//                    condition -> setCharCondition(charVar, transition);
//                    condition -> addTrueCondition(stateVarsVec[i][j], act);
//                    if(j < stackLength - 1)
//                    condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                    if(i < stackNum - 1)
//                    condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                    if (Activity::isFIN(transition)) {
//                        if (flag != CTK) {
//                            condition -> setTargetAct(targetAct);
//                            if (flag == CTP || flag == FLAG(-1) || flag ==RTF) {
//                                for (ID l : targetAct -> getAvailablePos()[aft]) {
//                                    if (l >= j) continue;
//                                    condition -> addFalseCondition(stateVarsVec[i][l], targetAct);
//                                }
//                            }
//                            stateVarsVec[i][j] -> addCondition(condition);
//                        }
//                    } else {
//                        if (flag == NTK) {
//                            if (j < stackLength - 1) {
//                                condition -> setTargetAct(targetAct);
//                                stateVarsVec[i][j + 1] -> addCondition(condition);
//                            }
//                        } else if (flag == CTP || flag == FLAG(-1)) {
//                            if (j < stackLength - 1) {
//                                Condition* copyCondition = new Condition(*condition);
//                                for (ID l : targetAct -> getAvailablePos()[aft]) {
//                                    if (l >= j) continue;
//                                    copyCondition -> addFalseCondition(stateVarsVec[i][l], targetAct);
//                                }
//                                copyCondition -> setTargetAct(targetAct);
//                                stateVarsVec[i][j + 1] ->addCondition(copyCondition);
//                            }
//                            for (ID k : targetAct -> getAvailablePos()[aft]) {
//                                if (k >= j) continue; 
//                                for (ID l = j; l > k; l--) {
//                                    Condition* copyCondition = new Condition(*condition);
//                                    copyCondition -> addTrueCondition(stateVarsVec[i][k], targetAct);
//                                    copyCondition -> setTargetAct(nullAct);
//                                    stateVarsVec[i][l] -> addCondition(copyCondition);
//                                }
//                            }
//
//                        }
//                    }
//                    if (flag == CTK) {
//                        cp4ClrTsk(i, i, condition, targetAct);
//                    } 
//                } else {
//                    if(i < stackNum - 1) {
//                        Condition* condition = new Condition(stackLength);
//                        for(ID k = 0; k <= i; k++)
//                            condition -> addFalseAftCondition(aftVars[k], targetAft);
//                        condition -> setCharCondition(charVar, transition);
//                        condition -> addTrueCondition(stateVarsVec[i][j], act);
//                        
//                        if(j < stackLength - 1)
//                        condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                        condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                        Condition* AftCondition = new Condition(*condition);
//                        if (Activity::isFIN(transition)) {
//                            Condition* nullCondition = new Condition(*condition);
//                            nullCondition -> setTargetAct(nullAct);
//                            stateVarsVec[i][j] -> addCondition(nullCondition);
//                        }
//                        condition -> setTargetAct(targetAct);
//                        AftCondition -> setTargetAft(targetAft);
//                        stateVarsVec[i + 1][0] -> addCondition(condition);
//                        aftVars[i + 1] -> addCondition(AftCondition);
//                    }
//
//                    //there exists a stack S , Aft(S) = Aft(B).
//                    for (ID k = 0; k < i; k++) {
//                        Condition* condition =  new Condition(stackLength);
//                        condition -> addTrueAftCondition(aftVars[k], targetAft);
//                        condition -> setCharCondition(charVar, transition);
//                        condition -> addTrueCondition(stateVarsVec[i][j], act);
//                        if(j < stackLength - 1)
//                        condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//                        if(i < stackNum - 1)
//                        condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//                        
//                        if (Activity::isFIN(transition) && j == 0 && i > 0) {
//                            if (flag == CTK) {
//                                cp4ClrTsk(i - 1, k, condition, targetAct);
//                                if (Activity::isFIN(transition)) {
//                                    mv2Top(i, k, condition, 1, j);
//                                } else {
//                                    mv2Top(i, k, condition, 0, 0);
//                                }
//                            }
//                        } else {
//                            if (flag == CTK) {
//                                cp4ClrTsk(i, k, condition, targetAct);
//                                if (Activity::isFIN(transition)) {
//                                    mv2Top(i, k, condition, 1, j);
//                                } else {
//                                    mv2Top(i, k, condition, 0, 0);
//                                }
//                            }
//                        }
//                        for(ID l : targetAct -> getAvailablePos()[targetAft])
//                        //there exists target Act
//                        {
//                            if (flag != NTK) {
//                                Condition* copyCondition = new Condition(*condition);
//                                copyCondition -> addTrueCondition(stateVarsVec[k][l], targetAct);         
//
//                                if (Activity::isFIN(transition) && j == 0 && i > 0) {
//                                    mv2Top(i, k, copyCondition, 1, j);
//                                    if(flag == CTP || flag == FLAG(-1))
//                                        cp4ClrTop(i - 1, k, l, copyCondition);
//                                    else if(flag == RTF)
//                                        cp4Rd2Frt(i - 1, k, l, copyCondition);
//                                } else {
//                                    if (Activity::isFIN(transition)) {
//                                        mv2Top(i, k, copyCondition, 1, j);
//                                    } else {
//                                        mv2Top(i, k, copyCondition, 0, 0);
//                                    }
//                                    if(flag == CTP || flag == FLAG(-1))
//                                        cp4ClrTop(i, k, l, copyCondition);
//                                    else if(flag == RTF)
//                                        cp4Rd2Frt(i, k, l, copyCondition);
//                                }
//                            }
//                        }
//                        for(ID m : targetAct -> getAvailablePos()[targetAft])
//                        //there is no target Act                        
//                        {
//                            if (flag == CTP || flag == RTF || flag == FLAG(-1)) {
//                                Condition* copyCondition = new  Condition(*condition);
//                                if (m > 0)
//                                    copyCondition -> addFalseCondition(stateVarsVec[k][m - 1], nullAct);
//                                copyCondition -> addTrueCondition(stateVarsVec[k][m], nullAct);
//                                for (ID l : targetAct -> getAvailablePos()[targetAft])
//                                    if (l < m)
//                                      copyCondition -> addFalseCondition(stateVarsVec[k][l], targetAct);         
//                                 
//                                if (Activity::isFIN(transition) && j == 0 && i > 0) {
//                                    mv2Top(i, k, copyCondition, 1, j);
//                                    cp4NewAct(i - 1, k, m, copyCondition, targetAct);
//                                } else {
//                                    if (Activity::isFIN(transition)) {
//                                    mv2Top(i, k, copyCondition, 1, j);
//                                } else {
//                                    mv2Top(i, k, copyCondition, 0, 0);
//                                }
//                                    cp4NewAct(i, k, m, copyCondition, targetAct);
//                                }
//                            } else if (flag == NTK) {
//                                Condition* copyCondition = new  Condition(*condition);
//                                if (m > 0)
//                                    copyCondition -> addFalseCondition(stateVarsVec[k][m - 1], nullAct);
//                                copyCondition -> addTrueCondition(stateVarsVec[k][m], nullAct);
//                                 
//                                if (Activity::isFIN(transition) && j == 0 && i > 0) {
//                                    mv2Top(i, k, copyCondition, 1, j);
//                                    cp4NewAct(i - 1, k, m, copyCondition, targetAct);
//                                } else {
//                                    if (Activity::isFIN(transition)) {
//                                    mv2Top(i, k, copyCondition, 1, j);
//                                } else {
//                                    mv2Top(i, k, copyCondition, 0, 0);
//                                }
//                                    cp4NewAct(i, k, m, copyCondition, targetAct);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}
//
////void ASMSolver::newTskNoIst(Activity* act, Transition* transition, FLAG flag)
////{
////    Activity* targetAct = transition -> getActivity();
////    Aft targetAft = targetAct -> getAft();
////    for(ID i = 0; i < stackNum; i++)
////    {
////        for(Aft aft : afts) 
////            for(ID j : act -> getAvailablePos()[aft])
////            {
////                Condition* condition =  new Condition(stackLength);
////                condition -> addTrueAftCondition(aftVars[i], targetAft);
////                condition -> setCharCondition(charVar, transition);
////                condition -> addTrueCondition(stateVarsVec[i][j], act);
////                if(j < stackLength - 1)
////                condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
////                if(i < stackNum - 1)
////                condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
////                
////                if (Activity::isFIN(transition)) {
////                    if (flag != CTK) {
////                        condition -> setTargetAct(targetAct);
////                        if (flag == CTP || flag == FLAG(-1) || flag ==RTF) {
////                            for (ID l : targetAct -> getAvailablePos()[aft]) {
////                                if (l >= j) continue;
////                                condition -> addFalseCondition(stateVarsVec[i][l], targetAct);
////                            }
////                        }
////                        stateVarsVec[i][j] -> addCondition(condition);
////                    }
////                } else {
////                    if (flag == NTK) {
////                        if (j < stackLength - 1) {
////                            condition -> setTargetAct(targetAct);
////                            stateVarsVec[i][j + 1] -> addCondition(condition);
////                        }
////                    } else if (flag == CTP || flag == FLAG(-1)) {
////                        if (j < stackLength - 1) {
////                            Condition* copyCondition = new Condition(*condition);
////                            for (ID l : targetAct -> getAvailablePos()[aft]) {
////                                if (l >= j) continue;
////                                copyCondition -> addFalseCondition(stateVarsVec[i][l], targetAct);
////                            }
////                            copyCondition -> setTargetAct(targetAct);
////                            stateVarsVec[i][j + 1] ->addCondition(copyCondition);
////                        }
////                        for (ID k : targetAct -> getAvailablePos()[aft]) {
////                            if (k >= j) continue; 
////                            for (ID l = j; l > k; l--) {
////                                Condition* copyCondition = new Condition(*condition);
////                                copyCondition -> addTrueCondition(stateVarsVec[i][k], targetAct);
////                                copyCondition -> setTargetAct(nullAct);
////                                stateVarsVec[i][l] -> addCondition(copyCondition);
////                            }
////                        }
////
////                    }
////                }
////                if (flag == CTK) {
////                    cp4ClrTsk(i, i, condition, targetAct);
////                } 
////                for(ID k = 0; k < i; k++)
////                //there exists a stack S , Aft(S) = Aft(B).
////                {
////                    Condition* condition =  new Condition(stackLength);
////                    condition -> addTrueAftCondition(aftVars[k], targetAft);
////                    condition -> setCharCondition(charVar, transition);
////                    condition -> addTrueCondition(stateVarsVec[i][j], act);
////                    if(j < stackLength - 1)
////                    condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
////                    if(i < stackNum - 1)
////                    condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
////                    
////                    if (Activity::isFIN(transition) && j == 0 && i > 0) {
////                        if (flag == CTK) {
////                            cp4ClrTsk(i - 1, k, condition, targetAct);
////                            if (Activity::isFIN(transition)) {
////                                mv2Top(i, k, condition, 1, j);
////                            } else {
////                                mv2Top(i, k, condition, 0, 0);
////                            }
////                        }
////                    } else {
////                        if (flag == CTK) {
////                            cp4ClrTsk(i, k, condition, targetAct);
////                            if (Activity::isFIN(transition)) {
////                                mv2Top(i, k, condition, 1, j);
////                            } else {
////                                mv2Top(i, k, condition, 0, 0);
////                            }
////                        }
////                    }
////                    for(ID l : targetAct -> getAvailablePos()[targetAft])
////                    //there exists target Act
////                    {
////                        if (flag != NTK) {
////                            Condition* copyCondition = new Condition(*condition);
////                            copyCondition -> addTrueCondition(stateVarsVec[k][l], targetAct);         
////
////                            if (Activity::isFIN(transition) && j == 0 && i > 0) {
////                                mv2Top(i, k, copyCondition, 1, j);
////                                if(flag == CTP || flag == FLAG(-1))
////                                    cp4ClrTop(i - 1, k, l, copyCondition);
////                                else if(flag == RTF)
////                                    cp4Rd2Frt(i - 1, k, l, copyCondition);
////                            } else {
////                                if (Activity::isFIN(transition)) {
////                                    mv2Top(i, k, copyCondition, 1, j);
////                                } else {
////                                    mv2Top(i, k, copyCondition, 0, 0);
////                                }
////                                if(flag == CTP || flag == FLAG(-1))
////                                    cp4ClrTop(i, k, l, copyCondition);
////                                else if(flag == RTF)
////                                    cp4Rd2Frt(i, k, l, copyCondition);
////                            }
////                        }
////                    }
////                    for(ID m : targetAct -> getAvailablePos()[targetAft])
////                    //there is no target Act                        
////                    {
////                        if (flag == CTP || flag == RTF || flag == FLAG(-1)) {
////                            Condition* copyCondition = new  Condition(*condition);
////                            if (m > 0)
////                                copyCondition -> addFalseCondition(stateVarsVec[k][m - 1], nullAct);
////                            copyCondition -> addTrueCondition(stateVarsVec[k][m], nullAct);
////                            for (ID l : targetAct -> getAvailablePos()[targetAft])
////                                if (l < m)
////                                  copyCondition -> addFalseCondition(stateVarsVec[k][l], targetAct);         
////                             
////                            if (Activity::isFIN(transition) && j == 0 && i > 0) {
////                                mv2Top(i, k, copyCondition, 1, j);
////                                cp4NewAct(i - 1, k, m, copyCondition, targetAct);
////                            } else {
////                                if (Activity::isFIN(transition)) {
////                                mv2Top(i, k, copyCondition, 1, j);
////                            } else {
////                                mv2Top(i, k, copyCondition, 0, 0);
////                            }
////                                cp4NewAct(i, k, m, copyCondition, targetAct);
////                            }
////                        } else if (flag == NTK) {
////                            Condition* copyCondition = new  Condition(*condition);
////                            if (m > 0)
////                                copyCondition -> addFalseCondition(stateVarsVec[k][m - 1], nullAct);
////                            copyCondition -> addTrueCondition(stateVarsVec[k][m], nullAct);
////                             
////                            if (Activity::isFIN(transition) && j == 0 && i > 0) {
////                                mv2Top(i, k, copyCondition, 1, j);
////                                cp4NewAct(i - 1, k, m, copyCondition, targetAct);
////                            } else {
////                                if (Activity::isFIN(transition)) {
////                                mv2Top(i, k, copyCondition, 1, j);
////                            } else {
////                                mv2Top(i, k, copyCondition, 0, 0);
////                            }
////                                cp4NewAct(i, k, m, copyCondition, targetAct);
////                            }
////                    }
////                        
////                }
////            }
////            if(i < stackNum - 1)
////            {
////                Condition* condition = new Condition(stackLength);
////                for(ID k = 0; k <= i; k++)
////                    condition -> addFalseAftCondition(aftVars[k], targetAft);
////                condition -> setCharCondition(charVar, transition);
////                condition -> addTrueCondition(stateVarsVec[i][j], act);
////                
////                if(j < stackLength - 1)
////                condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
////                condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
////                Condition* AftCondition = new Condition(*condition);
////                if (Activity::isFIN(transition)) {
////                    Condition* nullCondition = new Condition(*condition);
////                    nullCondition -> setTargetAct(nullAct);
////                    stateVarsVec[i][j] -> addCondition(nullCondition);
////                }
////                condition -> setTargetAct(targetAct);
////                AftCondition -> setTargetAft(targetAft);
////                stateVarsVec[i + 1][0] -> addCondition(condition);
////                aftVars[i + 1] -> addCondition(AftCondition);
////            }
////         }
////    }
////}
//
//
//void ASMSolver::startStdWithNTK(Activity* act, Transition* transition)
//{
//    newTskNoIst(act, transition, NTK);
//}
//
//void ASMSolver::startStk(Activity* act, Transition* transition)
//{
//    newTskNoIst(act, transition, FLAG(-1));
//}
//
//void ASMSolver::startStdWithCTP_NTK(Activity* act, Transition* transition)
//{
//    newTskNoIst(act, transition, CTP);        
//}
//
//void ASMSolver::startActWithCTK_NTK(Activity *act, Transition* transition)
//{
//    newTskNoIst(act, transition, CTK);        
//}
//
//void ASMSolver::startStdWithRTF_NTK(Activity *act, Transition* transition)
//{
//    newTskNoIst(act, transition, RTF);        
//}
//
//
//void ASMSolver::mv2Top(ID topId, ID sourceId, Condition *condition, bool flag, ID ActId)
//{
//    if(topId == sourceId) return;
//    if (flag && ActId == 0) {
//        Condition* nullCondition = new Condition(*condition);
//        nullCondition -> setTargetAct(nullAct);
//        stateVarsVec[topId][0] -> addCondition(nullCondition);
//        Condition* aftCondition = new Condition(*condition);
//        aftCondition -> setTargetAft(nullAft);
//        aftVars[topId] -> addCondition(aftCondition);
//        for (ID i = sourceId; i < topId - 1; i++) {
//            Condition *aftCondition = new Condition(*condition);
//            aftCondition -> setTargetAftVar(aftVars[i + 1]);
//            aftVars[i] -> addCondition(aftCondition);
//            for(ID j = 0; j < stackLength; j++)
//            {
//                Condition *copyCondition = new Condition(*condition);
//                copyCondition -> setTargetState(stateVarsVec[i + 1][j]);
//                stateVarsVec[i][j] -> addCondition(copyCondition);
//            }
//        }
//    }
//    if (ActId != 0) {
//        for (ID i = sourceId; i < topId; i++) {
//            Condition *aftCondition = new Condition(*condition);
//            aftCondition -> setTargetAftVar(aftVars[i + 1]);
//            aftVars[i] -> addCondition(aftCondition);
//            for (ID j = 0; j < stackLength; j++) {
//                Condition *copyCondition = new Condition(*condition);
//                if (flag && i == topId - 1 && j == ActId) {
//                    copyCondition -> setTargetAct(nullAct);
//                    stateVarsVec[i][j] -> addCondition(copyCondition);
//                } else {
//                    copyCondition -> setTargetState(stateVarsVec[i + 1][j]);
//                    stateVarsVec[i][j] -> addCondition(copyCondition);
//                }
//            }
//        }
//    }
//    Condition *aftCondition = new Condition(*condition);
//    aftCondition -> setTargetAftVar(aftVars[sourceId]);
//    aftVars[topId] -> addCondition(aftCondition);
//}
//
//void ASMSolver::cp(ID topId, ID sourceId, Condition* condition)
//{
//    for(ID j = 0; j < stackLength; j++)
//    {
//        Condition *copyCondition = new Condition(*condition);
//        copyCondition -> setTargetState(stateVarsVec[sourceId][j]);
//        stateVarsVec[topId][j] -> addCondition(copyCondition);
//    }  
//}
//
//void ASMSolver::cp4NewAct(ID topId, ID sourceId, ID ActId, Condition* condition, Activity* targetAct)
//{
//    for(ID j = 0; j < stackLength; j++)
//    {
//        Condition *copyCondition = new Condition(*condition);
//        if(j == ActId)
//            copyCondition -> setTargetAct(targetAct);
//        else
//            copyCondition -> setTargetState(stateVarsVec[sourceId][j]);
//        stateVarsVec[topId][j] -> addCondition(copyCondition);
//    }  
//}
//
//void ASMSolver::cp4ClrTop(ID topId, ID sourceId, ID ActId, Condition* condition)
//{
//    for(ID j = 0; j < stackLength; j++)
//    {
//        Condition *copyCondition = new Condition(*condition);
//        if(j <= ActId)
//            copyCondition -> setTargetState(stateVarsVec[sourceId][j]);
//        else
//            copyCondition -> setTargetAct(nullAct);
//        stateVarsVec[topId][j] -> addCondition(copyCondition);        
//    }        
//}
//
//void ASMSolver::cp4ClrTsk(ID topId, ID sourceId, Condition* condition, Activity* targetAct)
//{
//    for(ID j = 0; j < stackLength; j++)
//    {
//        Condition *copyCondition = new Condition(*condition);
//        if(j == 0)
//            copyCondition -> setTargetAct(targetAct);
//        else
//            copyCondition -> setTargetAct(nullAct);
//        stateVarsVec[topId][j] -> addCondition(copyCondition);        
//    }        
//}
//
//void ASMSolver::cp4Rd2Frt(ID topId, ID sourceId, ID ActId, Condition* condition)
//{
//    for(ID j = 0; j < stackLength; j++)
//    {
//        Condition *copyCondition = new Condition(*condition);
//        if(j < ActId)
//            copyCondition -> setTargetState(stateVarsVec[sourceId][j]);
//        else if(j == stackLength - 1)
//            copyCondition -> setTargetState(stateVarsVec[sourceId][ActId]);
//        else
//            copyCondition -> setTargetState(stateVarsVec[sourceId][j + 1]);
//        stateVarsVec[topId][j] -> addCondition(copyCondition);        
//    }        
//}
//
//
//
//void ASMSolver::pop()
//{
//    for(ID i = 0; i < stackNum; i++)
//    {
//        for(ID j = 0; j < stackLength; j++)
//        {
//            Condition* condition = new Condition(stackLength);
//            condition -> setCharCondition(charVar, nullTrans);
//            condition -> addFalseCondition(stateVarsVec[i][j], nullAct);
//            if(j < stackLength - 1)    
//                condition -> addTrueCondition(stateVarsVec[i][j + 1], nullAct);
//            if(i < stackNum - 1)    
//                condition -> addTrueAftCondition(aftVars[i + 1], nullAft);
//            if(j == 0)
//            {
//                Condition* aftCondition = new Condition(*condition);
//                aftCondition -> setTargetAft(nullAft);
//                aftVars[i] -> addCondition(aftCondition);
//            }
//                condition -> setTargetAct(nullAct);
//                stateVarsVec[i][j] -> addCondition(condition);
//        }
//    }
//
//}
//
//string Condition::getCodeStrSimple(int flag)
//{
//    string res;
//    for(StateCondition stateCondition : trueStateConditions)
//    {
//        ID id = stateCondition.first -> getID();
//        string str = "c" + to_string(stateCondition.second -> getID());
//        res += "s" + to_string(id) + " = " + str + " & ";
//    }
//    for(AftCondition aftCondition : trueAftConditions)
//    {
//        ID id = aftCondition.first -> getID();
//        string str = aftCondition.second;
//        res += "a" + to_string(id) + " = " + str + " & ";
//    }
//    if(falseStateConditions.size() + falseAftConditions.size())
//    {
//        res += "!( FALSE |";
//        for(StateCondition stateCondition : falseStateConditions)
//        {
//            ID id = stateCondition.first -> getID();
//            string str = "c" + to_string(stateCondition.second -> getID());
//            res += "s" + to_string(id) + " = " + str + " | ";
//        }
//        for(AftCondition aftCondition : falseAftConditions)
//        {
//            ID id = aftCondition.first -> getID();
//            string str = aftCondition.second;
//            res += "a" + to_string(id) + " = " + str + " | ";
//        }
//        res = res.substr(0, res.length() - 2); 
//        res += ") & ";
//    }
//    //if(flagConditions.size() > 1) 
//    //    for(ID i = 1; i < flagConditions.size(); i++)
//    //    {
//    //        ID id = flagConditions[i].first -> getID();
//    //        string str = to_string(flagConditions[i].second);
//    //        res += "f" + to_string(id) + " = " + str + " & ";
//    //    }
//    string str = "c" + to_string(charCondition.second -> getID());
//    res += "c = " + str; 
//    if(flag == 1)
//    {
//        if(getTargetAct())
//        {
//            res += " : c" + to_string(getTargetAct() -> getID()) + " ;\n";
//        }
//        else
//        {
//            res += " : s" + to_string(getTargetState() -> getID()) + " ;\n";
//        }
//    }
//    else if(flag == 0)
//    {
//        if(getTargetAft() != "")
//        {
//            res += " : " + getTargetAft() + " ;\n";
//        }
//        else
//        {
//            res += " : a" +  to_string(getTargetAftVar() -> getID()) + " ;\n";
//        }
//    }
//    /*else if(flag == 2)
//    {
//       res += " : " + to_string(getJumpVar()) + " ;\n";
// 
//    }*/
//    res = replaceAll(res, " FALSE |", "");
//    return res;
//}
//
//string ASMSolver::getCodeStrSimple()
//{
//    string res;
//    for(ID i = 0; i < stackNum; i++)
//    {
//        for(ID j  =  0; j < stackLength; j++)
//        {
//            string id = to_string(stateVarsVec[i][j] -> getID());
//            res += "next(s" + id + ") := case\n";
//            for(Condition* condition : stateVarsVec[i][j] -> getConditions())
//            {
//                res += condition -> getCodeStrSimple(1);
//            }
//            res += "TRUE : s" + id + ";\nesac;\n";
//        }
//
//    }
//    for(ID i = 0; i < stackNum; i++)
//    {
//        ID id = aftVars[i] -> getID();
//        res += "next(a" + to_string(id) + ") := case\n";
//        for(Condition* condition : aftVars[i] -> getConditions())
//        {
//            if (condition -> isValid())
//                res += condition -> getCodeStrSimple(0);
//        }
//        res += "TRUE : a" + to_string(id) + ";\nesac;\n";
//    }
//    return res;
//}
//string ASMSolver::getVarStrSimple()
//{
//    string res = "MODULE main\nVAR\n";
//    string actStr = "{ ";
//    string transStr = "{ ";
//    string aftStr = "{ ";
//    string flagStr = "{ ";
//    for(ID i = 0; i < activities.size(); i ++)
//    {
//        if(i == activities.size() - 1)
//            actStr += "c" + to_string(activities[i] -> getID());
//        else
//            actStr += "c" + to_string(activities[i] -> getID()) + ",";
//    }
//    actStr += " }";
//    
//    for(ID i = 0; i < transitions.size(); i ++)
//    {
//        if(i == transitions.size() - 1)
//            transStr += "c" + to_string(transitions[i] -> getID());
//        else
//            transStr += "c" + to_string(transitions[i] -> getID()) + ",";
//    }
//    transStr += " }";
//
//    for(ID i = 0; i < afts.size(); i ++)
//    {
//        if(i ==  afts.size() - 1)
//            aftStr += afts[i];
//        else
//            aftStr += afts[i] + ",";
//    }
//    aftStr += " }";
//    for(ID i = 0; i < flagNum; i++)
//    {
//        if(i < flagNum - 1)
//            flagStr += to_string(i) + ",";
//        else
//            flagStr += to_string(i);
//    }
//    flagStr += " }";
//    for(ID i = 0; i < stackNum; i++)
//        for(ID j = 0;  j < stackLength; j++)
//        {
//            ID id = stateVarsVec[i][j] -> getID();
//            res += "s" + to_string(id) + " : " + actStr + ";\n";
//        }
//    for(ID i = 0; i < stackNum; i++)
//    {
//        ID id = aftVars [i] -> getID();
//        res += "a" + to_string(id) + " : "+ aftStr + ";\n";
//    }
//    ID id = charVar -> getID();
//    res += "c : " + transStr + ";\n";
//    return res;
//}
//
//string ASMSolver::getInitStrSimple()
//{
//    string res = "ASSIGN\n";
//    for(ID i = 0; i < stackNum; i++)
//        for(ID j = 0; j <  stackLength; j++)
//        {
//            ID id = stateVarsVec[i][j] -> getID();
//            res += "init(s" + to_string(id) + ") := c" + to_string(nullAct -> getID()) +";\n";
//        }
//
//    for(ID i = 0; i < stackNum; i++)
//    {
//        ID id = aftVars[i] -> getID();
//        res += "init(a" + to_string(id) + ") := " + nullAft +";\n";
//    }
//    return res;
//}
//
//
//string ASMSolver::getInvStr4BackPatten(Activity* cur, Activity* back)
//{
//    string cid = to_string(cur -> getID());
//    string bid = to_string(back -> getID());
//    string res = "";
//    for(Aft aft : afts)
//    {
//        for(ID i = 0; i  < stackNum; i++)
//        for(ID j : cur -> getAvailablePos()[aft])
//        {
//            string ctr = to_string(stateVarsVec[i][j] -> getID());
//            if(i == 0 && j == 0) continue;
//            res += "(s" + ctr + " = c" + cid + " & ( ";
//            if(j == 0 && i > 0)
//            {
//                for(ID k = 0;  k < stackLength; k++)
//                {
//                    if(k == stackLength - 1)
//                    res += "( s" + to_string(stateVarsVec[i - 1][k] -> getID()) + " = c" + bid + ") ) ) | ";
//                    else
//                    res += "( s" + to_string(stateVarsVec[i - 1][k] -> getID()) + " = c" + bid + " & s" + to_string(stateVarsVec[i - 1][k + 1] -> getID()) + " = c" + to_string(nullAct -> getID()) + ") | ";
//                } 
//                                
//            }
//            else
//            {
//                string btr = to_string(stateVarsVec[i][j - 1] -> getID());
//                res += "s" + btr + " = c" + bid + ") ) | ";
//            }
//        }            
//    }
//    if(res == "") return "INVARSPEC\n!(FALSE);";
//    res = res.substr(0, res.length() - 2);    
//    res = "INVARSPEC\n!( " + res + ");";
//    return res;  
//}
//
//
//
//string StateItem(ID sid, ID tid, bool f) {
//    if (f) {
//        return "s" + to_string(sid) + " = " + "c" + to_string(tid);
//    } else {
//        return "s" + to_string(sid) + " = " + "s" + to_string(tid);
//    }
//}
//
//string CharItem(ID tid) {
//    return "c = c" + to_string(tid);
//}
//
//string StackItem(ID sid, ID tid) {
//    return "a" + to_string(sid) + " = " + "a" + to_string(tid);
//}
//
//string StackItem(ID sid, string aft) {
//    return "a" + to_string(sid) + " = " + aft;
//}
//
//string And(string lhs, string rhs) {
//    return "(" + lhs + ")" + " & " + "(" + rhs + ")";
//}
//
//string Or(string lhs, string rhs) {
//    return lhs + " | " + rhs;
//}
//
//string Not(string str) {
//    return "!(" + str + ")";
//}
//    
//
//string ASMSolver::getInvStr4BackHijacking(Activity* act)
//{
//    string pid = to_string(act -> getID());
//    string res = "FALSE";
//    for (Aft aft : afts) {
//        if (act -> getAvailablePos()[aft].size() == 0) continue;
//        string res0 = "FALSE";
//        for (ID i = 0; i  < stackNum; i++) {
//            string res1 = "FALSE";
//            for(ID j : act -> getAvailablePos()[aft]) {
//                if (j < stackLength - 1) {
//                    res1 = Or(res1, 
//                              And(StateItem(stateVarsVec[i][j] -> getID(), act -> getID(), 1),
//                                  StateItem(stateVarsVec[i][j + 1] -> getID(), nullAct -> getID(), 1)));
//                } else {
//                    res1 = Or(res1, StateItem(stateVarsVec[i][j] -> getID(), act -> getID(), 1));
//                }
//            }
//            if (i < stackNum - 1 && res1 != "") {
//                res0 = Or(res0, And(StackItem(aftVars[i + 1] -> getID(), nullAft), res1));
//            } else {
//                res0 = Or(res0, res1);
//            }
//        }
//        res = Or(res, res0);
//    }
//    res = "INVARSPEC\n!( " + res + ");";
//    return res; 
//}
//
//string ASMSolver::getInvStr4TskReparentingHijacking(Acts& actSet, Aft targetAft)
//{
//    string res = "FALSE";
//    for (Aft aft : afts) {
//        ID sum = 0;
//        for (Activity* act : actSet)
//            sum += act -> getAvailablePos()[aft].size();
//        if (sum == 0) continue;
//        string res0 = "FALSE";
//        for (ID i = 0; i  < stackNum; i++) {
//            string res1 = "FALSE";
//            for (Activity* act : actSet) {
//                string res2 = "FALSE";
//                for (ID j : act -> getAvailablePos()[aft]) {
//                    if (j < stackLength - 1) {
//                        res2 = Or(res2, 
//                              And(StateItem(stateVarsVec[i][j] -> getID(), act -> getID(), 1),
//                                  StateItem(stateVarsVec[i][j + 1] -> getID(), nullAct -> getID(), 1)));
//                    } else {
//                        res2 = Or(res2, StateItem(stateVarsVec[i][j] -> getID(), act -> getID(), 1));
//                    }
//                }
//                res1 = Or(res1, res2);
//            }
//            if (i < stackNum - 1) {
//                res0 = Or(res0, And(StackItem(aftVars[i + 1] -> getID(), nullAft), res1));
//            } else {
//                res0 = Or(res0, res1);
//            }
//        }
//        res = Or(res, res0);
//    }
//    res = "INVARSPEC\n!( " + res + ");";
//    return res;  
//}
//
//
//string ASMSolver::getInvStr4MaxLength(ID length)
//{
//    string nullActStr = "c" + to_string(nullAct -> getID());
//    string res = "INVARSPEC\n!( ";
//    for(ID i = 0; i < stackNum; i++)
//    {
//        res += "! ( a" + to_string(aftVars[i] -> getID()) + " = " + nullAft + " ) & ! ( s" + to_string(stateVarsVec[i][length - 1] -> getID()) + " = " + nullActStr +" ) | ";
//    }
//    res = res.substr(0, res.length() - 2);    
//    res += ");";
//    return res;
//}
//
//
//bool isJumpTransition(Transition* transition)
//{
//    if(transition -> getActivity() -> getLmd() == lmd_stk)
//        return true;
//    for(FLAG f : transition -> getFlags())
//        if(f == NTK) return true;
//    return false;
//}
//
//
//void ASMSolver::preDetection4BackHijacking(VerificationDatas& datas, Aft targetAft)
//{
//    for(Activity* act : activities) 
//    {
//        Afts aftSet;
//        for(Transition* transition : act -> getTransitions())
//        {
//            Aft aft = transition -> getActivity() -> getAft();
//            if(isJumpTransition(transition))
//                aftSet.insert(aft);
//        }
//        for(Aft aft : aftSet)
//        {
//            if(aft != targetAft) continue;
//            int* value;
//            string* path;
//            string smv = getSmv() + getInvStr4BackHijacking(act);
//            VerificationData data = {act, aft, 0, smv, value, path, this};
//            datas.push_back(data);
//        }
//    }
//}
//
//void ASMSolver::preDetection4TskReparentingHijacking(VerificationDatas& datas, Aft targetAft)
//{
//    unordered_map<Activity*, Acts> act2ActsMap; 
//    for(Activity* act : activities)
//        act2ActsMap[act] = Acts();
//    for(Activity* act : activities) 
//    {
//        for(Transition* transition : act -> getTransitions())
//        {
//            Activity* activity = transition -> getActivity();
//            if(activity -> getAft() == targetAft && isJumpTransition(transition))
//                act2ActsMap[activity].insert(act);
//        }
//    }
//    for(auto pair : act2ActsMap)
//    {
//        if(pair.second.size() == 0) continue;
//        int* value;
//        string* path;
//        Activity* act = pair.first;
//        string smv = getSmv() + getInvStr4TskReparentingHijacking(pair.second, targetAft);
//        VerificationData data = {act, targetAft, 0, smv, value, path, this};
//        datas.push_back(data);
//    }
//}
//
//void ASMSolver::pre4BackPatten(VerificationDatas& datas, Activity* targetAct)
//{
//    for(Activity* act : activities)
//    {
//       if(act == nullAct) continue;
//       string smv = getSmv() + getInvStr4BackPatten(targetAct, act);
//       int* value;
//       string* path;
//       VerificationData data = {act, nullAft, 0, smv, value, path, this};
//       datas.push_back(data);
//    }
//
//}
//
//void ASMSolver::pre4MaxLength(VerificationDatas& datas)
//{
//    for(ID i = stackLength / 2 + 2; i <= stackLength; i++)
//    //for(ID i = stackLength; i <= stackLength; i++)
//    {
//        string smv = getSmv() + getInvStr4MaxLength(i);
//        int* value;
//        string* path;
//        VerificationData data = {nullAct, nullAft, i, smv, value, path, this};
//        datas.push_back(data);
//    }
//}
//
//
//void ASMSolver::multiDetection()
//{
//    //preDetecti on();
//    //ID threadCount = verifiedDatas.size();
//    //thread threads[threadCount];
//    //for (ID i = 0; i < threadCount; ++i) 
//    //{
//    //    threads[i] = thread(getNuxmvRes, this, verifiedDatas[i]);
//    //}
//}
//
//int ASMSolver::getNuxmvRes(string smv)
//{
//    cout << "start Nuxmv..." << endl;
//    ofstream out("out.smv");
//    if (out.is_open())
//    {
//        out << smv;
//        out.close();
//    }
//    char buf_ps[2048];
//    string result;
//    FILE* ptr = NULL;
//    regex patten_true(".*invariant.+true");
//    regex patten_false(".*invariant.+false");
//    if((ptr = popen("./nuXmv-1.1.1-Darwin/bin/nuXmv out.smv", "r")) != NULL)
//    {
//        
//        while(fgets(buf_ps, 2048, ptr) != NULL)
//            result.append(buf_ps);
//        pclose(ptr);
//        ptr = NULL;
//    }
//    system("rm out.smv");
////    cout<<result<<endl;
//    if(regex_search(result, patten_true)) return 0;
//    else if(regex_search(result, patten_false)) return 1;
//    else return -1;
//}
////
