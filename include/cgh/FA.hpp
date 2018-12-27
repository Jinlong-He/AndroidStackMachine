//
//  FA.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/6/14.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef FA_hpp
#define FA_hpp

#include <fstream>
#include <iostream>

#include "State.hpp"

using namespace std;
namespace cgh{
    template <class Character> class DFA;
    template <class Character> class NFA;
    template <class Character> class SmartDFA;
    template <class Character> class DFAState;
    template <class Character> class NFAState;
    
    /// \brief FA is a virtual class which define a Finite Automaton.
    template <class Character>
    class FA
    {

        
        typedef typename Global<Character>::Word Word;
        typedef typename Global<Character>::FASet FASet;
        typedef typename Global<Character>::DFASet DFASet;
        typedef typename Global<Character>::FAList FAList;
        typedef typename Global<Character>::DFAState2 DFAState2;
        typedef typename Global<Character>::DFAStateSet DFAStateSet;
        typedef typename Global<Character>::DFATransMap DFATransMap;
        typedef typename Global<Character>::NFAStateSet NFAStateSet;
        typedef typename Global<Character>::NFATransMap NFATransMap;
        typedef typename Global<Character>::DFAState2Map DFAState2Map;
        typedef typename Global<Character>::NFAState2Map NFAState2Map;
        typedef typename Global<Character>::CharacterSet CharacterSet;
        typedef typename Global<Character>::DFAStateSetMap DFAStateSetMap;
        typedef typename Global<Character>::DFAStatePairMap DFAStatePairMap;
        typedef typename Global<Character>::Char2DFAStateSetMap Char2DFAStateSetMap;
        typedef typename Global<Character>::DFAState2NFAStateMap DFAState2NFAStateMap;
        
    protected:
        Flag flag;              /// < Records some attributes for this FA.
        CharacterSet alphabet;  /// < A set of characters which in the label on the transitions.

        /// \brief Default construction without arguments, initialize flag to 0.
        FA() : flag(0){}

        /// \brief Construct FA with alphabet.
        /// \param charSet The alphabet.
        FA(const CharacterSet& charSet) : flag(0), alphabet(charSet.begin(), charSet.end()) {}

        /// \brief Construct FA from file.
        /// \param file The FA description file.
        FA(FILE *file){}//todo

        /// \brief Sets this FA to deterministic or not by param b.
        /// \param b If b is true means deterministic otherwise not.
        void setDeterministicFlag(bool b){flag = b ? (flag | 1):(flag & ~1);}

        /// \brief Sets this FA to reachable or not by param b.
        /// \param b If b is true means reachable otherwise not.
        void setReachableFlag(bool b){flag = b ? (flag | (1 << 1)):(flag & ~(1 << 1));}

        /// \brief Sets this FA to minimal or not by param b.
        /// \param b If b is true means minimal otherwise not.
        void setMinimalFlag(bool b){flag = b ? (flag | (1 << 2)):(flag & ~(1 << 2));}

        /// \brief Gets a DFA which determinized by FA.
        /// \return A reference of DFA.
        virtual const DFA<Character>& determinize( void ) const = 0;

        /// \brief Gets a NFA which nondeterminized by FA.
        /// \return A reference of NFA.
        virtual const NFA<Character>& nondeterminize( void ) const = 0;

