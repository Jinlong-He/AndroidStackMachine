//
//  NFA.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/3.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef NFA_hpp
#define NFA_hpp

#include "FA.hpp"
#include "NFAState.hpp"
#include "PDS.hpp"
#include "RegularExp.hpp"

namespace cgh {
    
    /// \brief A class of Nondeterministic Finite Automaton.
    template <class Character>
    class NFA : public FA<Character>
    {
        
        typedef typename Global<Character>::Word Word;
        typedef typename Global<Character>::Char2 Char2;
        typedef typename Global<Character>::CharacterSet CharacterSet;
        typedef typename Global<Character>::NFAStateSet NFAStateSet;
        typedef typename Global<Character>::NFATransMap NFATransMap;
        
        typedef typename Global<Character>::NFAState2Map NFAState2Map;
        typedef typename Global<Character>::DFAState2NFAStateMap DFAState2NFAStateMap;
        typedef typename Global<Character>::PDSState2NFAStateMap PDSState2NFAStateMap;
        typedef typename Global<Character>::NFAStateSet2DFAStateMap NFAStateSet2DFAStateMap;
        typedef typename Global<Character>::NFAState2NFAStateSetMap NFAState2NFAStateSetMap;
        
    private:
        typedef pair<NFAState<Character>* , Character> StateChar;
        typedef pair<StateChar, Character> StateChar2;
        typedef set<StateChar> StateCharSet;
        typedef set<StateChar2> StateChar2Set;
        typedef unordered_map<Character, StateCharSet> Char2StateCharSetMap;
        typedef unordered_map<Character, StateChar2Set> Char2StateChar2SetMap;
        typedef unordered_map<NFAState<Character>* , Char2StateCharSetMap> NeedMap;
        typedef unordered_map<NFAState<Character>* , Char2StateChar2SetMap> Need2Map;
        
    private:
        NFAState<Character>*  initialState;     ///< The initial state for this NFA.
        NFAStateSet stateSet;       ///< The set of states for this NFA.
        NFAStateSet finalStateSet;  ///< The set of final states for this NFA.
    private:
        void cpTransByDFA(DFAState<Character>* state, DFAState2NFAStateMap &state2map) {
            FA<Character>::cpNFATransByDFA(this, state, state2map);
        }

        void cpTransByNFA(NFAState<Character>* state, NFAState2Map &state2map) {
            FA<Character>::cpNFATransByNFA(this, state, state2map);
        }

        void cpTransByNFA(NFAState<Character>* state, NFAState2Map &state2map, unordered_map<Character, Character>& char2Map) {
            NFAState<Character>*  sourceState = state2map[state];
            if (state -> isFinal()) addFinalState(sourceState);
            for (auto& mapPair : state -> getNFATransMap()) {
                for (NFAState<Character>*  state : mapPair.second){
                    NFAState<Character>*  targetState = nullptr;
                    auto state2MapIt = state2map.find(state);
                    if (state2MapIt == state2map.end()) {
                        targetState = mkState();
                        state2map[state] = targetState;
                        cpTransByNFA(state, state2map, char2Map);
                    } else {
                        targetState = state2MapIt -> second;
                    }
                    sourceState -> addNFATrans(char2Map[mapPair.first], targetState);
                }
            }
        }

        void getReachableStateSet(NFAStateSet& reachableStateSet, NFAStateSet& workSet) {
            NFAStateSet targetStateSet, newWorkSet;
            for (NFAState<Character>*  state : workSet) {
                targetStateSet.clear();
                state -> getTargetStateSet(targetStateSet);
                for (NFAState<Character>*  targetState : targetStateSet) {
                    if (reachableStateSet.insert(targetState).second)
                        newWorkSet.insert(targetState);
                }
            }
            if (newWorkSet.size() == 0) return;
            getReachableStateSet(reachableStateSet, newWorkSet);
        }
        
        void getReverseMap(NFAState2NFAStateSetMap& reverseMap) {
            for (NFAState<Character>*  state : stateSet) {
                for (auto& mapPair : state -> getNFATransMap()) {
                    for (NFAState<Character>*  nfaState : mapPair.second) {
                        reverseMap[nfaState].insert(state);
                    }
                }
            }
        }
        
