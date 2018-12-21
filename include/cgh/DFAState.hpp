//
//  DFAState.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/3.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef DFAState_hpp
#define DFAState_hpp

#include "State.hpp"
namespace cgh{
    template <class Character> class DFA;
    
    /// \brief States in Deterministic Finite Automaton.
    ///
    /// Example:
    ///    DFAState<char>* dfaState0 = dfa -> mkState();
    ///    DFAState<char>* dfaState1 = dfa -> mkState();
    ///    dfaState0 -> addDFATrans('a', dfaState1);
    ///    dfaState0 -> addDFATrans('b', dfaState0);
    ///    dfaState0 -> delDFATrans('a');
    ///    dfaState0 -> delDFATrans(dfaState1);
    ///    dfaState0 -> delDFATrans('b', dfaState0);
    ///    DFATransMap& dfaTransMap = dfaState0 -> getDFATransMap();
    ///    DFAStateSet dfaStateSet = dfaState0 -> getTargetSet();
    ///    DFAState* targetState = dfaState0 -> getTargetState('a');
    ///    dfa -> delDFAState(dfaState0);
    template <class Character>
    class DFAState : public State
    {
    public:
        typedef Global<Character> Global;
        typedef typename Global::CharacterSet CharacterSet;
        typedef typename Global::DFAStateSet DFAStateSet;
        typedef typename Global::DFATransMap DFATransMap;
        
    private:
        DFATransMap dfaTransMap; ///< A transition map for this state, the key is character and the value is a state.

        void getTargetStateSet(DFAStateSet& stateSet) {
            for (auto& mapPair : dfaTransMap)
                stateSet.insert(mapPair.second);
        }

    public:
        /// \brief Gets the reference of transition map for this state.
        ///
        /// responsibility to use a reference to get this map, otherwise it will call copy construction.
        /// This reference map can be used to modify.
        /// \return A map reference. 
        DFATransMap& getDFATransMap() {return dfaTransMap;}

        /// \brief Gets a const transition map for this state.
        ///
        /// This map can not be used to modify.
        /// \return A const map. 
        const DFATransMap& getDFATransMap() const {return dfaTransMap;}

        /// \brief Adds a transition which label is param character and target state is param target for this state.
        ///
        /// If this state has the a transition with the same label as pram character, then do nothing and return false.
        /// Otherwise add transition and return true;
        /// The target state must be created by the same DFA with this state.
        /// \param character The label in the transition, which is a template class.
        /// \param target The target state in the transition.
        /// \return A boolean representing whether add a transition to a state successfully.
        bool addDFATrans(Character character, DFAState *target) {
            if (dfaTransMap.count(character) != 0) return false;
            dfaTransMap[character] = target;
            return true;
        }

        /// \brief Deletes a transition which label is param character and target state is param target for this state.
        ///
        /// If this state has this transition, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param character The label in the transition, which is a template class.
        /// \param target The target state in the transition.
        /// Returns a boolean representing whether the transition is deleted successfully.
        bool delDFATrans(Character character, const DFAState *target) {
            auto mapIt = dfaTransMap.find(character);
            if (mapIt != dfaTransMap.end() && mapIt -> second == target) {
                dfaTransMap.erase(mapIt);
                return true;
            }
            return false;
        }

        /// \brief Deletes all transitions target to the param target for this state.
        /// 
        /// If the target state in the target states set of thie state, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param target The target state in the transition.
        /// \return A boolean representing whether the target state is deleted successfully.
        bool delDFATrans(const DFAState *target) {
            CharacterSet charSet;
            for (auto& mapPair : dfaTransMap)
                if (mapPair.second == target)
                    charSet.insert(mapPair.first);
            if (charSet.size() == 0) return false;
            for (Character character : charSet)
                dfaTransMap.erase(character);
            return true;
        }

        /// \brief Deletes the transition which the label is param character for this state.
        /// 
        /// If this character in the keys of transition map, then delete it and return true;
        /// Otherwise do nothing and return false;
        /// \param character The label in the transition, which is a template class.
        /// \return A boolean representing whether delete all transitions with given character successfully.
        bool delDFATrans(Character character) {return dfaTransMap.erase(character);}

        /// \brief Gets a set of all the target states for this state.
        /// \return A const set of states in DFA.
        const DFAStateSet getTargetStateSet() {
            DFAStateSet stateSet;
            for (auto& mapPair : dfaTransMap)
                stateSet.insert(mapPair.second);
            return stateSet;
        }

        /// \brief Gets the target states which from the transition that label is param character.
        ///
        /// If this state has no transition with the label param character, then return a nullptr.
        /// \param character The label in a transition, which is a template class.
        /// \return A states in DFA. 
        DFAState* getTargetStateByChar(Character character) {
            auto mapIt = dfaTransMap.find(character);
            if (mapIt != dfaTransMap.end()) return mapIt -> second;
            DFAState* null = nullptr;
            return null;
        }

        void output(){
            for (auto iter = dfaTransMap.begin(); iter != dfaTransMap.end(); iter++) {
                cout<< getID()<<" "<<iter->first<<" "<<iter->second->getID()<<endl;
            }
        }
        friend DFA<Character>;
    };
    
}

#endif /* DFAState_hpp */
