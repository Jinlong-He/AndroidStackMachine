//
//  ATM.cpp
//  ATM
//
//  Created by 何锦龙 on 2018/8/22.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#include "ATM.hpp"
#include "cgh/State.hpp"
using namespace cgh;
ID State::counter = 0;
template<> ID Global<ID>::epsilon = 0;
namespace atm {
    ATM::ATM(Parse& parse) : maxLength(0), window(0), ignoreAft(""), mainActivity(parse.getMainActivity()), mainAft(parse.getMainAft()), afts(parse.getAfts().begin(), parse.getAfts().end()) {
        nullActivity = new Activity("null", 0);
        botActivity = new Activity("bot", 1);
        for (Activity* act : parse.getActivities()) {
            if (act -> getDegree() > 0) {
                activities.insert(act);           
                actions.insert(act -> getActions().begin(), act -> getActions().end());
            }
        }
        Activity* botAct = new Activity();
        mainAction = new Action(botActivity, mainActivity, Flags(), parse.getActions().size() + 1);
        actions.insert(mainAction);
    }

    ATM::~ATM() {
        for (Activity* act : activities)
            delete act;
        for (Action* action : actions)
            delete action;
        for (auto& mapPair : dfaMap) {
            delete mapPair.second;
        }
        delete nullActivity;
        delete botActivity;
    }

    DFA<ID>* mkSITDFA(unordered_set<ID>& alphabet) {
        unordered_set<FA<ID>*> faSet;
        for (ID c : alphabet) {
            if (c <= 1) continue;
            DFA<ID>* dfa = new DFA<ID>(alphabet);
            DFAState<ID>* initialState = dfa -> mkInitialState();
            DFAState<ID>* state = dfa -> mkState();
            DFAState<ID>* finalState = dfa -> mkFinalState();
            initialState -> addDFATrans(c, state);
            initialState -> addDFATrans(1, finalState);
            state -> addDFATrans(1, finalState);
            for (ID c1 : alphabet) {
                if (c1 != c) {
                    initialState -> addDFATrans(c1, initialState);
                    state -> addDFATrans(c1, state);
                }
            }
            faSet.insert(dfa);
        }
        DFA<ID>* dfa = &(FA<ID>::intersectFA(faSet)).determinize();
        for (FA<ID>* fa : faSet) {
            delete fa;
        }
        return dfa;
    }

    DFA<ID>* mkLengthDFA(ID length, unordered_set<ID>& alphabet) {
        DFA<ID>* dfa = new DFA<ID>(alphabet);
        DFAState<ID>* initialState = dfa -> mkInitialState();
        DFAState<ID>* finalState = dfa -> mkFinalState();
        vector<DFAState<ID>*> states;
        for (ID i = 0; i < length; i++) {
            states.push_back(dfa -> mkState());
        }
        for (ID c : alphabet) {
            if (c <= 1) continue;
            initialState -> addDFATrans(c, states[0]);
            for (ID i = 0; i < length - 1; i++) {
                states[i] -> addDFATrans(c, states[i + 1]);
            }
        }
        states[length - 1] -> addDFATrans(1, finalState);
        return dfa;
    }

    DFA<ID>* mkPosDFA(ID pos, Activity* act, unordered_set<ID>& alphabet) {
        DFA<ID>* dfa = new DFA<ID>(alphabet);
        DFAState<ID>* initialState = dfa -> mkInitialState();
        DFAState<ID>* actState = dfa -> mkState();
        DFAState<ID>* finalState = dfa -> mkFinalState();
        initialState -> addDFATrans(act -> getID(), actState);
        if (pos == 1) {
            actState -> addDFATrans(1, finalState);
        } else {
            vector<DFAState<ID>*> states;
            for (ID i = 1; i < pos; i++) {
                states.push_back(dfa -> mkState());
            }
            for (ID c : alphabet) {
                actState -> addDFATrans(c, states[0]);
                for (ID i = 0; i < pos - 2; i++) {
                    states[i] -> addDFATrans(c, states[i + 1]);
                }
            }
            states[pos - 2] -> addDFATrans(1, finalState);
        }
        return dfa;
    }