        void getLiveStateSet(const NFAState2NFAStateSetMap& reverseMap, NFAStateSet& liveStateSet, NFAStateSet& workSet) {
            NFAStateSet newWorkSet;
            for (NFAState<Character>*  state : workSet) {
                auto mapIt = reverseMap.find(state);
                if (mapIt != reverseMap.end()) {
                    for (NFAState<Character>*  reverseState : mapIt -> second) {
                        if (liveStateSet.insert(reverseState).second) {
                            newWorkSet.insert(reverseState);
                        }
                    }
                }
            }
            if (workSet.size() == 0) return;
            getLiveStateSet(reverseMap, liveStateSet, newWorkSet);
        }

        void getTransMapByStateSet(const NFAStateSet& stateSet, NFATransMap& nfaTransMap) const {
            for (NFAState<Character>*  state : stateSet) {
                NFAStateSet workSet;
                for (auto& mapPair : state -> getNFATransMap()) {
                    if (mapPair.first != Global<Character>::epsilon) {
                        workSet.clear();
                        state -> getTargetStateSetByChar(workSet, mapPair.first);
                        if (workSet.size() > 0)
                            nfaTransMap[mapPair.first].insert(workSet.begin(), workSet.end());
                    }
                }
            }
        }
        
        void determinize(DFA<Character>* dfa, DFAState<Character>*  sourceState, const NFAStateSet& nfaStateSet, NFAStateSet2DFAStateMap &setMap) const {
            if (hasFinalState(nfaStateSet)) dfa -> addFinalState(sourceState);
            NFATransMap transMap;
            getTransMapByStateSet(nfaStateSet, transMap);
            for (auto& mapPair : transMap) {
                auto setMapIt = setMap.find(mapPair.second);
                DFAState<Character>*  targetState = nullptr;
                if (setMapIt == setMap.end()) {
                    targetState = dfa -> mkState();
                    setMap[mapPair.second] = targetState;
                    determinize(dfa, targetState, mapPair.second, setMap);
                } else {
                    targetState = setMapIt -> second;
                }
                sourceState -> addDFATrans(mapPair.first, targetState);
            }
        }
        
        bool addNeedMap(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc, NeedMap &needMap, Need2Map& need2Map) {
            StateChar stateChar(sState, sc);
            StateCharSet stateCharSet;
            stateCharSet.insert(stateChar);
            auto needmapIt = needMap.find(tState);
            if (needmapIt != needMap.end()) {
                auto mapIt = needmapIt -> second.find(tc);
                if (mapIt != needmapIt -> second.end()) {
                    if (!mapIt -> second.insert(stateChar).second) return false;
                } else {
                    needmapIt -> second[tc] = stateCharSet;
                }
            } else {
                Char2StateCharSetMap char2StateCharSetMap;
                char2StateCharSetMap[tc] = stateCharSet;
                needMap[tState] = char2StateCharSetMap;
            }
            return true;
        }
        bool addNeed2Map(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc1, Character tc2, NeedMap& needMap, Need2Map &need2Map) {
            StateChar stateChar(sState, sc);
            StateChar2 stateChar2(stateChar, tc2);
            StateChar2Set stateChar2Set;
            stateChar2Set.insert(stateChar2);
            auto need2MapIt = need2Map.find(tState);
            if (need2MapIt != need2Map.end()) {
                auto mapIt = need2MapIt -> second.find(tc1);
                if (mapIt != need2MapIt -> second.end()) {
                    if (!mapIt -> second.insert(stateChar2).second) return false;
                } else {
                    need2MapIt -> second[tc1] = stateChar2Set;
                }
            } else {
                Char2StateChar2SetMap char2StateChar2SetMap;
                char2StateChar2SetMap[tc1] = stateChar2Set;
                need2Map[tState] = char2StateChar2SetMap;
            }
            return true;
        }
        
