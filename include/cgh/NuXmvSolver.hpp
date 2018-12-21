//
//  NuXmvSolver.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/6.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef NuXmvSolver_hpp
#define NuXmvSolver_hpp
#include <cstddef>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
using namespace std;
class Value;
class StateAtomic;
class CharAtomic;
class StateVar;
class CharVar;
class Condition;
class Transition;
class NuXmvSolver;

typedef size_t ID;
typedef vector<Value*> Values;
typedef vector<StateAtomic*> StateAtomics;
typedef vector<CharAtomic*> CharAtomics;
typedef vector<Transition*> Transitions;
typedef vector<StateVar*> StateVars;
typedef vector<CharVar*> CharVars;
typedef vector<Transition*> Transitions;
typedef vector<StateAtomics> Configuration;
typedef vector<Condition*> Conditions;

/// /biref Transition in NuXmv.
class Transition {
private:
    Condition* condition;       ///< the condition for this Transition.
    Value* value;               ///< the target Value for this Transition.
    StateVar* stateVar;         ///< the target Var for this Transition.
public:
    /// \brief Default construction function.
    Transition() : condition(nullptr), value(nullptr), stateVar(nullptr) {}

    /// \brief Construction function with params.
    /// \param c Condition for this Transition.
    /// \param v Value for this Transition.
    Transition(Condition* c, Value* v) : condition(c), value(v), stateVar(nullptr) {}

    /// \brief Desconstruction function.
    ~Transition() {
    }

    /// \brief Construction function with params.
    /// \param c Condition for this Transition.
    /// \param sVar state Var for this Transition.
    Transition(Condition* c, StateVar* sVar) : condition(c), value(nullptr), stateVar(sVar) {}

    /// \brief Gets the condition for this Transition.
    /// \return Condition pointer.
    Condition* getCondition() {return condition;}
    Condition* getCondition() const {return condition;}

    /// \brief Gets the stateVar for this Transition.
    /// \return StateVar pointer.
    StateVar* getStateVar() {return stateVar;}
    StateVar* getStateVar() const {return stateVar;}

    /// \brief Gets the value for this Transition.
    /// \return Value pointer.
    Value* getValue() {return value;}
    Value* getValue() const {return value;}
};

/// \brief Values in NuXmv.
class Value {
private:
    ID id;      ///< the identity for this Value.
public:
    /// \brief Default construction function.
    Value() : id(0) {}

    /// \brief Constrction with param.
    /// \param i Id for this Value.
    Value(ID i) : id(i) {}

    /// \brief Gets Id for this Value.
    /// \return ID.
    ID getID() {return id;}
    ID getID() const {return id;}
};

/// \brief Variables in NuXmv.
class Var {
private:
    ID id;                  ///< the identity for this Var.
    Values values;          ///< the available values for this StateVar.
public:
    /// \brief Default construction function.
    Var() : id(0) {}

    /// \brief Constrction with param.
    /// \param i Id for this Var.
    /// \param vs available values for this StateVar.
    Var(ID i, const Values& vs) : id(i), values(vs.begin(), vs.end()) {}

    /// \brief Gets Id for this Var.
    /// \return ID.
    ID getID() {return id;}
    ID getID() const {return id;}

    /// \brief Gets the values for this StateVar.
    /// \return reference of Value pointer vector.
    Values& getValues() {return values;}
    const Values& getValues() const {return values;}

};

/// \brief State Variables in NuXmv.
class StateVar : public Var {
private:
    Value* initValue;           ///< the initial value for this StateVar.
    Value* trapValue;           ///< the trap Value for this StateVar.
    Transitions transitions;    ///< the transitions for this StateVar.
public:
    /// \brief Default construction function.
    StateVar() : Var(), initValue(nullptr), trapValue(nullptr) {}

    /// \brief Constrction with param.
    /// \param i Id for this StateVar.
    /// \param vs available values for this StateVar.
    /// \param iValue initial value for this StateVar.
    /// \param tValue trap value for this StateVar.
    StateVar(ID i, const Values& vs, Value* iValue, Value* tValue = nullptr) : Var(i, vs), initValue(iValue), trapValue(tValue) {}

    /// \brief Desconstruction function.
    ~StateVar() {
        for (Transition* transition : transitions) {
            delete transition;
        }
    }