    void ATM::mkConfig(const Aft& tAft, ID w) {
        ignoreAft = tAft;
        window = w;
        for (Activity* act : activities) {
            if (act -> getLmd() == lmd_std || act -> getLmd() == lmd_stp) {
                stdActivities.insert(act);
            } else if (act ->getLmd() == lmd_stk) {
                stkActivities.insert(act);
            } else {
                sitActivities.insert(act);
            }
        }
        CompleteGraph();
        for (Aft aft : afts) {
            for (Activity* realAct : tasks) {
                //cout << realAct -> getName() << endl;
                if (realAct -> getAft() == aft) {
                    realActMap[aft].insert(realAct);
                }
            }
        }
        mkNFAs();
        mkLoopMap();
        getMaxLength(isBounded());
        getAvailablePos();
        Afts aftSet;
        for (Aft aft : afts) {
            bool flag = false;
            for (Activity* realAct : realActMap[aft]) {
                for (Activity* act : activities) {
                    if (act -> getAvailablePos()[realAct].size() > 0) {
                        flag = true;
                        break;
                    }
                }
            }
            if (flag) {
                aftSet.insert(aft);
            } else {
                realActMap.erase(aft);
            }
        }
        afts.clear();
        afts.insert(aftSet.begin(), aftSet.end());
    }

    void ATM::getMaxLength(bool bounded) {
        maxLength = 0;
        if (bounded) {
            cout << "Bounded" << endl;
            for (auto& mapPair : availableActs) {
                Aft aft = mapPair.first -> getAft();
                if (mapPair.second.size() == 0) continue;
                unordered_set<ID> alphabet;
                alphabet.insert(1);
                for (Activity* act : mapPair.second) {
                    alphabet.insert(act -> getID());
                }
                DFA<ID>* sit = dfaMap[mapPair.first];
                for (ID length = mapPair.second.size(); length > 0; length--) {
                    DFA<ID>* lengthDFA = mkLengthDFA(length, alphabet);
                    DFA<ID>* inter = &(*lengthDFA & *sit).determinize();
                    if (!inter -> isNULL()) {
                        delete inter;
                        delete lengthDFA;
                        maxLength = length > maxLength ? length : maxLength;
                        if (lengthMap.count(aft) == 0) {
                            lengthMap[aft] = length;
                        } else {
                            ID l = lengthMap[aft];
                            lengthMap[aft] = length > l ? length : l;
                        }
                        break;
                    }
                    delete inter;
                    delete lengthDFA;
                }
                //break;
            }
        } else {
            cout << "Unbounded" << endl;
            for (auto& mapPair : availableActs) {
                if (mapPair.second.size() == 0) continue;
                Aft aft = mapPair.first -> getAft();
                unordered_set<ID> alphabet;
                alphabet.insert(1);
                for (Activity* act : mapPair.second) {
                    alphabet.insert(act -> getID());
                }
                DFA<ID>* dfa = mkSITDFA(alphabet);
                DFA<ID>* sit = &(*dfa & *(dfaMap[mapPair.first])).determinize().minimize();
                for (ID length = mapPair.second.size(); length > 0; length--) {
                    DFA<ID>* lengthDFA = mkLengthDFA(length, alphabet);
                    DFA<ID>* inter = &(*lengthDFA & *sit).determinize();
                    if (!inter -> isNULL()) {
                        delete lengthDFA;
                        delete inter;
                        maxLength = length > maxLength ? length : maxLength;
                        if (lengthMap.count(aft) == 0) {
                            lengthMap[aft] = length;
                        } else {
                            ID l = lengthMap[aft];
                            lengthMap[aft] = length > l ? length : l;
                        }
                        break;
                    }
                    delete inter;
                    delete lengthDFA;
                }
                delete dfa;
                delete sit;
                //break;
            }
        }
        cout << "maxLength : "<< maxLength << endl;
    }