        //void addPreStarNeedMap(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc, NeedMap &needMap, Need2Map& need2Map)
        //{
        //    if(addNeedMap(sState, sc, tState, tc, needMap, need2Map))
        //        addPreStarTrans(sState, sc, tState, tc, needMap, need2Map);
        //}
        //
        //void addPreStarNeed2Map(NFAState<Character>*  sState, Character sc, NFAState<Character>*  tState, Character tc1, Character tc2, NeedMap& needMap, Need2Map& need2Map)
        //{
        //    if(addNeed2Map(sState, sc, tState, tc1, tc2, needMap, need2Map))
        //    {
        //        NFAStateSet stateset = tState -> getTargetStateSetByChar(tc1);
        //        for(NFAState<Character>*  state : stateset)
        //            addPreStarNeedMap(sState, sc, state, tc2, needMap, need2Map);
        //    }
        //}
        //
        //void addPreStarTrans(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc, NeedMap& needMap, Need2Map& need2Map)
        //{
        //    NFAStateSet stateset;
        //    tState -> getTargetStateSetByChar(stateset, tc);
        //    for(NFAState<Character>*  state : stateset)
        //        if(sState -> addNFATrans(sc, state))
        //            addPreStarTrans(sState, sc, state, needMap, need2Map);
        //}
        //void addPreStarTrans(NFAState<Character>* sState, Character c, NFAState<Character>* tState, NeedMap& needMap, Need2Map& need2Map)
        //{
        //    Char2StateCharSetMap map = needMap[sState];
        //    Char2StateCharSetMapIter mapIt = map.find(c);
        //    if(mapIt != map.end())
        //    {
        //        StateCharSet stateCharSet = mapIt -> second;
        //        for(StateChar scpair : stateCharSet)
        //            if(scpair.first -> addNFATrans(scpair.second, tState))
        //                addPreStarTrans(scpair.first, scpair.second, tState, needMap, need2Map);
        //    }
        //    Char2StateChar2SetMap map2 = need2Map[sState];
        //    Char2StateChar2SetMapIter map2It = map2.find(c);
        //    if(map2It != map2.end())
        //    {
        //        StateChar2Set stateChar2Set = map2It -> second;
        //        for(StateChar2 sc2pair : stateChar2Set)
        //            addPreStarNeedMap(sc2pair.first.first, sc2pair.first.second, tState, sc2pair.second, needMap, need2Map);
        //    }
        //}
        //
        //
        void addPostStarNeedMap(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc, NeedMap &needMap, Need2Map& need2Map) {
            if (addNeedMap(sState, sc, tState, tc, needMap, need2Map))
                addPostStarTrans(sState, sc, tState, tc, needMap, need2Map);
        }
        
        void addPostStarNeed2Map(NFAState<Character>*  sState, Character sc, NFAState<Character>*  tState, Character tc1, Character tc2, NeedMap& needMap, Need2Map& need2Map) {
            if (addNeed2Map(sState, sc, tState, tc1, tc2, needMap, need2Map)) {
                NFAState<Character>*  midState = mkState();
                sState -> addNFATrans(sc, midState);
                addPostStarTrans(sState, sc, midState, needMap, need2Map);
                addPostStarNeedMap(midState, tc2, tState, tc1, needMap, need2Map);
            }
        }
        
        void addPostStarTrans(NFAState<Character>* sState, Character sc, NFAState<Character>* tState, Character tc, NeedMap& needMap, Need2Map& need2Map) {
            NFAStateSet nfaStateSet;
            tState -> getTargetStateSetByChar(nfaStateSet, tc);
            for (NFAState<Character>*  state : nfaStateSet) {
                if (sState -> addNFATrans(sc, state)) {
                    addPostStarTrans(sState, sc, state, needMap, need2Map);
                }
            }
        }
        
        void addPostStarTrans(NFAState<Character>* sState, Character c, NFAState<Character>* tState, NeedMap& needMap, Need2Map& need2Map) {
            Char2StateCharSetMap& map = needMap[sState];
            auto mapIt = map.find(c);
            if (mapIt != map.end()) {
                for (StateChar scpair : mapIt -> second) {
                    if (scpair.first -> addNFATrans(scpair.second, tState)) {
                        addPostStarTrans(scpair.first, scpair.second, tState, needMap, need2Map);
                    }
                }
            }
        }
    public:
        /// \brief Default construction function, sets initialState to nullptr.
        NFA() : FA<Character>(), initialState(nullptr){}

        /// \brief Construction function with alphabet.
        /// \param charSet The alphabet.
        NFA(const CharacterSet& charSet) : FA<Character>(charSet), initialState(NULL) {}

