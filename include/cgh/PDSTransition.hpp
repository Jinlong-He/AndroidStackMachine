//
//  PDSTrans.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/14.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef PDSTrans_hpp
#define PDSTrans_hpp
#include "Global.hpp"
#include "State.hpp"
namespace cgh
{
    class PDSState : public State
    { 
    public:
        PDSState() : State() {}
        //PDSState(ID id) : (this -> count)(id) {}
    };
    
    template <class Character>
    class PDSTrans
    {
    protected:
        PDSState* sourceState;
        PDSState* targetState;
        Character character;
    public:
        PDSTrans() : sourceState(nullptr), targetState(nullptr) {}
        PDSTrans(PDSState* sState, PDSState* tState, Character c) : sourceState(sState), targetState(tState), character(c) {}
        PDSState* getSourceState() { return sourceState; }
        PDSState* getTargetState() { return targetState; }
        Character getChar() { return character; }
        
        const PDSState* getSourceState() const { return sourceState; }
        const PDSState* getTargetState() const { return targetState; }
        const Character getChar() const { return character; }
    };
    
    template <class Character>
    class PopPDSTrans : public PDSTrans<Character>
    {
        typedef PDSTrans<Character> PDSTrans;
    public:
        PopPDSTrans() : PDSTrans() {}
        PopPDSTrans(PDSState* sState, PDSState* tState, Character c) : PDSTrans(sState, tState, c) {}
    };
    
    template <class Character>
    class PushPDSTrans : public PDSTrans<Character>
    {
        typedef PDSTrans<Character> PDSTrans;
        typedef typename Global<Character>::Char2 Char2;
    private:
        Char2 stack;
    public:
        PushPDSTrans() : PDSTrans() {}
        PushPDSTrans(PDSState* sState, PDSState* tState, Character c, const Char2& s) : PDSTrans(sState, tState, c), stack(s.first, s.second) {}
        Char2& getStack() { return stack; }
        const Char2& getStack() const { return stack; }
    };
    
    template <class Character>
    class ReplacePDSTrans : public PDSTrans<Character>
    {
        typedef PDSTrans<Character> PDSTrans;
    private:
        Character stack;
    public:
        ReplacePDSTrans() : PDSTrans() {}
        ReplacePDSTrans(PDSState* sState, PDSState* tState, Character c, Character s) : PDSTrans(sState, tState, c), stack(s) {}
        Character getStack() { return stack; }
        const Character getStack() const { return stack; }
    };
}


#endif /* PDSTrans_hpp */