    void ATM::getAvailablePos() {
        for (auto& mapPair : availableActs) {
            //cout << mapPair.first -> getName() << endl;
            if (mapPair.second.size() == 0) continue;
            Activity* realAct = mapPair.first;
            DFA<ID>* dfa = dfaMap[realAct];
            unordered_set<ID> alphabet;
            alphabet.insert(1);
            for (Activity* act : mapPair.second) {
                alphabet.insert(act -> getID());
            }
            for (Activity* act : mapPair.second) {
                //cout << act -> getName() << " : ";
                for (ID pos = 1; pos <= lengthMap[realAct -> getAft()]; pos++) {
                    DFA<ID>* posDFA = mkPosDFA(pos, act, alphabet);
                    DFA<ID>* inter = &(*posDFA & *dfa).determinize();
                    if (!inter -> isNULL()) {
                        act -> addAvailablePos(realAct, pos);
                        //cout << pos << " ";
                    }
                    delete posDFA;
                    delete inter;
                }
                //cout << endl;
            }
            //cout << endl;
        }
    }

    //void ATM::getMaxLengthBySI(VirtualActionsMap& virtualActionsMap, bool flag) {
    //    ContentMap contentMap;
    //    for (Aft aft : afts) {
    //        contentMap[aft] = Act2ActsVecMap();
    //     - 1}
    //    mkExitAndEntranceMap(virtualActionsMap);
    //    Acts visited;
    //    visited.insert(mainActivity);
    //    mkAvailablePosBySI(visited, mainActivity, 0, mainAft, virtualActionsMap, contentMap, flag);
    //    for (Aft aft : afts) {
    //        for (Action* action : entranceMap[aft]) {
    //            visited.clear();
    //            Activity* newAct = action -> getActivity();
    //            visited.insert(newAct);
    //            mkAvailablePosBySI(visited, newAct, 0, aft, virtualActionsMap, contentMap, flag);
    //        }
    //    }
    //    Afts aftSet;
    //    for (Aft aft : afts) {
    //        bool flag = false;
    //        for (Activity* act : activities) {
    //            if (act -> getAvailablePos()[aft].size() > 0) {
    //                flag = true;
    //                break;
    //            }
    //        }
    //        if (flag) aftSet.insert(aft);
    //    }
    //    afts.clear();
    //    afts.insert(aftSet.begin(), aftSet.end());
    //}

    bool isChange(ID& size, ID& curSize, PortMap& exitMap, PortMap& entranceMap, Acts& tasks, Act2ActsMap& taskMap) {
        curSize = tasks.size();
        for (Activity* act : tasks) {
            curSize += exitMap[act].size() + entranceMap[act].size() + taskMap[act].size();
        }
        bool f = false;
        if (curSize > size) f = true;
        size = curSize;
        return f;
    }

    bool isChange(ID& size, ID& curSize, Act2ActsMap& taskMap) {
        curSize = 0;
        for (auto& mapPair : taskMap) {
            curSize += mapPair.second.size();
        }
        bool f = false;
        if (curSize > size) f = true;
        size = curSize;
        return f;
    }

    void getTaskMapClosure(Activity* act, Act2ActsMap& taskMap, Acts& acts, Acts& visited) {
        for (Activity* newAct : taskMap[act]) {
            if (visited.insert(newAct).second) {
                acts.insert(taskMap[newAct].begin(), taskMap[newAct].end());
            }
        }
    }

    void getTaskMapClosure(Act2ActsMap& taskMap) {
        ID size = 0;
        ID curSize = 0;
        while (isChange(size, curSize, taskMap)) {
            for (auto& mapPair : taskMap) {
                Acts visited;
                visited.insert(mapPair.first);
                getTaskMapClosure(mapPair.first, taskMap, mapPair.second, visited);
            }
        }
    }