    /// \brief Gets the initial Value for this StateVar.
    /// \return Value pointer.
    Value* getInitialValue() {return initValue;}
    Value* getInitialValue() const {return initValue;}

    /// \brief Gets the trap Value for this StateVar.
    /// \return Value pointer.
    Value* getTrapValue() {return trapValue;}
    Value* getTrapValue() const {return trapValue;}

    /// \brief Gets the transitions for this StateVar.
    /// \return reference of Transition pointer vector.
    Transitions& getTransitions() {return transitions;}
    const Transitions& getTransitions() const {return transitions;}

    /// \brief Adds a transition in the transitions for this StateVar.
    /// \param condition Condition in the transition.
    /// \param value Value for this Transition.
    /// \return Transition pointer
    Transition* mkTransition(Condition* condition, Value* value) {
        Transition* transition = new Transition(condition, value);
        transitions.push_back(transition);
        return transition;
    }

    /// \brief Adds a transition in the transitions for this StateVar.
    /// \param condition Condition in the transition.
    /// \param stateVar StateVar for this Transition.
    /// \return Transition pointer
    Transition* mkTransition(Condition* condition, StateVar* stateVar) {
        Transition* transition = new Transition(condition, stateVar);
        transitions.push_back(transition);
        return transition;
    }

};

/// \brief State Variables in NuXmv.
class CharVar : public Var {
public:
    /// \brief Default construction function.
    CharVar() : Var() {}

    /// \brief Constrction with param.
    /// \param i Id for this CharVar.
    /// \param vs available values for this StateVar.
    CharVar(ID i, const Values& vs) : Var(i, vs) {}
};

/// \brief The Atomic in the NuXmv
///
/// Example: s1 = c1.
class Atomic {
private:
    Value* value;       ///< the Value for this Atomic.
    bool flag;          ///< the flag representing positive or not for this Atomic.
public:
    /// \brief Default construction function.
    Atomic() : value(nullptr), flag(true) {}

    /// \brief construcion with params.
    /// \param v Value for this Atomic.
    /// \param f flag for this Atomic.
    Atomic(Value* v, bool f = true) : value(v), flag(f) {}

    /// \brief Gets the Value for this Atomic.
    /// \return Value pointer.
    Value* getValue() {return value;}
    Value* getValue() const {return value;}

    /// \brief Gets the flag for this Atomic.
    /// \return Boolean.
    bool isPositive() {return flag;}
    bool isPositive() const {return flag;}
};

class StateAtomic : public Atomic {
private:
    StateVar* stateVar;     ///< the State Var for this StateAtomic.
public:
    /// \brief Default construction function.
    StateAtomic() : Atomic() {}

    /// \brief construcion with params.
    /// \param sVar State Var for this StateAtomic
    /// \param v Value for this StateAtomic.
    /// \param f flag for this StateAtomic.
    StateAtomic(StateVar* sVar, Value* v, bool f = true) : Atomic(v, f), stateVar(sVar) {}

    /// \brief Gets the stateVar for this StateAtomic.
    /// \return StateVar pointer.
    StateVar* getStateVar() {return stateVar;}
    StateVar* getStateVar() const {return stateVar;}
};

class CharAtomic : public Atomic {
private:
    CharVar* charVar;     ///< the Char Var for this CharAtomic.
public:
    /// \brief Default construction function.
    CharAtomic() : Atomic() {}

    /// \brief construcion with params.
    /// \param cVar Char Var for this CharAtomic
    /// \param v Value for this CharAtomic.
    /// \param f flag for this CharAtomic.
    CharAtomic(CharVar* cVar, Value* v, bool f = true) : Atomic(v, f), charVar(cVar) {}

    /// \brief Gets the charVar for this CharAtomic.
    /// \return CharVar pointer.
    CharVar* getCharVar() {return charVar;}
    CharVar* getCharVar() const {return charVar;}
};

/// \brief Condition in NuXmv.
class Condition {
private:
    StateAtomics stateAtomics;      ///< the state Atomics for this Condition.
    CharAtomics charAtomics;        ///< the char Atomics for this Condition.

public:
    /// \brief Defualt construction function.
    Condition() {}

