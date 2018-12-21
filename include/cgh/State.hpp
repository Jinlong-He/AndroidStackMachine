//
//  State.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/6/14.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef State_hpp
#define State_hpp

#include "Global.hpp"

namespace cgh{
    using namespace std;
    
    template <class Character> class DFA;
    template <class Character> class NFA;
    template <class Character> class DFAState;
    template <class Character> class NFAState;

    class State {
        static ID counter;

    protected:
        ID id; /// < The unique id for a state.
        Flag flag; /// < Records some information for this state.

        /// \brief Construction function for this class.
        ///
        /// counter is a static variable that it will plus 1 when a new state is created.
        /// counter maintain that every state has unique id globally.
        State() : id (counter++), flag(0) {}
        virtual ~State() {}

    public:
        /// \brief Sets a flag representing this state is final state or not accroding the param b.
        /// \param b If b is true then set this state to be the final state, otherwise, to be the normal state.
        void setFinalFlag(bool b) {flag = b ? (flag | 1) : (flag & ~1);}

        /// \brief Sets a flag representing this state is visited or not accroding the param b.
        /// \param b If b is true then set this state to be the visited state, otherwise, to be the normal state.
        void setVisitedFlag(bool b) {flag = b ? (flag | (1 << 1)) : (flag & ~(1 << 1));}

        /// \brief Sets a flag representing this state is valid state or not accroding the param b.
        /// \param b If b is true then set this state to be the final state, otherwise, to be the unvalid state.
        void setValidFlag(bool b) {flag = b ? (flag | (1 << 2)) : (flag & ~(1 << 2));}

        /// \brief Gets a ID that is the id of this state.
        /// \return A ID that is the id of this state.
        ID getID() const {return id;}

        /// \brief Gets a boolean that representing whether this state is a final state. 
        /// \return A boolean.
        bool isFinal() const {return (flag & 1) == 1;}

        /// \brief Gets a boolean that representing whether this state is visited. 
        /// \return A boolean.
        bool isVisited() const {return (flag & 1 << 1) == (1 << 1);}

        /// \brief Gets a boolean that representing whether this state is a valid state. 
        /// \return A boolean.
        bool isValid() const {return (flag & (1 << 2)) == (1 << 2);}
        
        friend NFA<class Character>;
        friend DFA<class Character>;
        friend DFAState<class Character>;
        friend DFAState<class Character>;
    };
}
#endif /* State_hpp */