    void ATM::mkExitAndEntranceMap(Act2ActsMap& taskMap, Act2ActionsMap& visitedActions) {
        ID size = 0;
        ID curSize = 0;
        while (isChange(size, curSize, exitMap, entranceMap, tasks, taskMap)) {
            for (Activity* act : tasks) {
                Acts visited;
                visited.insert(act);
                act -> mkExitAndEntranceMap(visited, exitMap, entranceMap, act, tasks, taskMap, mainActivity, ignoreAft, virtualActionsMap, visitedActions, availableActions);
            }
        }
    }
    bool hasCTPFlag(Flags& flags) {
            for (FLAG flag : flags)
                if (flag == CTP) return true;
            return false;
    }
    void ATM::addVirtualAction(Act2ActsMap& taskMap) {
        for (Activity* realAct : tasks) {
            for (Port* exitPort : exitMap[realAct]) {
                for (Port* entrancePort : entranceMap[realAct]) {
                    Activity* exRealAct = exitPort -> getRealActivity();
                    Activity* exPortAct = exitPort -> getPortActivity();
                    bool fin = exitPort -> getFin();
                    Activity* enRealAct = entrancePort -> getRealActivity();
                    Activity* enPortAct = entrancePort -> getPortActivity();
                    Flags& flags = entrancePort -> getFlags();
                    if (taskMap[exRealAct].count(enRealAct) != 0) {
                        if (realAct == enPortAct && realAct != mainActivity && !hasCTPFlag(flags)) 
                            continue;
                        virtualActionsMap[realAct][exPortAct].insert(new Action(exPortAct, enPortAct, flags, fin));
                    }
                }
            }
        }
    }
    
    void ATM::CompleteGraph() {
        Act2ActsMap taskMap;
        Act2ActionsMap visitedActions;
        tasks.insert(mainActivity);
        ID size = 0;
        ID curSize = 0;
        while (isChange(size, curSize, exitMap, entranceMap, tasks, taskMap)) {
            mkExitAndEntranceMap(taskMap, visitedActions);
            getTaskMapClosure(taskMap);
            addVirtualAction(taskMap);
        }
        //for (auto& mapPair : taskMap) {
        //    cout << mapPair.first -> getName() << " : ";
        //    for (Activity* act : mapPair.second) {
        //        cout << act -> getName() << " ";
        //    }
        //    cout << endl;
        //}
    }

    bool HasSameAct(const Acts& acts1, const Acts& acts2) {
        for (Activity* act : acts1) {
            if (acts2.count(act) > 0) return true;
        }
        return false;
    }

    void getLoopActs(unordered_map<Activity*, pair<Acts, Act2ActionsMap> >& act2PrePostActsMap) {
        bool flag = true;
        while (flag && act2PrePostActsMap.size() > 0) {
            flag = false;
            for (auto& pair : act2PrePostActsMap) {
                if (pair.second.first.size() == 0) {
                    act2PrePostActsMap.erase(pair.first);
                    for (auto& pair1 : act2PrePostActsMap) {
                        pair1.second.first.erase(pair.first);
                    }
                    flag = true;
                    break;
                } else if (pair.second.second.size() == 0) {
                    act2PrePostActsMap.erase(pair.first);
                    for (auto& pair1 : act2PrePostActsMap) {
                        pair1.second.second.erase(pair.first);
                    }
                    flag = true;
                    break;
                }
            }
        }
    }

    void getLoopByAct(Activity* act, Acts& visited, unordered_map<Activity*, pair<Acts, Act2ActionsMap> >& act2PrePostActsMap, Acts& minActs) {
        for (auto& pair : act2PrePostActsMap[act].second) {
            if (visited.count(pair.first) > 0) {
                minActs.insert(visited.begin(), visited.end());
                return;
            }         
        }
        for (auto& pair : act2PrePostActsMap[act].second) {
            Acts newMinActs;
            Acts newVisited(visited.begin(), visited.end());
            newVisited.insert(pair.first);
            getLoopByAct(pair.first, newVisited, act2PrePostActsMap, newMinActs);
            if (newMinActs.size() == 0) continue;
            if (minActs.size() == 0 || minActs.size() > newMinActs.size()) {
                minActs.clear();
                minActs.insert(newMinActs.begin(), newMinActs.end());
            }
        }
    }