        /// \brief Copy construction function.
        /// \param nfa The copied NFA.
        NFA(const NFA& nfa) {
            if (!nfa.isNULL()) {
                this -> flag = nfa.flag;
                this -> setAlphabet(nfa.alphabet);
                NFAState<Character>*  iniState = mkInitialState();
                if (nfa.initialState -> isFinal())
                    addFinalState(iniState);
                NFAState2Map state2Map;
                state2Map[nfa.initialState] = iniState;
                cpTransByNFA(nfa.initialState, state2Map);
            }
        }

        NFA(const NFA& nfa, unordered_map<Character, Character>& char2Map) {
            if (!nfa.isNULL()) {
                this -> flag = nfa.flag;
                CharacterSet charSet;
                for (Character c : nfa.alphabet) {
                    charSet.insert(char2Map[c]);
                }
                this -> setAlphabet(charSet);
                NFAState<Character>*  iniState = mkInitialState();
                NFAState2Map state2Map;
                state2Map[nfa.initialState] = iniState;
                cpTransByNFA(nfa.initialState, state2Map, char2Map);
            }
        }

        /// \brief Copy construction function with state2Map.
        /// \param nfa The copied NFA.
        /// \param state2Map Records the map for outside.
        NFA(const NFA& nfa, NFAState2Map& state2Map) {
            if (!nfa.isNULL()) {
                this -> flag = nfa.flag;
                this -> setAlphabet(nfa.alphabet);
                NFAState<Character>*  iniState = mkInitialState();
                state2Map[nfa.initialState] = iniState;
                cpTransByNFA(nfa.initialState, state2Map);
            }
        }

        /// \brief Copy construction function by DFA.
        /// \param dfa The copied DFA.
        NFA(const DFA<Character>& dfa) {
            if(!dfa.isNULL())
            {
                this -> flag = dfa.flag;
                this -> setAlphabet(dfa.alphabet);
                NFAState<Character>*  iniState = mkInitialState();
                if (dfa.initialState -> isFinal())
                    addFinalState(iniState);
                DFAState2NFAStateMap state2Map;
                state2Map[dfa.initialState] = iniState;
                cpTransByDFA(dfa.initialState, state2Map);
            }
        }

        /// \brief Copy construction function by regEx.
        /// \param regEx The regular expression.
        NFA(const string& regEx) {
            BasicRegEx<Character> basicRegEx(regEx);
            *this = *basicRegEx.mkNFA();
        }
        
        /// \brief Desconstruction function.
        ///
        /// delete all pointers of states for this NFA.
        ~NFA() {
            initialState = nullptr;
            for (NFAState<Character>*  state : stateSet) {
                delete state;
            }
        }

        FA<Character>& copy() {
            return *(new NFA(*this));
        }

        /// \brief Sets initialState to param state.
        /// \param state The state set to be initialState.
        void setInitialState(NFAState<Character>*  state) {initialState = state;}

        /// \brief Adds param state to finalStateSet.
        /// \param state The state need to be added in finalStateSet.
        void addFinalState(NFAState<Character>*  state) {
            finalStateSet.insert(state);
            state -> setFinalFlag(1);
        }

        /// \brief Gets stateSet.
        /// \return The set reference of NFAState pointer for this NFA.
        NFAStateSet& getStateSet() {return stateSet;}

        /// \brief Gets finalStateSet.
        /// \return The set reference of NFAState pointer for this NFA.
        NFAStateSet& getFinalStateSet() {return finalStateSet;}

        /// \brief Gets initialState.
        /// \return The NFAstate pointer of initialState for this NFA.
        NFAState<Character>*  getInitialState() {return initialState;}

        /// \brief Gets stateSet, a const function.
        /// \return The const set reference of NFAState pointer for this NFA.
        const NFAStateSet& getStateSet() const {return stateSet;}

        /// \brief Gets finalStateSet, a const function.
        /// \return The const set reference of NFAState pointer for this NFA.
        const NFAStateSet& getFinalStateSet() const {return finalStateSet;}

        /// \brief Gets initialState, a const function.
        /// \return The const NFAState pointer of initialState for this NFA.
        const NFAState<Character>*  getInitialState() const {return initialState;}

        /// \brief Removes all state in the finalStateSet for this NFA.
        void clearFinalStateSet()
        {
            for (NFAState<Character>*  state : finalStateSet) {
                state -> setFinalFlag(0);
            }
            finalStateSet.clear();
        }