        virtual FA& copy() = 0;
    private:
        static void cpNFATransByDFA(NFA<Character>* nfa, DFAState<Character>* state, DFAState2NFAStateMap& state2map) {
            NFAState<Character>* sourceState = state2map[state];
            if (state -> isFinal()) nfa -> addFinalState(sourceState);
            for (auto& mapPair : state -> getDFATransMap()) {
                NFAState<Character>* targetState = nullptr;
                auto state2MapIt = state2map.find(mapPair.second);
                if (state2MapIt == state2map.end()) {
                    targetState = nfa -> mkState();
                    state2map[mapPair.second] = targetState;
                    cpNFATransByDFA(nfa, mapPair.second, state2map);
                } else {
                    targetState = state2MapIt -> second;
                }
                sourceState -> addNFATrans(mapPair.first, targetState);
            }
        }
        static void cpNFATransByNFA(NFA<Character>* nfa, NFAState<Character> *state, NFAState2Map &state2map) {
            NFAState<Character>* sourceState = state2map[state];
            if (state -> isFinal()) nfa -> addFinalState(sourceState);
            for (auto& mapPair : state -> getNFATransMap()) {
                for (NFAState<Character>* state : mapPair.second){
                    NFAState<Character>* targetState = nullptr;
                    auto state2MapIt = state2map.find(state);
                    if (state2MapIt == state2map.end()) {
                        targetState = nfa -> mkState();
                        state2map[state] = targetState;
                        cpNFATransByNFA(nfa, state, state2map);
                    } else {
                        targetState = state2MapIt -> second;
                    }
                    sourceState -> addNFATrans(mapPair.first, targetState);
                }
            }
        }
        static void cpDFATransByDFA(DFA<Character>* dfa, DFAState<Character>* state, DFAState2Map& state2map)
        {
            DFAState<Character>* sourceState = state2map[state];
            if (state -> isFinal()) dfa -> addFinalState(sourceState);
            for (auto& mapPair : state -> getDFATransMap()) {
                DFAState<Character>* targetState = nullptr;
                auto state2MapIt = state2map.find(mapPair.second);
                if (state2MapIt == state2map.end()) {
                    targetState = dfa -> mkState();
                    state2map[mapPair.second] = targetState;
                    cpDFATransByDFA(dfa, mapPair.second, state2map);
                } else {
                    targetState = state2MapIt -> second;
                }
                sourceState -> addDFATrans(mapPair.first, targetState);
            }
        }

        static void getTransMapByStateSet(const DFAStateSet& stateSet, Char2DFAStateSetMap& nfaTransMap) {
            DFATransMap& transMap = (*stateSet.begin()) -> getDFATransMap();
            DFAStateSet workSet;
            for (auto& mapPair : transMap) {
                workSet.clear();
                for (DFAState<Character>* dfaState : stateSet) {
                    DFAState<Character>* state = dfaState -> getTargetStateByChar(mapPair.first);
                    if (!state) break;
                    workSet.insert(state);
                }
                if (workSet.size() == stateSet.size())
                    nfaTransMap[mapPair.first] = workSet;
            }
        }
        
        static void makeDFATrans(DFAState<Character>* preState, DFAStateSetMap &setMapping, const Char2DFAStateSetMap &nfaTransMap, DFA<Character>* dfa) {
            Char2DFAStateSetMap transMap;
            for (auto& mapPair : nfaTransMap) {
                auto setMapIter = setMapping.find(mapPair.second);
                DFAState<Character>* postState;
                if (setMapIter == setMapping.end()) {
                    transMap.clear();
                    FA::getTransMapByStateSet(mapPair.second, transMap);
                    if(DFA<Character>::allFinalState(mapPair.second)) postState = dfa -> mkFinalState();
                    else postState = dfa -> mkState();
                    setMapping[mapPair.second] = postState;
                    makeDFATrans(postState, setMapping, transMap, dfa);
                } else {
                    postState = (setMapping[mapPair.second]);
                }
                preState -> addDFATrans(mapPair.first, postState);
            }
        }