    void getMiniLoopActs(unordered_map<Activity*, pair<Acts, Act2ActionsMap> >& act2PrePostActsMap, Acts& loopActs, Acts& minActs) {
        for (Activity* act : loopActs) {
            Acts visited;
            Acts newMinActs;
            visited.insert(act);
            getLoopByAct(act, visited, act2PrePostActsMap, newMinActs);
            if (newMinActs.size() == 0) continue;
            if (minActs.size() == 0 || minActs.size() > newMinActs.size()) {
                minActs.clear();
                minActs.insert(newMinActs.begin(), newMinActs.end());
            }
        }
    }

    void ATM::mkLoopMap() {
        for (Activity* realAct : tasks) {
            Aft aft = realAct -> getAft();
            unordered_map<Activity*, pair<Acts, Act2ActionsMap> > act2PrePostActsMap;
            for (Action* action : availableActions[realAct]) {
                Activity* act = action -> getSourceAct();
                if (action -> isNormalAction(aft, act)) {
                    Activity* postActivity = action -> getTargetAct();
                    act2PrePostActsMap[act].second[postActivity].insert(action);
                    act2PrePostActsMap[postActivity].first.insert(act);
                }
            }
            getLoopActs(act2PrePostActsMap);
            if (act2PrePostActsMap.size() > 0) {
                Acts loopActs, minActs;
                for (auto& mapPair : act2PrePostActsMap) {
                    loopActs.insert(mapPair.first);
                }
                getMiniLoopActs(act2PrePostActsMap, loopActs, minActs);
                loopMap[realAct] = minActs;
                //cout << realAct -> getName() << " : " << endl;
                //for (Activity* act : loopActs) {
                //    cout << act -> getName() << endl;
                //}
                //cout << endl;
            }
        }
        //if (loopActs.size() > 0) return false;
        if (loopMap.size() > 0) {
            bounded = false;
        } else {
            bounded = true;
        }
    }

    void ATM::getCTPActs(Act2ActsMap& cActsMap) {
        for (Activity* realAct : tasks) {
            Aft realAft = realAct -> getAft();
            for (Action* action : availableActions[realAct]) {
                if (action -> hasCTPFlag()) {
                    Activity* newAct = action -> getTargetAct();
                    cActsMap[realAct].insert(newAct);
                }
            }
        }
    }

    template<class T>
    void Combination(vector<T>& datas,int len, vector<unordered_set<T> >& comDatas) {
        int n = 1 << len;
        for (int i = 1; i < n; i++) {
            unordered_set<T> newDatas;
            for (int j = 0; j < len; j++) {
                int temp = i;
                if (temp & (1 << j)) {
                    newDatas.insert(datas[j]);
                }
            }
            comDatas.push_back(newDatas);
        }
    }

    void getPowerSet(Acts& acts, vector<Acts>& powerSet) {
       vector<Activity*> datas(acts.begin(), acts.end()); 
       powerSet.push_back(Acts());
       Combination(datas, datas.size(), powerSet);
    }

    void getActsVec(Activity* act, Acts& cActs, vector<Acts>& powerSet, vector<Acts>& actsVec) {
        if (cActs.count(act) == 0) {
            actsVec.insert(actsVec.end(), powerSet.begin(), powerSet.end());
        } else {
            for (Acts& acts : powerSet) {
                if (acts.count(act) != 0) {
                    actsVec.push_back(acts);
                }
            }
        }
    }