        /// \brief Checks whether given param stateSet has finalState.
        /// \param stateSet The NFAStateSet for checking.
        /// \return True means param stateSet has finalState, otherwise not.
        static bool hasFinalState(const NFAStateSet& stateSet)
        {
            for (NFAState<Character>*  state : stateSet) {
                if (state -> isFinal()) return true;
            }
            return false;
        }

        /// \brief Checks whether all states in the given param stateSet are finalState.
        /// \param stateSet The NFAStateSet for checking.
        /// \return True means all states are finalState, otherwise not.
        static bool allFinalState(const NFAStateSet& stateSet)
        {
            for (NFAState<Character>*  state : stateSet) {
                if (!state -> isFinal()) return false;
            }
            return true;
        }

        bool isNULL() const {
            if (!initialState) return true;
            if (finalStateSet.size() == 0) return true;
            return false;
        }

        /// \brief Makes a state in this NFA.
        /// \return A NFAState pointer made by this NFA.
        NFAState<Character>*  mkState() {
            NFAState<Character>* state = new NFAState<Character>();
            stateSet.insert(state);
            return state;
        }
        
        /// \brief Makes a initialState in this NFA.
        /// \return A NFAState pointer made by this NFA.
        NFAState<Character>*  mkInitialState() {
            initialState = mkState();
            return initialState;
        }
        
        /// \brief Makes a finalState in this NFA.
        /// \return A NFAState pointer made by this NFA.
        NFAState<Character>*  mkFinalState() {
            NFAState<Character>* state = mkState();
            state -> setFinalFlag(1);
            finalStateSet.insert(state);
            return state;
        }

        DFA<Character>& determinize( void ) {
            if (isNULL()) return FA<Character>::EmptyDFA();
            DFA<Character>* dfa = new DFA<Character>(this -> alphabet);
            NFAStateSet epsilonClosure;
            initialState -> getEpsilonClosure(epsilonClosure);
            epsilonClosure.insert(initialState);
            NFAStateSet2DFAStateMap setMap;
            DFAState<Character>* iniState = dfa -> mkInitialState();
            determinize(dfa, iniState, epsilonClosure, setMap);
            dfa -> setReachableFlag(1);
            return *dfa;
        }

        const DFA<Character>& determinize( void ) const {
            return const_cast<NFA*>(this) -> determinize();
        }
        
        NFA& nondeterminize( void ) {
            return *this;
        }

        const NFA& nondeterminize( void ) const {
            return *this;
        }

        FA<Character>& subset(const NFAState<Character>* iState, const NFAState<Character>* fState)
        {
            if (isNULL()) return FA<Character>::EmptyNFA();
            NFA *nfa = new NFA(this -> alphabet);
            NFAState<Character>*  iniState = nfa -> mkInitialState();
            NFAState2Map state2Map;
            state2Map[const_cast<State*>(iState)] = iniState;
            nfa -> cpTransByNFA((const_cast<State*>(iState)), state2Map);
            nfa -> clearFinalStateSet();
            NFAState<Character>*  nfaState = (state2Map[const_cast<State*>(fState)]);
            nfa -> addFinalState(nfaState);
            nfa -> removeDeadState();
            return *nfa;
        }

        FA<Character>& rightQuotient(Character character)
        {
            NFA* nfa = new NFA(*this);
            NFAStateSet finSteteSet;
            NFAStateSet tempSet;
            for (NFAState<Character>*  state : nfa -> stateSet) {
                tempSet.clear();
                state -> getTargetStateSetByChar(tempSet, character);
                if (NFA::hasFinalState(tempSet))
                    finSteteSet.insert(state);
            }
            nfa -> clearFinalStateSet();
            for(NFAState<Character>*  state : finSteteSet)
                nfa -> addFinalState(state);
            return *nfa;
        }
        
