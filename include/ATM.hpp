//
//  ATM.hpp
//  ATM
//
//  Created by 何锦龙 on 2018/8/22.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef ATM_hpp
#define ATM_hpp

#include <stdio.h>
#include "Activity.hpp"
#include "Parse.hpp"
#include "cgh/NFA.hpp"
#include "cgh/DFA.hpp"
using namespace cgh;
namespace atm {
    typedef unordered_map<Activity*, DFA<ID>*> DFAMap;
    typedef pair<Activity*, Acts> ActActsPair;
    typedef vector<ActActsPair> ActActsPairs;
    typedef unordered_map<ActActsPair, ID> ActActsPairMap;
    typedef unordered_map<ID, ID> ID2Map;
    typedef unordered_map<Activity*, vector<char> > Act2CharsMap;
    typedef unordered_map<Activity*, ActActsPairs> Act2ActActsPairsMap;
    typedef unordered_map<Aft, ID> Aft2IDMap;
    class ATM
    {
    private:
        Acts activities;                        ///< the activities set in ATM
        Acts stdActivities;                     ///< the standard activities set in ATM
        Acts stpActivities;                     ///< the singleTop activities set in ATM
        Acts stkActivities;                     ///< the singleTask activities set in ATM
        Acts sitActivities;                     ///< the singleInstance activities set in ATM
        Afts afts;                              ///< the affinities set of all activities in ATM
        Actions actions;                        ///< the actions set of all activities in ATM
        Action* mainAction;                     ///< the action of main activity in ATM
        Activity* mainActivity;                 ///< the main activity in ATM
        Activity* nullActivity;                 ///< the null activity in ATM
        Activity* botActivity;                  ///< the bot activity in ATM
        Aft mainAft;                            ///< the affinity of main activity in ATM
        Aft ignoreAft;                          ///< the target affinity using by analysising hijacking
        PortMap exitMap;                        ///< the exit port map records all Ports can cause switching task.
        PortMap entranceMap;                    ///< the enrance port map records all Ports can be caused switching task.
        VirtualActionsMap virtualActionsMap;    ///< the virtual actions for each task.        
        Acts tasks;                             ///< the task set of all posible tasks. 
        Aft2ActsMap realActMap;                 ///< the realActs of each task.
        Act2ActionsMap availableActions;        ///< the visitedActions of each task.
        Act2ActsMap availableActs;              ///< the visitedActions of each task.
        ID maxLength;                           ///< the max length for this ATM
        Aft2IDMap lengthMap;                    ///< the max legnth for each task.
        ID window;                              ///< the max length limitation for this ATM
        DFAMap dfaMap;                          ///< the NFA of each task.
        Act2ActsMap loopMap;                    ///< the loopActs of each task.
        bool bounded;                           ///< records this ATM is bounded or not.
    public:
        ATM() : mainActivity(nullptr) {} 
        ATM(Parse& parse);
        ~ATM();
        Afts& getAfts() {return afts;}
        Acts& getActivities() {return activities;}
        Actions& getActions() {return actions;}
        Action* getMainAction() {return mainAction;}
        Activity* getMainActivity() {return mainActivity;}
        Aft& getMainAft() {return mainAft;}
        Aft& getIgnoreAft() {return ignoreAft;}
        Aft2ActsMap& getRealActMap() {return realActMap;}
        Act2ActsMap& getLoopMap() {return loopMap;}
        Aft2IDMap& getLengthMap() {return lengthMap;}

        ID getStackLength() {return maxLength + 1;}
        ID getStackNum() {return afts.size();}
        bool isBounded() {return bounded;}

        void mkConfig(const Aft& tAft, ID w);
        void mkLoopMap();
        void mkExitAndEntranceMap(Act2ActsMap& taskMap, Act2ActionsMap& visitedActions);
        void addVirtualAction(Act2ActsMap& taskMap);
        void CompleteGraph();
        void getMaxLengthBySI(bool flag);
        void getMaxLength(bool bounded);
        void getAvailablePos();
        void getCTPActs(Act2ActsMap& cActsMap);
        void getCharMap(Activity* realAct, Actions& actions, ActActsPairs& chars, ActActsPairMap& charMap, ID2Map& char2Map, Act2ActActsPairsMap& act2PairsMap, Acts& cActs, vector<Acts>& powerSet);
        void mkNFAs();
    };
}
#endif /* ATM_hpp */