        static void intersectFA(DFA<Character>* dfa, DFAState<Character>* sourceState, const DFAStateSet& stateSet, DFAStateSetMap &setMap) {
            if(DFA<Character>::allFinalState(stateSet)) 
                dfa -> addFinalState(sourceState);
            setMap[stateSet] = sourceState;
            DFATransMap& transMap = (*(stateSet.begin())) -> getDFATransMap();
            DFAStateSet newStateSet;
            for (auto& mapPair : transMap) {
                newStateSet.clear();
                Character character = mapPair.first;
                newStateSet.insert(mapPair.second);
                for (DFAState<Character>* state : stateSet) {
                    if (state == *(stateSet.begin())) continue;
                    DFATransMap& otherTransMap = state -> getDFATransMap();
                    auto mapIt = otherTransMap.find(character);
                    if (mapIt != otherTransMap.end()) {
                        newStateSet.insert(mapIt -> second);
                    }
                }
                if (newStateSet.size() == stateSet.size()) {
                    DFAState<Character>* targetState = nullptr;
                    auto setMapIt = setMap.find(newStateSet);
                    if (setMapIt == setMap.end()) {
                        targetState = dfa -> mkState();
                        intersectFA(dfa, targetState, newStateSet, setMap);
                    } else {
                        targetState = setMapIt -> second;
                    }
                    sourceState -> addDFATrans(character, targetState);
                }
            }
        }


        static void intersectFA(DFA<Character>* dfa, DFAState<Character>* sourceState, const DFAState2& statePair, DFAStatePairMap& dfaStatePairMap) {
            if (statePair.first -> isFinal() && statePair.second -> isFinal())
                dfa -> addFinalState(sourceState);
            dfaStatePairMap[statePair] = sourceState;
            DFATransMap& lhsTransMap = statePair.first -> getDFATransMap();
            DFATransMap& rhsTransMap = statePair.second -> getDFATransMap();
            for (auto& lhsPair : lhsTransMap) {
                Character character = lhsPair.first;
                auto rhsIt = rhsTransMap.find(character);
                if (rhsIt != rhsTransMap.end()) {
                    auto& rhsPair = *rhsIt;
                    DFAState<Character>* targetState = nullptr;
                    DFAState2 newStatePair(lhsPair.second, rhsPair.second);
                    auto pairMapIt = dfaStatePairMap.find(newStatePair);
                    if (pairMapIt == dfaStatePairMap.end()) {
                        targetState = dfa -> mkState();
                        intersectFA(dfa, targetState, newStatePair, dfaStatePairMap);
                    } else {
                        targetState = pairMapIt -> second;
                    }
                    sourceState -> addDFATrans(character, targetState);
                }
            }
        }

        static void unionFA(NFA<Character>* nfa, const DFAState2& statePair) {
            NFAState<Character>* initialState = nfa -> getInitialState();
            DFAState2NFAStateMap lhsState2Map;
            DFAState2NFAStateMap rhsState2Map;
            DFAState<Character>* lhsDFAState = statePair.first;
            DFAState<Character>* rhsDFAState = statePair.second;
            NFAState<Character>* lhsNFAState = nfa -> mkState();
            NFAState<Character>* rhsNFAState = nfa -> mkState();
            lhsState2Map[lhsDFAState] = lhsNFAState;
            rhsState2Map[rhsDFAState] = rhsNFAState;
            initialState -> addEpsilonTrans(lhsNFAState); 
            initialState -> addEpsilonTrans(rhsNFAState); 
            cpNFATransByDFA(nfa, lhsDFAState, lhsState2Map);
            cpNFATransByDFA(nfa, rhsDFAState, rhsState2Map);
        }

        static void concatenateFA(NFA<Character>* nfa, const DFAState2& statePair) {
            NFAState<Character>* initialState = nfa -> getInitialState();
            DFAState2NFAStateMap lhsState2Map;
            DFAState2NFAStateMap rhsState2Map;
            DFAState<Character>* lhsDFAState = statePair.first;
            DFAState<Character>* rhsDFAState = statePair.second;
            NFAState<Character>* lhsNFAState = nfa -> mkState();
            NFAState<Character>* rhsNFAState = nfa -> mkState();
            lhsState2Map[lhsDFAState] = lhsNFAState;
            rhsState2Map[rhsDFAState] = rhsNFAState;
            initialState -> addEpsilonTrans(lhsNFAState); 
            cpNFATransByDFA(nfa, lhsDFAState, lhsState2Map);
            for (NFAState<Character>* finalState : nfa -> getFinalStateSet()) {
                finalState -> addEpsilonTrans(rhsNFAState);
            }
            nfa -> clearFinalStateSet();
            cpNFATransByDFA(nfa, rhsDFAState, rhsState2Map);
        }