    void ATM::getCharMap(Activity* realAct, Actions& actions, ActActsPairs& chars, ActActsPairMap& charMap, ID2Map& char2Map, Act2ActActsPairsMap& act2PairsMap, Acts& cActs, vector<Acts>& powerSet) {
        for (Action* action : actions) {
            availableActs[realAct].insert(action -> getSourceAct());
            availableActs[realAct].insert(action -> getTargetAct());
        }
        vector<Acts> actsVec;
        for (Activity* act : availableActs[realAct]) {
            actsVec.clear();
            getActsVec(act, cActs, powerSet, actsVec);
            chars.push_back(ActActsPair(nullActivity, Acts()));
            chars.push_back(ActActsPair(botActivity, Acts()));
            char2Map[0] = 0;
            char2Map[1] = 1;
            for (Acts& acts : actsVec) {
                ActActsPair pair(act, acts);
                act2PairsMap[act].push_back(pair);
                chars.push_back(pair);
                charMap[pair] = chars.size();
                char2Map[chars.size()] = act -> getID();
            }
        }
    }

    void mkPDS(PDS<ID>* pds, Activity* realAct, Activity* mainActivity, Actions& actions, ActActsPairMap& charMap, Act2ActActsPairsMap& act2PairsMap, Acts& cActs) {
        PDSState* state = pds -> mkControlState();
        if (cActs.count(realAct) == 0) {
            pds -> mkPushPDSTrans(state, state, 1, pair<ID, ID>(charMap[ActActsPair(realAct, Acts())], 1));
        } else {
            Acts realActs;
            realActs.insert(realAct);
            pds -> mkPushPDSTrans(state, state, 1, pair<ID, ID>(charMap[ActActsPair(realAct, realActs)], 1));
        }
        Acts unionActs;
        for (Action* action : actions) {
            Activity* sourceAct = action -> getSourceAct();
            Activity* targetAct = action -> getTargetAct();
            if (action -> hasNTKFlag() && targetAct == realAct && targetAct != mainActivity)
                continue;
            ActActsPairs& sourcePairs = act2PairsMap[sourceAct];
            for (auto sPair : sourcePairs) {
                ID sChar = charMap[sPair];
                ID tChar = 0;
                bool flag = true;
                if (cActs.count(targetAct) != 0) {
                    unionActs.clear();
                    unionActs.insert(sPair.second.begin(), sPair.second.end());
                    unionActs.insert(targetAct);
                    tChar = charMap[ActActsPair(targetAct, unionActs)];
                    if (action -> hasCTPFlag()) {
                        if (sPair.second.count(targetAct) != 0) {
                            flag = false;
                        }
                    } 
                } else {
                    tChar = charMap[ActActsPair(targetAct, sPair.second)];
                }
                if (!flag) continue;
                if (action -> isFinishStart()) {
                    pds -> mkReplacePDSTrans(state, state, sChar, tChar);
                } else {
                    pds -> mkPushPDSTrans(state, state, sChar, pair<ID, ID>(tChar, sChar));
                }
            }
        }
    }

    NFA<ID>* mkNFA(PDS<ID>* pds, const unordered_set<ID>& alphabet, ID2Map& char2Map) {
        NFA<ID>* nfa = new NFA<ID>(alphabet);
        NFAState<ID>* state = nfa -> mkInitialState();
        NFAState<ID>* finalState = nfa -> mkFinalState();
        state -> addNFATrans(1, finalState);
        Global<ID>::PDSState2NFAStateMap state2Map;
        state2Map[*(pds -> getControlStateSet().begin())] = state;
        NFA<ID>* postStar = &(nfa -> postStar(*pds, state2Map));
        NFA<ID>* res = new NFA<ID>(*postStar, char2Map);
        delete postStar;        
        delete nfa;
        return res;
    }

