//ATMSolver.hpp
//ASM
//
//Created by 何锦龙 on 2018/8/23.
//Copyright © 2018年 何锦龙. All rights reserved.


#ifndef ATMSolver_hpp
#define ATMSolver_hpp

#include <stdio.h>
#include "ATM.hpp"
#include "cgh/NuXmvSolver.hpp"
using namespace std;
using namespace cgh;
namespace atm {
    class ATMSolver;

    typedef vector<ID> Order;
    typedef vector<Order> Orders;
    typedef unordered_map<ID, Activity*> ID2ActMap;
    typedef unordered_map<ID, Action*> ID2ActionMap;
    typedef unordered_map<ID, Order> ID2OrderMap;
    typedef unordered_map<Aft, ID> Aft2IDMap;
    typedef unordered_map<Activity*, Value*> Act2ValueMap;
    typedef unordered_map<Action*, Value*> Action2ValueMap;
    typedef unordered_map<Order, Value*> Order2ValueMap;
    typedef unordered_map<ID, ID> ID2IDMap;

    struct VerificationData {
        Activity* act;
        Aft aft;
        string smv;
        int* value;
        string* path;
        ATMSolver* solver;
    };
    typedef vector<VerificationData> VerificationDatas;


    /// \brief This class is a Solver class for ATM based on NuXmvSolver.
    class ATMSolver : public NuXmvSolver {
    private:
        ATM* atm;                           ///< the ATM to be verified.
        Value* nullValue;                   ///< a special value for null.
        Value* popValue;                    ///< a special value for pop.
        ID2ActMap id2ActMap;                ///< a map from id to Activity in atm.
        ID2ActionMap id2ActionMap;          ///< a map from id to Action in atm.
        ID2OrderMap id2OrderMap;            ///< a map from id to Order in atm.
        ID2IDMap lengthMap;                 ///< a map from id to id in atm.
        Aft2IDMap aft2IDMap;                ///< a map from affinity to id.
        Act2ValueMap act2ValueMap;          ///< a map from Activity to Value.
        Action2ValueMap action2ValueMap;    ///< a map from Action to Value;
        Order2ValueMap order2ValueMap;      ///< a map from Order to Value;
        Orders orders;                      ///< records all orders for this class.
        ID stateVarNum;

    public:
        /// \brief Default construction function for this ATMSolver, set atm and nullValue to nullptr.
        ATMSolver() : atm(nullptr), nullValue(nullptr), popValue(nullptr) {}

        /// \brief Construction with one parameters.
        /// \param a The ATM to be verified.
        ATMSolver(ATM* a) : atm(a) {nullValue = new Value();}

        /// \brief Gets ID to Activity Map for this Solver.
        /// \return A reference of map from ID to Activity pointer.
        ID2ActMap& getID2ActMap() {return id2ActMap;}

        /// \brief Gets ID to Action Map for this Solver.
        /// \return A reference of map from ID to Action pointer.
        ID2ActionMap& getID2ActionMap() {return id2ActionMap;}


        /// \brief Gets the task length.
        /// \return ID.
        ID getTaskLength() {return atm -> getStackLength();}

        /// \brief Gets the task number.
        /// \reutrn ID.
        ID getTaskNum() {return atm -> getStackNum();}

        /// \brief Gets Activity set in atm.
        /// \return A set of Activity.
        Acts& getActivities() {return atm -> getActivities();}

        /// \brief Gets Action set in atm.
        /// \return A set of Action.
        Actions& getActions() {return atm -> getActions();} 

        /// \brief Gets Aft set in atm.
        /// \return A set of Aft.
        Afts& getAfts() {return atm -> getAfts();} 

        /// \brief Gets realActMap in atm.
        /// \return A map from Aft to Acts.
        Aft2ActsMap& getRealActMap() {return atm -> getRealActMap();}

        /// \brief Gets loopMap in atm.
        /// \return A map from Act to Acts.
        Act2ActsMap& getLoopMap() {return atm -> getLoopMap();}

        /// \brief Gets lengthMap in atm.
        /// \return A map from Aft to ID.
        Aft2IDMap& getLengthMap() {return atm -> getLengthMap();}

        /// \brief Gets main Activity in atm.
        /// \return A Activity pointer..
        Activity* getMainActivity() {return atm -> getMainActivity();}

        /// \brief Gets main Action in atm.
        /// \return A Action pointer..
        Action* getMainAction() {return atm -> getMainAction();}

        /// \brief Gets main Affinity in atm.
        /// \return A Affinity.
        Aft getMainAft() {return getMainActivity() -> getAft();}

