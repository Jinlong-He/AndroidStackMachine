//
//  NFAState.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/2.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef NFAState_hpp
#define NFAState_hpp

#include "State.hpp"
namespace cgh{
    template <class Character> class NFA;

    /// \brief States in Nondeterministic Finite Automaton.
    ///
    /// Example:
    ///    NFAState<char>* nfaState0 = nfa -> mkNFAState();
    ///    NFAState<char>* nfaState1 = nfa -> mkNFAState();
    ///    nfaState0 -> addNFATrans('a', nfaState1);
    ///    nfaState0 -> addNFATrans('b', nfaState0);
    ///    nfaState1 -> addEpsilonTrans(nfaState1);
    ///    nfaState0 -> delNFATrans('a');
    ///    nfaState0 -> delNFATrans('b', nfaState0);
    ///    NFATransMap& nfaTransMap = nfaState0 -> getNFATransMap();
    ///    NFAStateSet nfaStateSet = nfaState0 -> getTargetSet();
    ///    NFAStateSet nfaStateSet = nfaState0 -> getTargetSet('a');
    ///    NFAStateSet nfaStateSet = nfaState1 -> getEpsilonClosure();
    ///    nfa -> delNFAState(nfaState0);
    template <class Character>
    class NFAState : public State {
    public:
        typedef Global<Character> Global;
        typedef typename Global::CharacterSet CharacterSet;
        typedef typename Global::NFAStateSet NFAStateSet;
        typedef typename Global::NFATransMap NFATransMap;
        
    private:
        NFATransMap nfaTransMap; ///< A transition map for this state, the key is character and the value is a set of states.
        
        void getTargetStateSet(NFAStateSet& stateSet) {
            for(auto& mapPair : nfaTransMap)
                getTargetStateSetByChar(stateSet, mapPair.first);
        }
        
        void getTargetStateSetByChar(NFAStateSet& stateSet, Character character) {
            if (character == Global::epsilon) {
                getEpsilonClosure(stateSet);
                return;
            }
            NFAStateSet epsilonClosure;
            getEpsilonClosure(epsilonClosure);
            epsilonClosure.insert(this);
            for (NFAState* nfaState : epsilonClosure) {
                NFATransMap& transMap = nfaState -> getNFATransMap();
                auto mapIt = transMap.find(character);
                if (mapIt != transMap.end()) {
                    for(NFAState* state : mapIt -> second) {
                        state -> getEpsilonClosure(stateSet);
                        stateSet.insert(state);
                    }
                }
            }
        }

        void getEpsilonClosure(NFAStateSet& epsilonClosure) {
            auto mapIt = nfaTransMap.find(Global::epsilon);
            if (mapIt != nfaTransMap.end()) { 
                NFAStateSet workSet;
                for (NFAState* state : mapIt -> second)
                    if (epsilonClosure.insert(state).second) workSet.insert(state);
                for (NFAState* state : workSet)
                    state -> getEpsilonClosure(epsilonClosure);
            }
        }

    public:
        /// \brief Gets the reference of transition map for this state.
        ///
        /// responsibility to use a reference to get this map, otherwise it will call copy construction.
        /// This reference map can be used to modify.
        /// \return A map reference. 
        NFATransMap& getNFATransMap() {return nfaTransMap;}

        /// \brief Gets a const transition map for this state.
        ///
        /// This map can not be used to modify.
        /// \return A const map. 
        const NFATransMap getNFATransMap() const {return nfaTransMap;}

        /// \brief Adds a transition which label is param character and target state is param target for this state.
        ///
        /// If this state has the same transition, then do nothing and return false.
        /// Otherwise add transition and return true;
        /// The target state must be created by the same NFA with this state.
        /// \param character The label in the transition, which is a template class.
        /// \param target The target state in the transition.
        /// \return A boolean representing whether add a transition to a state successfully.
        bool addNFATrans(Character character, NFAState *target) {
            NFAStateSet& stateSet = nfaTransMap[character];
            return stateSet.insert(target).second;
        }

        /// \brief Adds a epsilon transition for this state.
        ///
        /// If this state has the same transition, then do nothing and return false.
        /// Otherwise add transition and return true;
        /// The target state must be created by the same NFA with this state.
        /// \param target The target state in the transition.
        /// \return A boolean representing whether add an epsilon transition to a state successfully.
        bool addEpsilonTrans(NFAState *target) {return addNFATrans(Global::epsilon, target);}
        