    void ATM::mkNFAs() {
        Act2ActsMap cActsMap;
        getCTPActs(cActsMap);
        for (Activity* realAct : tasks) {
            //if (realAct != mainActivity) continue; 
            ActActsPairs chars;
            ActActsPairMap charMap;
            ID2Map char2Map;
            Act2ActActsPairsMap act2PairsMap;
            vector<Acts> powerSet;
            getPowerSet(cActsMap[realAct], powerSet);
            getCharMap(realAct, availableActions[realAct], chars, charMap, char2Map, act2PairsMap, cActsMap[realAct], powerSet);
            PDS<ID>* pds = new PDS<ID>();
            mkPDS(pds, realAct, mainActivity, availableActions[realAct], charMap, act2PairsMap, cActsMap[realAct]);
            unordered_set<ID> alphabet;
            for (ID i = 1; i <= chars.size(); i++) {
                alphabet.insert(i);
            }
            NFA<ID>* nfa = mkNFA(pds, alphabet, char2Map);
            DFA<ID>* dfa = &(nfa -> determinize());
            DFA<ID>* mini = &(dfa -> minimize());
            dfaMap[realAct] = mini;
            delete nfa;
            delete pds;
            delete dfa;
            //nfa -> determinize().minimize().print("res.dot");
        }
    }

    //void ATM::getAvailablePos() {

    //}

    //bool ATM::mkAvailablePosBySI(Acts& visited, Activity* act, ID pos, Aft aft, VirtualActionsMap& virtualActionsMap, ContentMap& contentMap, bool flag) {
    //    if (pos > window) return false;
    //    if (!(act -> addAvailablePos(aft, pos, maxLength, visited, contentMap))) return false;
    //    Actions actions(act -> getActions().begin(), act -> getActions().end());
    //    actions.insert(virtualActionsMap[aft][act].begin(), virtualActionsMap[aft][act].end());
    //    for (Action* action : actions) {
    //        if (action -> isSwitchingTaskAction(aft)) continue;
    //        Activity* newAct = action -> getActivity();
    //        Aft newAft = newAct -> getAft();
    //        Acts newVisited(visited.begin(), visited.end());
    //        if (action -> hasCTKFlag()) {
    //            mkAvailablePosBySI(newVisited, newAct, 0, aft, virtualActionsMap, contentMap, flag);
    //            continue;
    //        }
    //        if (newVisited.insert(newAct).second) {
    //            if (action -> hasFINFlag()) {
    //                newVisited.erase(act);
    //                mkAvailablePosBySI(newVisited, newAct, pos, aft, virtualActionsMap, contentMap, flag);
    //            } else {
    //                mkAvailablePosBySI(newVisited, newAct, pos + 1, aft, virtualActionsMap, contentMap, flag);
    //            }
    //        } else {
    //            if (!flag) {
    //                if (action -> isNormalAction(aft, act)){
    //                    if (action -> hasFINFlag()) {
    //                        newVisited.erase(act);
    //                        mkAvailablePosBySI(newVisited, newAct, pos, aft, virtualActionsMap, contentMap, flag);
    //                    } else {
    //                        mkAvailablePosBySI(newVisited, newAct, pos + 1, aft, virtualActionsMap, contentMap, flag);
    //                    }
    //                }
    //            }
    //            if (action -> hasRTFFlag()) {
    //                if (action -> hasFINFlag()) {
    //                    newVisited.erase(act);
    //                    mkAvailablePosBySI(newVisited, newAct, pos - 1, aft, virtualActionsMap, contentMap, flag);
    //                } else {
    //                    mkAvailablePosBySI(newVisited, newAct, pos, aft, virtualActionsMap, contentMap, flag);
    //                }
    //                newVisited.erase(newAct);
    //                for (Activity* act : newVisited) {
    //                    for (ID i : act -> getAvailablePos()[aft]) {
    //                        if (i > 0 && i <= pos)
    //                            mkAvailablePosBySI(newVisited, act, i - 1, aft, virtualActionsMap, contentMap, flag);
    //                    }
    //                }
    //            }
    //        }
    //    }
    //    return true;
    //}
} 