        /// \brief Gets StateVar from param tId and param pId.
        /// \param tId The index of the task.
        /// \param pId The position of the Activity.
        /// \return StateVar pointer.
        StateVar* getStateVar(ID tId, ID pId) {
            ID sId = 0;
            for (ID i = 0; i < tId; i++) {
                sId += lengthMap[i] + 1;
            }
            return stateVars[sId + pId];
        }

        /// \brief Gets StateVar from param id.
        /// \param id The index of the task.
        /// \return StateVar pointer.
        StateVar* getStateVar(ID id) {
            ID sId = 0;
            for (ID i = 0; i < id; i++) {
                sId += lengthMap[i] + 1;
            }
            return stateVars[sId + 1];
            //return stateVars[id * (getTaskLength() + 1)];
        }

        /// \brief Gets Order StateVar.
        /// \return StateVar pointer.
        StateVar* getStateVar() {
            return stateVars[stateVarNum - 1];
        }

        /// \brief Gets order StateVar for this ATMSolver.
        /// \return StateVar pointer.
        void mkOrderAtomic(Condition* condition, ID id) {
            ID pos = stateVarNum - 1;
            for (auto& mapPair : order2ValueMap) {
                if (mapPair.first[id] != 1) {
                    condition -> mkStateAtomic(stateVars[pos], mapPair.second, 0);
                }
            }
        }

        /// \brief Initialize this ATMSolver.
        void init();
        void initActVars();
        void initTaskVars();
        void initOrderVars();
        void initCharVars();

        /// \brief Produce the condition for start mainActivity in atm for this Solver.
        void startMain();
        void startActions();
        void pop();

        Condition* mkCondition() {
            Condition* condition = new Condition();
            conditions.push_back(condition);
            return condition;
        }
        Condition* mkCondition(Activity* realAct, Action* action, ID i, ID j, bool order = true);
        Condition* mkCondition(Condition* condition);
        void Push(Condition* condition, Action* action, ID i, ID j, bool fin = false);
        void ClrTop(Condition* condition, Activity* realAct, Action* action, ID i, ID j, ID m);
        void ClrTop(Condition* condition, Activity* realAct, Action* action, ID i, ID j);
        void ClrTop(Activity* realAct, Action* action, ID i, ID j);
        void Finish(Condition* condition, Action* action, ID i, ID j);
        void getNewOrder(Order& order, Order& newOrder, ID id);
        void getNewPopOrder(Order& order, Order& newOrder, ID id);
        void SwitchTask(Condition* condition, Action* action, Order& order);
        void New(Condition* condition, Action* action, ID i, ID j, bool push);

        void mkOrderTrans();
        void startSTD(Action* action);
        void startCTP(Action* action);
        void startNewSTD(Action* action);
        void startNewCTP(Action* action);

        void mkLoopConfiguration(Activity* real, Acts& loopActs);
        void mkBackPattenConfiguration(Activity* act, Activity* bAct);