        /// \brief Deletes a transition which label is param character and target state is param target for this state.
        ///
        /// If this state has this transition, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param character The label in the transition, which is a template class.
        /// \param target The target state in the transition.
        /// Returns a boolean representing whether the transition is deleted successfully.
        bool delNFATrans(Character character, const NFAState *target) {
            auto mapIt = nfaTransMap.find(character);
            if (mapIt == nfaTransMap.end()) {
                return false;
            } else {
                NFAStateSet& stateSet = mapIt -> second;
                auto sIt = stateSet.find(target);
                if (sIt == stateSet.end()) return false;
                if (stateSet.size() == 1) {
                    nfaTransMap.erase(mapIt);
                } else {
                    stateSet.erase(sIt);
                }
                return true;
            }
        }
        
        /// \brief Deletes all transitions target to the param target for this state.
        /// 
        /// If the target state in the target states set of thie state, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param target The target state in the transition.
        /// \return A boolean representing whether the target state is deleted successfully.
        bool delNFATrans(const NFAState *target) {
            int count = 0;
            CharacterSet charSet;
            for (auto& mapPair : nfaTransMap) {
                NFAStateSet& stateSet = mapPair.second;
                auto sIt = stateSet.find(const_cast<NFAState*>(target));
                if (sIt != stateSet.end()) {
                    count++;
                    if (stateSet.size() == 1) {
                        charSet.insert(mapPair.first);
                    } else {
                        stateSet.erase(sIt);
                    }
                }
            }
            if (count == 0) return false;
            for(Character character : charSet)
                nfaTransMap.erase(character);
            return true;
        }

        /// \brief Deletes all transitions which the label is param character for this state.
        /// 
        /// If this character in the keys of transition map, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param character The label in the transition, which is a template class.
        /// \return A boolean representing whether delete all transitions with given character successfully.
        bool delNFATrans(Character character) {return nfaTransMap.erase(character);}

        /// \brief Gets a set of all the target states for this state.
        /// \return A const set of states in NFA.
        const NFAStateSet getTargetStateSet() {
            NFAStateSet stateSet;
            for (auto& mapPair : nfaTransMap)
                getTargetStateSetByChar(stateSet, mapPair.first);
            return stateSet;
        }

        /// \brief Gets a set of all the target states which from the transition that label is param character.
        ///
        /// If this state has no transition with the label param character, then return a empty set.
        /// \param character The label in a transition, which is a template class.
        /// \return A const set of states in NFA.
        const NFAStateSet getTargetStateSetByChar(Character character) {
            NFAStateSet epsilonClosure;
            getEpsilonClosure(epsilonClosure);
            if (character == Global::epsilon) return epsilonClosure;
            epsilonClosure.insert(this);
            NFAStateSet stateSet;
            for (NFAState* nfaState : epsilonClosure) {
                NFATransMap& transMap = nfaState -> getNFATransMap();
                auto mapIt = transMap.find(character);
                if (mapIt != transMap.end()) {
                    for(NFAState* state : mapIt -> second) {
                        state -> getEpsilonClosure(stateSet);
                        stateSet.insert(state);
                    }
                }
            }
            return stateSet;
        }

        /// \brief Gets a set of all the states reached by epsilon from this state.
        ///
        /// It is a closure.
        /// \return A const set of states in NFA.
        const NFAStateSet getEpsilonClosure() {
            NFAStateSet epsilonClosure;
            auto mapIt = nfaTransMap.find(Global::epsilon);
            if (mapIt == nfaTransMap.end()) return epsilonClosure;
            NFAStateSet workSet;
            for (NFAState* state : mapIt -> second)
                if (epsilonClosure.insert(state).second) workSet.insert(state);
            for (NFAState* state : workSet)
                state -> getEpsilonClosure(epsilonClosure);
            return epsilonClosure;
        }

        void output() {
            for (auto& mapPair : nfaTransMap)
                for (NFAState* state : mapPair.second)
                    cout << getID() << " " << mapPair.first << " " << state -> getID() << endl;
            }

        friend NFA<Character>;
    };
}


#endif /* NFAState_hpp */