        static void complementFA(DFA<Character>* dfa, DFAState<Character>* state) {
            DFAState<Character>* initialState = dfa -> getInitialState();
            DFAState2Map stateMap;
            stateMap[state] = initialState;
            cpDFATransByDFA(dfa, state, stateMap);
            DFAState<Character>* trapState = dfa -> mkState();
            dfa -> getFinalStateSet().clear();
            for (DFAState<Character>* state : dfa -> getStateSet()) {
                for (Character character : dfa -> getAlphabet()) {
                    if (state -> getDFATransMap().count(character) == 0) {
                        state -> addDFATrans(character, trapState);
                    }
                }
                if (state -> isFinal()) {
                    state -> setFinalFlag(0);
                } else {
                    state -> setFinalFlag(1);
                    dfa -> getFinalStateSet().insert(state);
                }
            }
        }

        SmartDFA<Character> minimize() const {
            if (isMinimal()) return SmartDFA<Character>(&determinize(), 0);
            return SmartDFA<Character>(&const_cast<DFA<Character>&>(determinize()).minimize(), 1);
        }


    public:
        /// \brief Virtual desconstruction fuction.
        virtual ~FA(){}

        /// \brief Judges whether this FA is deterministic or not.
        ///
        /// ture means deterministic, false means nondeterministic.
        /// \return A boolean.
        bool isDeterministic() const {return (flag & 1) == 1;}

        /// \brief Judges whether this FA is reachable or not.
        ///
        /// ture means reachable, false means unreachable.
        /// reachable means all the states in this FA can be reached from initial state.
        /// \return A boolean.
        bool isReachable() const {return (flag & 1 << 1) == (1 << 1);}

        /// \brief Judges whether this FA is minimal or not.
        ///
        /// ture means minimal, false means not.
        /// \return A boolean.
        bool isMinimal() const {return (flag & 1 << 2) == (1 << 2);}

        /// \brief Gets the alphabet.
        /// \return A reference set of Characters.
        CharacterSet& getAlphabet() {return alphabet;}

        /// \brief Gets the alphabet.
        /// \return A const reference set of Characters.
        const CharacterSet& getAlphabet() const {return alphabet;}
        
        /// \brief Sets alphabet given a set of Character.
        ///
        /// Clears current alphabet and copy param charSet to alphabet.
        /// \param charSet a const reference set of Character. 
        void setAlphabet(const CharacterSet &charSet) {
            alphabet.clear();
            alphabet.insert(charSet.begin(),charSet.end());
        }

        /// \brief Sets alphabet given a ordered_set of Character.
        ///
        /// Clears current alphabet and copy param charSet to alphabet.
        /// \param charSet a const reference ordered_set of Character. 
        void setAlphabet(const set<Character> &charSet) {
            alphabet.clear();
            alphabet.insert(charSet.begin(),charSet.end());
        }


        /// \brief Gets a FA which is the intersection of param lhsfa and param rhsfa.
        ///
        /// A static function.
        /// \param lhsfa A const reference FA.
        /// \param rhsfa A const reference FA.
        /// \return A reference of FA.
        static FA& intersectFA(const FA& lhsfa, const FA& rhsfa) {
            if (lhsfa.isNULL() || rhsfa.isNULL()) return EmptyDFA();
            DFA<Character>* lhsdfa = lhsfa.minimize().getDFA();
            DFA<Character>* rhsdfa = rhsfa.minimize().getDFA();
            DFA<Character>* dfa = new DFA<Character>(lhsdfa -> getAlphabet()); 
            DFAStatePairMap pairMap;
            DFAState<Character>* initialState = dfa -> mkInitialState();
            intersectFA(dfa, initialState, DFAState2(lhsdfa -> getInitialState(), rhsdfa -> getInitialState()), pairMap);
            dfa -> setReachableFlag(1);
            return *dfa;
        }

