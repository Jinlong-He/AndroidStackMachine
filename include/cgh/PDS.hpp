//
//  PDS.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/14.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef PDS_hpp
#define PDS_hpp

#include "PDSTransition.hpp"

namespace cgh
{
    template <class Character>
    class PDS
    {
        typedef Global<Character> Global;
        typedef PopPDSTrans<Character> PopPDSTrans;
        typedef PushPDSTrans<Character> PushPDSTrans;
        typedef ReplacePDSTrans<Character> ReplacePDSTrans;
        
        typedef typename Global::Char2 Char2;
        typedef typename Global::NFAStateSet NFAStateSet;
        typedef typename Global::PDSTransList PDSTransList;
        typedef typename Global::PopPDSTransList PopPDSTransList;
        typedef typename Global::PushPDSTransList PushPDSTransList;
        typedef typename Global::ReplacePDSTransList ReplacePDSTransList;
    private:
        PDSStateSet stateSet;                   ///< The set of states for this PDS.
        PDSStateSet controlStateSet;            ///< The set of control states for this PDS.
        PopPDSTransList popTransList;           ///< The list of pop transitions for this PDS.
        PushPDSTransList pushTransList;         ///< The list of push transitions for this PDS.
        ReplacePDSTransList replaceTransList;   ///< The list of replace transitions for this PDS.
    public:
        /// \brief Default construction function.
        PDS() {}

        /// \brief Desconstruction function.
        ///
        /// delete all states in the stateSet and all transtions.
        ~PDS() {
            for (PDSState* state : stateSet) {
                delete state;
            }
            for (PopPDSTrans* trans : popTransList) {
                delete trans;
            }
            for (PushPDSTrans* trans : pushTransList) {
                delete trans;
            }
            for (ReplacePDSTrans* trans : replaceTransList) {
                delete trans;
            }
        }

        PDSStateSet& getControlStateSet() { return controlStateSet; }
        PDSStateSet& getStateSet() { return stateSet; }
        PopPDSTransList& getPopTransList() { return popTransList;}
        PushPDSTransList& getPushTransList() { return pushTransList; }
        ReplacePDSTransList& getReplaceTransList() { return replaceTransList; }
        
        const PDSStateSet& getControlStateSet() const { return controlStateSet; }
        const PDSStateSet& getStateSet() const { return stateSet; }
        const PopPDSTransList& getPopTransList() const { return popTransList;}
        const PushPDSTransList& getPushTransList() const { return pushTransList; }
        const ReplacePDSTransList& getReplaceTransList() const { return replaceTransList; }

        /// \brief Make a state for this PDS.
        ///
        /// \return A PDSState pointer.
        PDSState* mkState() {
            PDSState* state = new PDSState();
            stateSet.insert(state);
            return state;
        }

        /// \brief Make a control state for this PDS.
        ///
        /// \return A PDSState pointer.
        PDSState* mkControlState() {
            PDSState* state = mkState();
            controlStateSet.insert(state);
            return state;
        }

        ///// \brief Make a state with param id for this PDS.
        ///// \param id The id for this state.
        ///// \return A PDSState pointer.
        //PDSState* mkState(ID id) {
        //    PDSState* state = new PDSState(id);
        //    stateSet.insert(state);
        //    return state;
        //}

        ///// \brief Make a control state for this PDS.
        ///// \param id The id for this state.
        ///// \return A PDSState pointer.
        //PDSState* mkControlState(ID id) {
        //    PDSState state = mkState(id);
        //    controlStateSet.insert(state);
        //    return state;
        //}

        /// \brief Make a popPDSTrans for this PDS.
        /// \param souceState The source state in a popPDSTrans.
        /// \param targetState The target state in a popPDSTrans.
        /// \param character The source character in a popPDSTrans.
        /// \return A PopPDSTrans pointer.
        PopPDSTrans* mkPopPDSTrans(PDSState* sourceState, PDSState* targetState, Character character) {
            PopPDSTrans* pdsTrans = new PopPDSTrans(sourceState, targetState, character);
            popTransList.push_back(pdsTrans);
            return pdsTrans;
        }

        /// \brief Make a pushPDSTrans for this PDS.
        /// \param souceState The source state in a pushPDSTrans.
        /// \param targetState The target state in a pushPDSTrans.
        /// \param character The source character in a pushPDSTrans.
        /// \return A PushPDSTrans pointer.
        PushPDSTrans* mkPushPDSTrans(PDSState* sourceState, PDSState* targetState, Character character, const Char2& stack) {
            PushPDSTrans* pdsTrans = new PushPDSTrans(sourceState, targetState, character, stack);
            pushTransList.push_back(pdsTrans);
            return pdsTrans;
        }

        /// \brief Make a replacePDSTrans for this PDS.
        /// \param souceState The source state in a replacePDSTrans.
        /// \param targetState The target state in a replacePDSTrans.
        /// \param character The source character in a replacePDSTrans.
        /// \return A ReplacePDSTrans pointer.
        ReplacePDSTrans* mkReplacePDSTrans(PDSState* sourceState, PDSState* targetState, Character character, Character stack) {
            ReplacePDSTrans* pdsTrans = new ReplacePDSTrans(sourceState, targetState, character, stack);
            replaceTransList.push_back(pdsTrans);
            return pdsTrans;
        }
    };
}

#endif /* PDS_hpp */