    /// \brief Adds a State Atomic for this Condition.
    /// \param stateAtomic Be added in stateAtomics.
    void mkStateAtomic(StateVar* stateVar, Value* value, bool flag = true) {
        StateAtomic* stateAtomic = new StateAtomic(stateVar, value, flag);
        stateAtomics.push_back(stateAtomic);
    }

    /// \brief Adds a Char Atomic for this Condition.
    /// \param charAtomic Be added in charAtomics.
    void mkCharAtomic(CharVar* charVar, Value* value, bool flag = true) {
        CharAtomic* charAtomic = new CharAtomic(charVar, value, flag);
        charAtomics.push_back(charAtomic);
    }

    StateAtomics& getStateAtomics() {return stateAtomics;}
    CharAtomics& getCharAtomics() {return charAtomics;}

};



class NuXmvSolver {
protected:
    StateVars stateVars;            ///< the stateVar vector for this NuXmvSolver.
    CharVars charVars;              ///< the charVar vector for this NuXmvSolver.
    Values values;                  ///< the whole values for this NuXmvSolver.
    Configuration configuration;    ///< the vefication configuration for this NuXmvSolver.
    Conditions conditions;          ///< the all conditions in thie NuXmvSolver.

    string StateVarItem(const StateVar* stateVar) {
        return "s" + to_string(stateVar -> getID());
    }
    
    string CharVarItem(const CharVar* charVar) {
        return "c" + to_string(charVar -> getID());
    }

    string ValueItem(const Value* value) {
        return "v" + to_string(value -> getID());
    }

    string StateAtomicItem(const StateAtomic* stateAtomic) {
        string stateVarItem = StateVarItem(stateAtomic -> getStateVar());
        string valueItem = ValueItem(stateAtomic -> getValue());
        if (stateAtomic -> isPositive()) {
            return stateVarItem + " = " + valueItem;
        } else {
            return "!(" + stateVarItem + " = " + valueItem + ")";
        }
    }

    string CharAtomicItem(const CharAtomic* charAtomic) {
        string charVarItem = CharVarItem(charAtomic -> getCharVar());
        string valueItem = ValueItem(charAtomic -> getValue());
        if (charAtomic -> isPositive()) {
            return charVarItem + " = " + valueItem;
        } else {
            return "!(" + charVarItem + " = " + valueItem + ")";
        }
    }

    string And(const string& lhs, const string& rhs) {
        if (lhs.length() == 0) return rhs;
        if (rhs.length() == 0) return lhs;
        return lhs + " & " + rhs;
    }

    string Or(const string& lhs, const string& rhs) {
        if (lhs.length() == 0) return rhs;
        if (rhs.length() == 0) return lhs;
        return "(" + lhs + " | " + rhs + ")";
    }

    string Not(const string& str) {
        return "(!" + str + ")";
    }

    string Init(const StateVar* stateVar) {
        return "init(" + StateVarItem(stateVar) + ")";
    }

    string Next(const StateVar* stateVar) {
        return "next(" + StateVarItem(stateVar) + ")";
    }

    /// \brief Gets string of this Condition.
    /// \return string.
    string getStr(Condition* condition) {
        string res = "";
        for (StateAtomic* stateAtomic : condition -> getStateAtomics()) {
            string stateAtomicItem = StateAtomicItem(stateAtomic);
            res = And(res, stateAtomicItem);
        }
        for (CharAtomic* charAtomic : condition -> getCharAtomics()) {
            string charAtomicItem = CharAtomicItem(charAtomic);
            res = And(res, charAtomicItem);
        }
        return res;
    }
public:
    /// \brief Default construction function.
    NuXmvSolver() {}

    /// \brief Desconstruction function.
    ~NuXmvSolver() {
        for (StateVar* stateVar : stateVars) {
            delete stateVar;
        }
        for (CharVar* charVar : charVars) {
            delete charVar;
        }
        for (Value* value : values) {
            delete value;
        }
        for (Condition* condition : conditions) {
            delete condition;
        }
        for (StateAtomics& stateAtomics : configuration) {
            for (StateAtomic* stateAtomic : stateAtomics) {
                delete stateAtomic;
            }
        }
    }