        /// \brief Gets a FA which is the union of param lhsfa and param rhsfa.
        ///
        /// A static function.
        /// \param lhsfa A const reference FA.
        /// \param rhsfa A const reference FA.
        /// \return A reference of FA.
        static FA& unionFA(const FA& lhsfa, const FA& rhsfa) {
            if (lhsfa.isNULL()) return rhsfa.copy();
            if (rhsfa.isNULL()) return lhsfa.copy();
            DFA<Character>* lhsdfa = lhsfa.minimize().getDFA();
            DFA<Character>* rhsdfa = rhsfa.minimize().getDFA();
            NFA<Character>* nfa = new NFA<Character>(lhsdfa -> getAlphabet()); 
            NFAState<Character>* initialState = nfa -> mkInitialState();
            unionFA(nfa, DFAState2(lhsdfa -> getInitialState(), rhsdfa -> getInitialState()));
            return *nfa;
        }

        /// \brief Gets a FA which is the concatenation of param lhsfa and param rhsfa.
        ///
        /// A static function.
        /// \param lhsfa A const reference FA.
        /// \param rhsfa A const reference FA.
        /// \return A reference of FA.
        static FA& concatenateFA(const FA& lhsfa, const FA& rhsfa) {
            if (lhsfa.isNULL()) return rhsfa.copy();
            if (rhsfa.isNULL()) return lhsfa.copy();
            DFA<Character>* lhsdfa = lhsfa.minimize().getDFA();
            DFA<Character>* rhsdfa = rhsfa.minimize().getDFA();
            NFA<Character>* nfa = new NFA<Character>(lhsdfa -> getAlphabet()); 
            NFAState<Character>* initialState = nfa -> mkInitialState();
            concatenateFA(nfa, DFAState2(lhsdfa -> getInitialState(), rhsdfa -> getInitialState()));
            return *nfa;
        }

        /// \brief Gets a FA which is the complement of param fa.
        ///
        /// A static function.
        /// \param fa A const reference FA.
        /// \return A reference of FA.
        static DFA<Character>& complementFA(const FA& fa) {
            if (fa.isNULL()) return EmptyDFA();
            DFA<Character>* mindfa = fa.minimize().getDFA();
            DFA<Character>* dfa = new DFA<Character>(fa.getAlphabet()); 
            DFAState<Character>* initialState = dfa -> mkInitialState();
            complementFA(dfa, mindfa -> getInitialState());
            return *dfa;
        }

        /// \brief Gets a FA which is the deference from param lhsfa to param rhsfa.
        ///
        /// A static function.
        /// \param lhsfa A const reference FA.
        /// \param rhsfa A const reference FA.
        /// \return A reference of FA.
        static FA& minusFA(const FA& lhsfa, const FA& rhsfa) {
            return intersectFA(lhsfa, complementFA(rhsfa));
        }
        
        /// \brief Gets whether this FA is NULL.
        /// \return A boolean.
        virtual bool isNULL() const = 0;

        /// \brief Gets a FA which is the intersection of this FA and param fa.
        /// \param fa A const reference FA.
        /// \return A reference of FA.
        FA& operator & (const FA &fa) const {
            return intersectFA(*this, fa);
        }

        /// \brief Gets a FA which is the union of this FA and param fa.
        /// \param fa A const reference FA.
        /// \return A reference of FA.
        FA& operator | (const FA &fa) const { 
            return unionFA(*this, fa);
        }

        /// \brief Gets a FA which is the diference set from this FA to param fa.
        /// \param fa A const reference FA.
        /// \return A reference of FA.
        FA& operator -(const FA &fa) const {
            return minusFA(*this, fa);
        }

        /// \brief Gets a FA which is the complement of this FA.
        /// \return A reference of DFA.
        DFA<Character>& operator ! ( void ) const {
            return complementFA(*this);
        }

        /// \brief Gets a FA which is the concatination of this FA and param fa.
        /// \param fa A const reference FA.
        /// \return A reference of FA.
        FA& concatenateFA(const FA &fa) const {
            return concatenateFA(*this, fa);
        }

