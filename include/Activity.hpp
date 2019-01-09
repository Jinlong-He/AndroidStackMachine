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
namespace atm {
    class Activity;
    class Action;
    class Port;
    enum Lmd {lmd_std, lmd_stk, lmd_stp, lmd_sit};
    enum FLAG {CTK, CTP, NTK, RTF, STP, MTK, TOH, FIN};
    typedef string Aft;
    typedef size_t ID;
    //typedef unordered_set<FLAG> Flags;
    typedef vector<FLAG> Flags;
    typedef unordered_set<ID> IDs;
    typedef unordered_set<Aft> Afts;
    typedef unordered_set<Activity*> Acts;
    typedef unordered_map<Activity*, IDs> AvailablePos;
    typedef unordered_map<Activity*, Acts> Act2ActsMap;
    typedef unordered_map<Aft, Acts> Aft2ActsMap;
    typedef unordered_map<Activity*, vector<Acts> > Act2ActsVecMap;
    typedef unordered_map<Activity*, Act2ActsVecMap> ContentMap;


    /// \brief This class records some infomation for calculating the available positions.
    class Port {
    private:
        Activity* portActivity;     ///< the port activity;
        Activity* realActivity;     ///< the real activity of another task.
        Flags flags;                ///< the flags in action.
        bool fin;                   ///< the fin in action.
    public:
        /// \brief Default construction function, sets portActivity and realActivity to nullptr.
        Port() : portActivity(nullptr), realActivity(nullptr) {}

        /// \brief construction function with params.
        /// \param portAct The portActivity.
        /// \param realAct The realActivity.
        /// \param fs The flags.
        /// \param f The fin.
        Port(Activity* portAct, Activity* realAct, const Flags& fs, bool f) : portActivity(portAct), realActivity(realAct), flags(fs.begin(), fs.end()), fin(f) {}

        /// \brief desconstruction function.
        ~Port() {}

        /// \brief Gets Port Activity.
        /// \return Activity pointer.
        Activity* getPortActivity() {return portActivity;}
        Activity* getPortActivity() const {return portActivity;}

        /// \brief Gets Real Activity.
        /// \return Activity pointer.
        Activity* getRealActivity() {return realActivity;}
        Activity* getRealActivity() const {return realActivity;}

        /// \brief Gets flags.
        /// \return reference of Flags.
        Flags& getFlags() {return flags;}
        const Flags& getFlags() const {return flags;}

        /// \brief Gets fin.
        /// \return Boolean. 
        bool getFin() {return fin;}
        bool getFin() const {return fin;}

        static bool isEqualFlags(const Flags& lhsFlags, const Flags& rhsFlags) {
            if (lhsFlags.size() != rhsFlags.size()) return false;
            for (auto flag1 : lhsFlags) {
                bool f = false;
                for (auto flag2 : rhsFlags) {
                    if (flag1 == flag2)
                        f = true;
                }
                if (!f) return false;
            }
            return true;
        }
    };
    
    struct PortHash {
        size_t operator()(const Port* port) const {
            return ((size_t) (port -> getPortActivity()) ^ (size_t) (port -> getRealActivity())); 
        }
    };
    struct PortCmp {
        bool operator()(const Port* lhsPort, const Port* rhsPort) const {
            return (lhsPort -> getPortActivity() == rhsPort -> getPortActivity())
                 & (lhsPort -> getRealActivity() == rhsPort -> getRealActivity())
                 & (lhsPort -> getFin() == rhsPort -> getFin())
                 & (Port::isEqualFlags(lhsPort -> getFlags(), rhsPort -> getFlags()));
        }
    };
    typedef unordered_set<Port*, PortHash, PortCmp> Ports;
    typedef unordered_map<Activity*, Ports> PortMap;

    class Action
    {
    private:
        Activity* sourceAct;    ///< source activity in an action.
        Activity* targetAct;    ///< target activity in an action.
        Flags flags;            ///< the falgs set in an acton.
        ID id;                  ///< the uniqe identity for an action
        bool finish;            ///< represent start or finishstart.
    public:
        Action() : sourceAct(nullptr), targetAct(nullptr) {};
        Action(Activity* sAct, Activity* tAct, const Flags& fs, bool f, ID i = 0) : sourceAct(sAct), targetAct(tAct), flags(fs.begin(), fs.end()), id(i), finish(f) {}

        Activity* getSourceAct() {return sourceAct;}
        Activity* getSourceAct() const {return sourceAct;}
        Activity* getTargetAct() {return targetAct;}
        Activity* getTargetAct() const {return targetAct;}
        Flags& getFlags() {return flags;}
        const Flags& getFlags() const {return flags;}
        ID getID() {return id;}
        ID getID() const {return id;}
        bool isFinishStart() {return finish;}
        bool isFinishStart() const {return finish;}