        FA<Character>& leftQuotient(Character character)
        {
            if (initialState -> getNFATransMap().count(character) == 0) return FA<Character>::EmptyNFA();
            NFA* nfa = new NFA(*this);
            NFAStateSet set;
            nfa -> initialState -> getTargetStateSetByChar(set, character);
            NFAState<Character>*  iniState = nfa -> mkInitialState();
            for(NFAState<Character>*  state : set)
                iniState -> addEpsilonTrans(state);
            nfa -> removeUnreachableState();
            return *nfa;
        }
        
        
        void removeUnreachableState()
        {
            if (isNULL()) return;
            NFAStateSet reachableStateSet, workSet;
            workSet.insert(initialState);
            reachableStateSet.insert(initialState);
            getReachableStateSet(reachableStateSet, workSet);
            if (!NFA::hasFinalState(reachableStateSet)) {
                initialState = NULL;
                return;
            }
            if (reachableStateSet.size() != this -> stateSet.size()) {
                NFAStateSet delSet;
                for (NFAState<Character>*  state : stateSet) {
                    if (reachableStateSet.count(state) == 0) {
                        NFAStateSet targetStateSet = state -> getTargetStateSet();
                        for (NFAState<Character>*  tState : targetStateSet) {
                            if (reachableStateSet.count(tState) > 0)
                                state -> delNFATrans(tState);
                        }
                        delSet.insert(state);
                    }
                }
                for (NFAState<Character>*  state : delSet) {
                    stateSet.erase(state);
                    delete state;
                }
            }
            this -> setReachableFlag(1);
        }

        void removeDeadState()
        {
            if (isNULL()) return;
            NFAState2NFAStateSetMap reverseMap;
            getReverseMap(reverseMap);
            NFAStateSet liveStateSet(finalStateSet.begin(), finalStateSet.end());
            getLiveStateSet(reverseMap, liveStateSet, finalStateSet);
            if (liveStateSet.count(initialState) == 0) {
                initialState = NULL;
                return;
            }
            NFAStateSet delSet;
            for(NFAState<Character>*  state : stateSet) {
                if (liveStateSet.count(state) == 0) {
                    NFAStateSet sourceStateSet = reverseMap.find(state) -> second;
                    for (NFAState<Character>*  sState : sourceStateSet) {
                        if (liveStateSet.count(sState) > 0)
                            sState -> delNFATrans(state);
                    }
                    delSet.insert(state);
                }
            }
            for (NFAState<Character>*  state : delSet) {
                stateSet.erase(state);
                delete state;
            }
        }

        //        Word getOneRun();
        
        bool isAccepted(const Word &word)
        {
            if (isNULL()) return false;
            NFAStateSet workSet;
            workSet.insert(initialState);
            for (Character c : word) {
                if (workSet.size() > 0) {
                    NFAStateSet newWorkSet;
                    for(NFAState<Character>*  state : workSet)
                        state -> getTargetStateSetByChar(newWorkSet, c);
                    workSet.clear();
                    if (newWorkSet.size() > 0) {
                        workSet.insert(newWorkSet.begin(), newWorkSet.end());
                    } else {
                        return false;
                    }
                }
            }
            if (workSet.size() == 0) return false;
            if (NFA::hasFinalState(workSet)) return true;
            return false;
        }

        bool isAccepted(Character character)
        {
            if (isNULL()) return false;
            NFAStateSet workSet;
            initialState -> getTargetStateSetByChar(workSet, character);
            if (workSet.size() == 0) return false;
            if (NFA::hasFinalState(workSet)) return true;
            return false;
        }
        
        bool isEmpty()
        {
            if (isNULL()) return true;
            if (!this -> isReachable()) removeUnreachableState();
            if (finalStateSet.size() == 0) return true;
            return false;
        }
        