        void pre4BackPatten(VerificationDatas& datas, Activity* act);
        void pre4GetLoop(VerificationDatas& datas);

    }; 

}
//class Var;
//class StateVar;
//class CharVar;
//class FlagVar;
//class AftVar;
//class Condition;
//
//typedef pair<StateVar*, Activity*> StateCondition;
//typedef pair<CharVar*, Transition*> CharCondition;
//typedef pair<AftVar*, Aft> AftCondition;
//typedef vector<StateCondition> StateConditions;
//typedef vector<AftCondition> AftConditions;
//typedef vector<Condition*> Conditions;
//typedef vector<StateVar*> StateVars;
//typedef vector<AftVar*> AftVars;
//typedef vector<StateVars> StateVarsVec;
//
//class ASMSolver; 
//
//struct VerificationData
//{
//    Activity* act;
//    Aft aft;
//    ID length;
//    string smv;
//    int* value;
//    string* path;
//    ASMSolver* solver;
//};
//typedef vector<VerificationData> VerificationDatas;
//
//class Var
//{
//protected:
//    ID id;
//public:
//    Var() {}
//    Var(ID i) { id = i;}
//    ID getID() {return id;}
//};
//
//class StateVar : public Var
//{
//private:
//    Conditions conditions;
//public:
//    StateVar() {}
//    StateVar(ID size) : Var(size) {}
//    ~StateVar()
//    {
//        for(Condition* condition : conditions)
//            delete condition;
//    }
//    Conditions& getConditions() {return conditions;}
//    void addCondition(Condition* condition) {conditions.push_back(condition);}
//    friend Condition;
//};
//
//
//class CharVar : public Var
//{
//public:
//    CharVar() {}
//    CharVar(ID size) : Var(size) {}
//};
//
//class AftVar : public Var
//{
//private:
//    Conditions conditions;
//public:
//    AftVar() {}
//    AftVar(ID size) : Var(size) {}
//    Conditions& getConditions() {return conditions;}
//    void addCondition(Condition* condition) {conditions.push_back(condition);}
//    ~AftVar()
//    {
//        for(Condition* condition : conditions)
//            delete condition;
//    }
//};
//
//
//class Solver
//{
//    
//};
//
//class ASMSolver : public Solver
//{
//private:
//    StateVarsVec stateVarsVec;
//    AftVars aftVars;
//    CharVar* charVar;
//    ID stackLength;
//    ID stackNum;
//    vector<Aft> afts;
//    Activities activities;
//    Activities mainActivities;
//    Activity* nullAct;
//    Activity* mainActivity;
//    Transition* nullTrans;
//    Transitions transitions;
//    Transitions mainTransitions;
//    Aft nullAft;
//    Aft mainAft;
//    Aft targetAft;
//    Conditions conditions;
//    string smv;
//    unordered_map<ID, Activity*> id2ActMap;
//    unordered_map<ID, Transition*> id2TransMap;
//private:
//bool isSTD(Transition* transition);
//bool isCTP(Transition* transition);
//bool isRTF(Transition* transition);
//bool isNTK(Transition* transition);
//bool isSTK(Transition* transition);
//bool isRTF_NTK(Transition* transition);
//bool isCTP_NTK(Transition* transition);
//bool isCTK_NTK(Transition* transition);
//public:
//    ASMSolver();
//    ASMSolver(ID actNum, ID aftNum);
//    ~ASMSolver();
//    ID getStackNum() {return stackNum;}
//    string& getSmv() {return smv;}
//    Activities& getActivities() {return activities;}
//    Activity* getMainActivity() {return mainActivity;}
//    unordered_map<ID, Activity*>& getID2ActMap() {return id2ActMap;}
//    unordered_map<ID, Transition*>& getID2TransMap() {return id2TransMap;}
//    void init(ID actNum, ID aftNum, ID length);
//    void loadVars(ASM* a);
//    void setMaxLength(ID length);
//    void startMain(bool flag);
//    void startStd(Activity* act, Transition* transition, FLAG flag);
//    void newTskNoIst(Activity* act, Transition* transition, FLAG flag);
//    void startStk(Activity* act, Transition* transition);
//    void startStdWithCTP(Activity* act, Transition* transition);
//    void startStdWithNTK(Activity* act, Transition* transition);
//    void startActWithCTK_NTK(Activity* act, Transition* transition);
//    void startStdWithCTP_NTK(Activity* act, Transition* transition);
//    void startStdWithRTF_NTK(Activity* act, Transition* transition);
//    
//    void startStdWithRTF(Activity* act, Transition* transition);
//    void pop();
//    void mv2Top(ID topId, ID sourceId, ID ActId, Condition* condition, Activity* targetAct);
//    void mv2Top(ID topId, ID sourseId, Condition* condition, bool flag, ID ActId);
//    void cp(ID topId, ID sourseId, Condition* condition);
//    void cp4NewAct(ID topId, ID sourseId, ID ActID, Condition* condition, Activity* targetAct);
//    void cp4ClrTop(ID topId, ID sourseId, ID ActID, Condition* condition);
//    void cp4ClrTsk(ID topId, ID sourseId, Condition* condition, Activity* targetAct);
//    void cp4Rd2Frt(ID topId, ID sourseId, ID ActID, Condition* condition);
//    void mv2TopAndClTop(ID topId, ID sourceId, ID ActId, Condition* condition, Activity* targetAct);
//    void mv2TopAndClTask(ID topId, ID sourceId, Condition* condition, Activity* targetAct);
//    
//    string getVarStrSimple();
//    string getInitStrSimple();
//    string getCodeStrSimple();
//    string getInvStrSimple();
//    string getInvStr4BackPatten();
//    string getInvStr4BackPatten(Activity* cur, Activity* back);
//    string getInvStr4BackHijacking(Activity* act);
//    string getInvStr4TskReparentingHijacking(Acts& actSet, Aft targetAft);
//    string getInvStr4MaxLength(ID length);
//    int getNuxmvRes(string commond);
//    int getNuxmvResFromTerminal(string commond);
//    void preDetection4BackHijacking(VerificationDatas& datas, Aft targetAft);
//    void preDetection4TskReparentingHijacking(VerificationDatas& datas, Aft targetAft);
//    void pre4BackPatten(VerificationDatas& datas, Activity* targetAct);
//    void pre4MaxLength(VerificationDatas& datas);
//    void multiDetection();
//    
//};

#endif /* ATMSolver_hpp */