        /// \brief Gets the intersection of param faSet.
        /// \param faSet A set of FA.
        /// \return A reference of FA.
        static FA& intersectFA(const FASet& faSet) {
            DFASet dfaSet;
            DFAStateSet stateSet;
            for (FA* fa : faSet) {
                if(fa -> isNULL()) return FA::EmptyDFA();
                DFA<Character>*dfa = &(fa -> determinize());
                if(!fa -> isDeterministic()) dfaSet.insert(dfa);
                DFAState<Character>* initialState = dfa -> getInitialState();
                stateSet.insert(initialState);
            }
            DFA<Character>*dfa = new DFA<Character>();
            DFAState<Character>* initialState = dfa -> mkInitialState();
            DFAStateSetMap setMap;
            setMap[stateSet] = initialState;
            intersectFA(dfa, initialState, stateSet, setMap);
            for (DFA<Character>* dfa : dfaSet) {
                delete dfa;
            }
            dfa -> setReachableFlag(1);
            return *dfa;
        }
        //        static bool multiIntersectionAndDeterminEmptiness(const FASet &faSet);//todo
        
        /// \brief Gets the concatenation of param faList.
        /// \param faList A list of FA.
        /// \return A reference of FA.
        static FA& concatenateFA(const FAList& faList) {
            NFA<Character>*nfa = new NFA<Character>();
            NFAState<Character>* iniState = nfa -> mkNFAInitialState();
            NFAStateSet fStateSet;
            fStateSet.insert(iniState);
            DFAState2NFAStateMap dfaState2Map;
            NFAState2Map nfaState2Map;
            for (FA* fa : faList) {
                if (fa -> isNULL()) continue;
                NFAState<Character>* state = nfa -> mkNFAState();
                for (NFAState<Character>* nfaState : fStateSet)
                    nfaState -> addEpsilonTrans(state);
                fStateSet.clear();
                nfa -> clearFinalStateSet();
                if (fa -> isDeterminate()) {
                    dfaState2Map.clear();
                    DFA<Character>&fa = fa -> determine();
                    DFAState<Character>* iniState = fa.getInitialState();
                    if(iniState->isFinal()) nfa -> addFinalState(state);
                    dfaState2Map[iniState] = state;
                    nfa -> makeCopyTransByDFA(iniState, dfaState2Map);
                    
                } else {
                    nfaState2Map.clear();
                    NFA<Character> &fa = fa -> nondetermine();
                    NFAState<Character>* iniState = fa.getInitialState();
                    if (iniState -> isFinal()) nfa -> addFinalState(state);
                    nfaState2Map[iniState] = state;
                    nfa->makeCopyTransByNFA(iniState, nfaState2Map);
                }
                fStateSet.insert(nfa -> finalStateSet.begin(), nfa -> finalStateSet.end());
            }
            return *nfa;
        }
        
        /// \brief Gets the union of param faSet.
        /// \param faSet A set of FA.
        /// \return A reference of FA.
        static FA& unionFA(const FASet& faSet) {
            NFA<Character>* nfa = new NFA<Character>();
            NFAState<Character>* iniState = nfa -> mkNFAInitialState();
            DFAState2NFAStateMap dfaState2Map;
            NFAState2Map nfaState2Map;
            for(FA* fa : faSet)
            {
                if (fa -> isNULL()) continue;
                NFAState<Character>* state = nfa -> mkNFAState();
                iniState -> addEpsilonTrans(state);
                if (fa -> isDeterminate()) {
                    dfaState2Map.clear();
                    DFA<Character>& tempDfa = fa -> determine();
                    DFAState<Character>* iniState = tempDfa.getInitialState();
                    if(iniState -> isFinal()) nfa->addFinalState(state);
                    dfaState2Map[iniState] = state;
                    nfa -> makeCopyTransByDFA(iniState, dfaState2Map);
                } else {
                    nfaState2Map.clear();
                    NFA<Character>& tempNfa = fa -> nondetermine();
                    NFAState<Character>* iniState = tempNfa.getInitialState();
                    if(iniState -> isFinal()) nfa -> addFinalState(state);
                    nfaState2Map[iniState] = state;
                    nfa -> makeCopyTransByNFA(iniState, nfaState2Map);
                }
            }
            return *nfa;
        }
        