        //NFA& preStar(const PDS& pds, PDSState2NFAStateMap& state2Map)
        //{
        //    NFAState2Map copyMap;
        //    NFA* nfa = new NFA(*this, copyMap);
        //    NeedMap needMap;
        //    Need2Map need2Map;
        //    PDSStateSet tempSet;
        //    for(PDSState2NFAStateMapIter it = state2Map.begin(); it != state2Map.end(); it++)
        //        tempSet.insert(it -> first);
        //    for(PDSState* state : pds.getStateSet())
        //    {
        //        if(tempSet.find(state) == tempSet.end())
        //            state2Map[state] = nfa -> mkState();
        //        else
        //            state2Map[state] = copyMap[state2Map[state]];
        //    }
        //    
        //    for(PDSTrans* trans : pds.pdsPopTransList)
        //    {
        //        NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
        //        NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
        //        Character character = trans -> getChar();
        //        sourceState -> addNFATrans(character, targetState);
        //    }
        //    for(ReplacePDSTrans* trans : pds.pdsReplaceTransList)
        //    {
        //        NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
        //        NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
        //        Character character = trans -> getChar();
        //        Character stack = trans -> getStack();
        //        nfa -> addPreStarNeedMap(sourceState, character, targetState, stack, needMap, need2Map);
        //    }
        //    for(PushPDSTrans* trans : pds.pdsPushTransList)
        //    {
        //        NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
        //        NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
        //        Character character = trans -> getChar();
        //        Char2 stack = trans -> getStack();
        //        nfa -> addPreStarNeed2Map(sourceState, character, targetState, stack.first, stack.second, needMap, need2Map);
        //    }
        //    nfa -> removeUnreachableState();
        //    nfa -> removeDeadState();
        //    return *nfa;
        //}
        //
        NFA& postStar(const PDS<Character>& pds, PDSState2NFAStateMap& state2Map) {
            NFAState2Map copyMap;
            NFA* nfa = new NFA(*this, copyMap);
            NeedMap needMap;
            Need2Map need2Map;
            PDSStateSet pdsStateSet;
            for (auto& mapPair : state2Map)
                pdsStateSet.insert(mapPair.first);
            for (PDSState* state : pds.getStateSet()) {
                if (pdsStateSet.find(state) == pdsStateSet.end()) {
                    state2Map[state] = nfa -> mkState();
                } else {
                    state2Map[state] = copyMap[state2Map[state]];
                }
            }
            
            for (PopPDSTrans<Character>* trans : pds.getPopTransList()) {
                NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
                NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
                Character character = trans -> getChar();
                nfa -> addPostStarNeedMap(targetState, Global<Character>::epsilon, sourceState, character, needMap, need2Map);
            }

            for (ReplacePDSTrans<Character>* trans : pds.getReplaceTransList()) {
                NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
                NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
                Character character = trans -> getChar();
                Character stack = trans -> getStack();
                nfa -> addPostStarNeedMap(targetState, stack, sourceState, character, needMap, need2Map);
            }

            for (PushPDSTrans<Character>* trans : pds.getPushTransList()) {
                NFAState<Character>*  sourceState = state2Map[trans -> getSourceState()];
                NFAState<Character>*  targetState = state2Map[trans -> getTargetState()];
                Character character = trans -> getChar();
                Char2& stack = trans -> getStack();
                nfa -> addPostStarNeed2Map(targetState, stack.first, sourceState, character, stack.second, needMap, need2Map);
            }
            nfa -> removeUnreachableState();
            nfa -> removeDeadState();
            return *nfa;
        }
        
        
    
        void output()const{
            if(!initialState) return;
            cout<<initialState -> getID()<<endl;
            for(auto it = stateSet.begin(); it != stateSet.end(); it++)
            {
                if((*it)->isFinal()) cout<<"$"<<(*it)->getID()<<endl;;
                dynamic_cast<NFAState<Character>* >((*it))->output();
            }
        }
        
        void print(string filename)const
        {
            ofstream f;
            f.open(filename);
            f << "digraph {\n";
            f << "rankdir=LR;\n";
            
            
            // cout initial
            f << "Q"<<initialState -> getID() << "[color=blue];\n";
            
            
            // cout final states
            for (auto iter = finalStateSet.begin(); iter != finalStateSet.end(); iter++) {
                f << "Q" << (*iter) -> getID() << " [shape=doublecircle];\n";
            }
            f << ";\n";
            
            
            // cout trisitions
            for(auto iter = stateSet.begin(); iter != stateSet.end(); iter++)
            {
                NFATransMap& transMap = (*iter)->getNFATransMap();
                ID id = (*iter)->getID();
                for (auto iter = transMap.begin(); iter != transMap.end(); iter++) {
                    for (auto iter1 = iter -> second.begin(); iter1 != iter -> second.end(); iter1++) {
                        f << "Q" << id <<  " -> " << "Q" << (*iter1)->getID() << "[label=\"" << iter -> first <<"\"];\n";
                    }
                }
            }
            f <<"}\n";
            f.close();
            system("dot -Tpng -o res.png res.dot");
        }
        friend FA<Character>;
        friend BasicRegEx<Character>;
        
    };
}

#endif /* NFA_hpp */