        bool isSwitchingTaskAction(Aft aft);
        bool isNormalAction(Aft aft, Activity* act);
        bool isSTPAction(Activity* act);
        bool hasNTKFlag();
        bool hasCTKFlag();
        bool hasCTPFlag();
        bool hasRTFFlag();
    };
    struct ActionHash {
        size_t operator()(const Action* action) const {
            return ((size_t) (action -> getSourceAct()) ^ (size_t) (action -> getTargetAct())); 
        }
    };
    struct ActionCmp {
        bool operator()(const Action* lhsAction, const Action* rhsAction) const {
            return (lhsAction -> getSourceAct() == rhsAction -> getSourceAct())
                 & (lhsAction -> getTargetAct() == rhsAction -> getTargetAct())
                 & (lhsAction -> isFinishStart() == rhsAction -> isFinishStart())
                 & (Port::isEqualFlags(lhsAction -> getFlags(), rhsAction -> getFlags()));
        }
    };
    typedef unordered_set<Action*, ActionHash, ActionCmp> Actions;
    typedef unordered_map<Activity*, Actions> Act2ActionsMap;
    typedef unordered_map<Activity*, Act2ActionsMap> VirtualActionsMap;
    typedef unordered_map<Aft, Actions> Aft2ActionsMap;


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
        /// \brief Default construction function.
        Activity() {}
        
        /// \brief Default desconstruction function.
        ~Activity();

        /// \brief Construction function with params.
        /// \param name The name for this Activity.
        /// \param i The identity for this Activity.
        /// \param lmd The lunch mode for this Activity. default is lmd_std.
        /// \param aft The affinity for this Activity. default is "".
        Activity(const string& name, ID i, Lmd lmd = lmd_std, Aft aft = "") : id(i), degree(0), affinity(aft), lunchMode(lmd), activityName(name) {}
        
        /// \brief Gets the id for this Activity.
        /// \return ID.
        ID getID() {return id;}

        /// \brief Gets the degree for this Activity.
        /// \return ID.
        ID getDegree() {return degree;}

        /// \brief Gets the affinity for this Activity.
        /// \return Aft.
        Aft& getAft() {return affinity;}

        /// \brief Gets the lunch mode for this Activity.
        /// \return Lmd.
        Lmd getLmd() {return lunchMode;}

        /// \brief Gets the name for this Activity.
        /// \return string.
        string& getName() {return activityName;}

        /// \brief Gets the actions for this Activity.
        /// \return The reference of Action set.
        Actions& getActions() {return actions;}

       /// \brief Gets the available positions for this Activity.
        /// \return The reference of map.
        AvailablePos& getAvailablePos() {return availablePos;}

        /// \brief Lets attribute degree increase 1.
        void addDegree() {degree++;}

        /// \brief Adds an param action in actions.
        /// \param action The action to be added.
        void addAction(Action* action);

        /// \brief Adds a position param pos within param aft in the available positions add update the param maxLength.
        ///
        /// If param pos is a new position in the availble positions within param aft, then add it and return true.
        /// Else if param pos is not a new one, but param visited is a new one in contentMap with param aft, return true.
        /// Else return false. 
        /// \param aft The affinity to be focuced on.
        /// \param pos The position to be added into available positions.
        /// \param maxLength The max length to be updated by the position.
        /// \param visited A acivity set records the whole activity in this content.
        /// \param contenMap A map records all contents of the activity within all affinities.
        /// \return A boolean means whether this content with param pos and param visited within param aft is new for this Activity.
        bool addAvailablePos(Activity* act, ID pos, ID& maxLength, const Acts& visited, ContentMap& contentMap);

        void addAvailablePos(Activity* realAct, ID pos);

        /// \brief Finds a action in the actions which can cause switching task and put this Activity into param exitMap within param aft and this action into entranceMap within param targetAft.
        ///
        /// If an action within param aft which will cause switching task against param targetAft, then put this Activity into exitMap within param aft and put this action into entranceMap within param targetAft.
        /// Else if an action within param aft will not cause, then let the activity in this action involk this function.
        /// \param visited A activity set records visited activites from the begining.
        /// \param exitMap A map records the exit port activities within all affinities. 
        /// \param entranceMap A map records the entrance port actions within all affinities.
        /// \param aft The affinity to be focuced.
        /// \param targetAft The target affinity to be compared with by determining switching task.
        void mkExitAndEntranceMap(const Acts& visited, PortMap& exitMap, PortMap& entranceMap, Activity* realAct, Acts& tasks, Act2ActsMap& taskMap, Activity* mainAct, Aft& ignoreAft, VirtualActionsMap& virtualActionsMap, Act2ActionsMap& visitedActions, Act2ActionsMap& availableActions);


    void mkOutActionsMap(Acts& visited, Activity* realAct, Aft2ActionsMap& actionsMap, Acts& reachActs, Actions& reachActions);
        
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

    
    class Task
    {
    private:
        Aft aft;            ///< the affinity for this task which is same with realAct's affinity.
        Activity* realActivity;  ///< the Activity which create this task.   
    public:
        Task() : realActivity(nullptr) {} 
        Task(Aft a, Activity* realAct) : aft(a), realActivity(realAct) {}

        Aft& getAft() {return aft;}
        Activity* getRealActivity() {return realActivity;}
    };

    

}

#endif /* Activity_hpp */