        /// \brief Gets a DFA which determinized by FA.
        /// \return A reference of DFA.
        virtual DFA<Character>& determinize( void ) = 0;

        /// \brief Gets a NFA which nondeterminized by FA.
        /// \return A reference of NFA.
        virtual NFA<Character>& nondeterminize( void ) = 0;

        /// \brief Gets a FA which is the right quotient by param character of this FA.
        /// \param character A Character.
        /// \return A reference of FA.
        virtual FA& rightQuotient(Character character) = 0;

        /// \brief Gets a FA which is the left quotient by param character of this FA.
        /// \param character A Character.
        /// \return A reference of FA.
        virtual FA& leftQuotient(Character character) = 0;
        
        /// \brief Removes all unreachable states from initial state.
        virtual void removeUnreachableState() = 0;

        /// \brief Removes all dead states which cannot reach final states.
        virtual void removeDeadState() = 0;
        
        //virtual Word getOneRun() = 0; todo
        
        /// \brief Judges whether this FA is accepted by a param word. 
        /// \param word A const reference of vector of Character.
        /// \return A boolean.
        virtual bool isAccepted(const Word &word) = 0;

        /// \brief Judges whether this FA is accepted by a param character. 
        /// \param word A const Character.
        /// \return A boolean.
        virtual bool isAccepted(Character character) = 0;
        
        virtual void output()const = 0;

        virtual void print(string filename)const = 0;
        
        virtual bool isEmpty() = 0;
        
        bool operator ==(const FA& fa )
        {
            DFA<Character>& cDFA = !(fa);
            DFA<Character>& iDFA = (*this & cDFA).determinize();
            if(!iDFA.isEmpty())
            {
                delete &cDFA;
                delete &iDFA;
                return false;
            }
            cDFA = !(*this);
            iDFA = (cDFA & fa).determinize();
            if(!iDFA.isEmpty())
            {
                delete &cDFA;
                delete &iDFA;
                return false;
            }
            delete &cDFA;
            delete &iDFA;
            return true;
        }
        bool operator <=(const FA& fa )
        {
            DFA<Character>& cDFA = !(fa);
            DFA<Character>& iDFA = (*this & cDFA).determinize();
            if(!iDFA.isEmpty())
            {
                delete &cDFA;
                delete &iDFA;
                return false;
            }
            delete &cDFA;
            delete &iDFA;
            return true;
        }
        static DFA<Character>& EmptyDFA()
        {
            DFA<Character>* dfa = new DFA<Character>();
            return *dfa;
        }
        static NFA<Character>& EmptyNFA()
        {
            NFA<Character>* nfa = new NFA<Character>();
            return *nfa; 
        }
        static DFA<Character>& CompleteFA(const CharacterSet &charSet)
        {
            DFA<Character>* dfa = new DFA<Character>();
            DFAState<Character>* iniState = dfa->mkDFAFinalState();
            dfa->setAlphabet(charSet);
            for(Character character : charSet)
                iniState->addDFATrans(character, iniState);
            return *dfa;
        }
        static DFA<Character>& SigmaStarFA(const CharacterSet &charSet)
        {
            DFA<Character>* dfa = new DFA<Character>();
            DFAState<Character>* iniState = dfa->mkDFAInitialState();
            dfa->addFinalState(iniState);
            dfa->setAlphabet(charSet);
            for(Character character : charSet)
                iniState->addDFATrans(character, iniState);
            return *dfa;
        }
        
        friend DFA<Character>;
        friend NFA<Character>;
        
    };
    
    
    
    
    
}

#endif /* FA_hpp */