    /// \brief Makes a StateVar for this NuXmvSolver.
    /// \param id Id for this StateVar.
    /// \param values available values for this StateVar.
    /// \param initValue initial value for this StateVar.
    /// \param trapValue trap value for this StateVar.
    /// \return StateVar pointer.
    StateVar* mkStateVar(ID id, const Values& values, Value* initValue, Value* trapValue = nullptr) {
        StateVar* stateVar = new StateVar(id, values, initValue, trapValue);
        stateVars.push_back(stateVar); 
        return stateVar;
    }

    /// \brief Makes a CharVar for this NuXmvSolver.
    /// \param id Id for this CharVar.
    /// \param values available values for this CharVar.
    /// \return CharVar pointer.
    CharVar* mkCharVar(ID id, const Values& values) {
        CharVar* charVar = new CharVar(id, values);
        charVars.push_back(charVar);
        return charVar;
    }

    /// \brief Makes a Value for this NuXmvSolver.
    /// \param id Id for this Value.
    /// \return Value pointer.
    Value* mkValue(ID id) {
        Value* value = new Value(id);
        values.push_back(value);
        return value;
    }

    void mkConfiguration(const Configuration& config) {
        configuration.insert(configuration.end(), config.begin(), config.end());
    }

    string getVAR() {
        //cout << "VAR" << endl;
        string res = "MODULE main\nVAR\n";
        for (StateVar* stateVar : stateVars) {
            //cout << stateVar -> getID() << endl;
            string valueItems = "{";
            for (Value* value : stateVar -> getValues()) {
                valueItems += ValueItem(value) + ", ";
            }
            valueItems = valueItems.substr(0, valueItems.length() - 2);
            valueItems += "};\n";
            res += StateVarItem(stateVar) + " : " + valueItems;
        }
        for (CharVar* charVar : charVars) {
            string valueItems = "{";
            for (Value* value : charVar -> getValues()) {
                valueItems += ValueItem(value) + ", ";
            }
            valueItems = valueItems.substr(0, valueItems.length() - 2);
            valueItems += "};\n";
            res += CharVarItem(charVar) + " : " + valueItems;
        }
        return res;
    }

    string getASSIGN_INIT() {
        //cout << "INIT" << endl;
        string res = "ASSIGN\n";
        for (StateVar* stateVar : stateVars) {
            res += Init(stateVar) + " := " + ValueItem(stateVar -> getInitialValue()) + ";\n"; 
        }
        return res;
    }

    string getASSIGN_NEXT() {
        //cout << "NEXT" << endl;
        string res = "";
        for (StateVar* stateVar : stateVars) {
            string transitionStr = Next(stateVar) + " := case\n";
            Value* trapValue = stateVar -> getTrapValue();
            for (Transition* transition : stateVar -> getTransitions()) {
                string conditionStr = getStr(transition -> getCondition());
                StateVar* targetStateVar = transition -> getStateVar();
                Value* targetValue = transition -> getValue();
                if (targetStateVar) {
                    transitionStr += conditionStr + " : " + StateVarItem(targetStateVar) + ";\n";
                } else {
                    transitionStr += conditionStr + " : " + ValueItem(targetValue) + ";\n";
                }
            }
            if (trapValue) {
                transitionStr += "TRUE : " + ValueItem(trapValue) + ";\n";
            } else {
                transitionStr += "TRUE : " + StateVarItem(stateVar) + ";\n";
            }
            transitionStr += "esac;\n";
            res += transitionStr;
        }
        return res;
    }

    string getINVARSPEC() {
        //cout << "INV" << endl;
        string res = "INVARSPEC\n!(";
        string configurationStr = "";
        for (StateAtomics& stateAtomics: configuration) {
            string stateVarConditionStr = "";
            for (StateAtomic* stateAtomic : stateAtomics) {
                stateVarConditionStr = And(stateVarConditionStr, StateAtomicItem(stateAtomic));
            }
            configurationStr = Or(configurationStr, stateVarConditionStr);
        }
        res += configurationStr + ");";
        return res;
    }

    string getPreSMV() {
        return getVAR() + getASSIGN_INIT() + getASSIGN_NEXT();
    }

    string getSMV() {
        return getVAR() + getASSIGN_INIT() + getASSIGN_NEXT() + getINVARSPEC();
    }
    

};


#endif /* NuXmvSolver_hpp */
