//
//  RegularExp.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/15.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef RegularExp_hpp
#define RegularExp_hpp

#include "Global.hpp"
namespace cgh {
    template <class Character> class NFA;
    template <class Character> class NFAState;
    
    
    template<class Character>
    class Char
    {
    protected:
        char type;//char is 0; opt is 1, 2, 3, 4
        NFA<Character>* nfa;
    public:
        NFA<Character>* getNFA() { return nfa; }
        void setNFA(NFA<Character>* nfa) { this -> nfa = nfa; }
        bool isOpt() { return type; }
        char getType() { return type; }
        virtual bool BinaryOpt() = 0;
        virtual bool isCatOpt() = 0;
        virtual bool isUnionOpt() = 0;
        virtual bool isLeftBracket() = 0;
        virtual bool isRightBracket() = 0;
        virtual bool isUnintOpt() = 0;
        virtual bool isStar() = 0;
        virtual bool isPlus() = 0;
        
        Char():nfa(NULL), type(0) {}
        Char(char t):nfa(NULL), type(t) {}
    };
    
    template<class Character>
    class BasicChar : public Char<Character>
    {
    private:
        int character;
    public:
        int getChar() { return character; }
        bool BinaryOpt() { return this -> isOpt() && ( character == '|' || character == 128); }
        bool isCatOpt() { return this -> isOpt() && character == 128; }
        bool isUnionOpt() { return this -> isOpt() && character == '|'; }
        bool isLeftBracket() { return this -> isOpt() && character == '('; }
        bool isRightBracket() { return this -> isOpt() && character == ')'; }
        bool isUnintOpt() { return this -> isOpt() && ( character == '*' || character == '+'); }
        bool isStar() { return this -> isOpt() && character == '*'; }
        bool isPlus() { return this -> isOpt() && character == '+'; }
        bool isQustion() { return this -> isOpt() && character == '?';}
        
        BasicChar(int c, char t):Char<Character>(t) { character = c; }
        BasicChar(int c):Char<Character>() { character = c; }
        
        void output()
        {
            if(character < 128)
                cout << (char)character << "(" << (int)this -> type << ") ";
            else
                cout << ".(" << (int)this -> type << ") ";
        }
        void output1()
        {
            if(character < 128)
                cout << (char)character ;
            else
                cout << "." ;
        }
    };
    
    
    class RegEx
    {
    protected:
        unordered_set<char> optSet;
        string regEx;
    public:
        RegEx() {}
        //        virtual void mkComplementRegEx(const string& regEx, vector<int>& res);
        //        virtual void mkPostfixEx();
        //        virtual void mkNFA(const string& regEx, NFA<char>* nfa) = 0;
        string getRegEx(){return regEx;}
        virtual bool isRegEx() = 0;
        bool isOpt(char c) { return optSet.find(c) != optSet.end(); }
        bool isUnintOpt(char c) { return c == '*' || c == '+' || c == ':'; }
        virtual bool isLeft(char c) = 0;
        virtual bool isRight(char c) = 0;
    };
    
    template<class Character>
    class BasicRegEx : public RegEx
    {
    public:
        typedef Global<Character> Global;
        typedef BasicChar<Character> BasicChar;
        typedef NFA<Character> NFA;
        typedef NFAState<Character> NFAState;
        
        typedef typename Global::NFAState2Map NFAState2Map;
    public:
        BasicRegEx()
        {
            optSet.insert('(');
            optSet.insert(')');
            optSet.insert('+');
            optSet.insert('*');
            optSet.insert('?');
            optSet.insert('|');
        }
        BasicRegEx(const string& str)
        {
            regEx = str;
            optSet.insert('(');
            optSet.insert(')');
            optSet.insert('+');
            optSet.insert('*');
            optSet.insert('?');
            optSet.insert('|');
        }
        bool isRegEx()
        {
            int count = 0;
            for(ID i = 0; i < regEx.size(); i++)
            {
                if(regEx[i] == '|')
                {
                    if(i == regEx.size() -1)
                        return false;
                    else if(isLeftOpt(regEx[i + 1]))
                        return false;
                }
                else if(regEx[i] == '(') count++;
                else if(regEx[i] == ')') count--;
                else if(isUnintOpt(regEx[i]))
                    if(i < regEx.size() -1)
                        if(isUnintOpt(regEx[i + 1]))
                            return false;
            }
            if(count != 0) return false;
            return true;
        }
        bool isLeft(char c) { return c != '|' && c != '('; }
        bool isLeftOpt(char c) { return optSet.find(c) != optSet.end() && c != '(';}
        bool isRight(char c) { return optSet.find(c) == optSet.end() || c == '('; }
        void mkComplementRegEx(vector<BasicChar*>& res)
        {
            if(!isRegEx()) return;
            ID length = regEx.length();
            for(ID i = 0; i < length; i++)
            {
                if(regEx[i] == '\\' && i < length - 1)
                {
                    res.push_back(new BasicChar(regEx[++i], 0));
                    if(i < length - 1 && isLeft(regEx[i + 1]))
                        res.push_back(new BasicChar(128, 3));
                }
                else
                {
                    if (regEx[i] == '(' || regEx[i] == ')')
                        res.push_back(new BasicChar(regEx[i], 1));
                    else if(regEx[i] == '|')
                        res.push_back(new BasicChar(regEx[i], 2));
                    else if(regEx[i] == '*' || regEx[i] == '+' || regEx[i] == '?')
                        res.push_back(new BasicChar(regEx[i], 4));
                    else
                        res.push_back(new BasicChar(regEx[i], 0));
                    if(i < length - 1 && isLeft(regEx[i]) && isRight(regEx[i + 1]))
                        res.push_back(new BasicChar(128, 3));
                }
            }
        }
        void toPostfixEx(vector<BasicChar*>& res)
        {
            vector<BasicChar*> source;
            mkComplementRegEx(source);
            if(source.size() == 1)
            {
                if(!source[0] -> isOpt())
                {
                    source.push_back(new BasicChar(128, 3));
                    source.push_back(new BasicChar(0, 0));
                }
            }
            stack<BasicChar*> stack;
            for(BasicChar* basicChar : source)
            {
                if(!basicChar -> isOpt())
                    res.push_back(basicChar);
                else if(basicChar -> isLeftBracket())
                    stack.push(basicChar);
                else if(basicChar -> isRightBracket())
                {
                    while(!stack.empty() && !stack.top() -> isLeftBracket())
                    {
                        res.push_back(stack.top());
                        stack.pop();
                    }
                    stack.pop();
                }
                else
                {
                    while(!stack.empty() && stack.top() -> getType() >= basicChar ->getType())
                    {
                        res.push_back(stack.top());
                        stack.pop();
                    }
                    stack.push(basicChar);
                }
            }
            while(!stack.empty())
            {
                res.push_back(stack.top());
                stack.pop();
            }
        }
        
        NFA* mkNFA()
        {
            vector<BasicChar*> postfix;
            toPostfixEx(postfix);
            stack<BasicChar*> stack;
            for(BasicChar* basicChar: postfix)
            {
                if(!basicChar -> isOpt())
                    stack.push(basicChar);
                else
                {
                    BasicChar* rhsChar = stack.top();
                    NFA* rhsNFA = rhsChar -> getNFA();
                    if(basicChar -> isStar())
                    {
                        if(!rhsNFA)
                        {
                            rhsNFA = new NFA();
                            NFAState* state = rhsNFA -> mkInitialState();
                            rhsNFA -> addFinalState(state);
                            state -> addNFATrans(rhsChar -> getChar(), state);
                            stack.top() -> setNFA(rhsNFA);
                        }
                        else
                        {
                            for(NFAState* state : rhsNFA -> getFinalStateSet())
                                state -> addEpsilonTrans(rhsNFA -> getInitialState());
                            rhsNFA -> addFinalState(rhsNFA -> getInitialState());
                        }
                    }
                    else if(basicChar -> isQustion())
                    {
                        if(!rhsNFA)
                        {
                            rhsNFA = new NFA();
                            NFAState* iState = rhsNFA -> mkInitialState();
                            NFAState* fState = rhsNFA -> mkFinalState();
                            rhsNFA -> addFinalState(iState);
                            iState -> addNFATrans(rhsChar -> getChar(), fState);
                            stack.top() -> setNFA(rhsNFA);
                        }
                        else
                        {
                            rhsNFA -> addFinalState(rhsNFA -> getInitialState());
                        }
                    }
                    else if(basicChar -> isPlus())
                    {
                        if(!rhsNFA)
                        {
                            rhsNFA = new NFA();
                            NFAState* iniState = rhsNFA -> mkInitialState();
                            NFAState* finState = rhsNFA -> mkFinalState();
                            iniState -> addNFATrans(rhsChar -> getChar(), finState);
                            finState -> addNFATrans(rhsChar -> getChar(), finState);
                            stack.top() -> setNFA(rhsNFA);
                        }
                        else
                        {
                            NFAState* finState = rhsNFA -> mkState();
                            NFAState2Map state2Map;
                            for(NFAState* state : rhsNFA -> getFinalStateSet())
                                state -> addEpsilonTrans(finState);
                            rhsNFA -> clearFinalStateSet();
                            rhsNFA -> addFinalState(finState);
                            state2Map[rhsNFA -> getInitialState()] = finState;
                            rhsNFA -> cpTransByNFA(rhsNFA -> getInitialState(), state2Map);
                            rhsNFA -> getFinalStateSet().erase(finState);
                            for(NFAState* state : rhsNFA -> getFinalStateSet())
                                state -> addEpsilonTrans(finState);
                        }
                    }
                    else if(basicChar -> isUnionOpt())
                    {
                        stack.pop();
                        if(!rhsNFA)
                        {
                            NFA* lhsNFA = stack.top() -> getNFA();
                            if(!lhsNFA)
                            {
                                lhsNFA = new NFA();
                                NFAState* iniState = lhsNFA -> mkInitialState();
                                NFAState* finState = lhsNFA -> mkFinalState();
                                iniState -> addNFATrans(stack.top() -> getChar(), finState);
                                iniState -> addNFATrans(rhsChar -> getChar(), finState);
                                stack.top() -> setNFA(lhsNFA);
                            }
                            else
                            {
                                lhsNFA -> getInitialState() -> addNFATrans(rhsChar -> getChar(), lhsNFA -> mkFinalState());
                            }
                        }
                        else
                        {
                            NFA* lhsNFA = stack.top() -> getNFA();
                            if(!lhsNFA)
                            {
                                lhsNFA = new NFA();
                                NFAState* state = lhsNFA -> mkInitialState();
                                state -> addNFATrans(stack.top() -> getChar(), lhsNFA -> mkFinalState());
                                NFAState2Map state2Map;
                                NFAState* iniState = rhsNFA -> getInitialState();
                                state2Map[iniState] = state;
                                if(iniState -> isFinal())
                                    lhsNFA -> addFinalState(state);
                                lhsNFA -> cpTransByNFA(iniState, state2Map);
                                stack.top() -> setNFA(lhsNFA);
                            }
                            else
                            {
                                NFAState* state = lhsNFA -> getInitialState();
                                NFAState2Map state2Map;
                                NFAState* iniState = rhsNFA -> getInitialState();
                                state2Map[iniState] = state;
                                if(iniState -> isFinal())
                                    lhsNFA -> addFinalState(state);
                                lhsNFA -> cpTransByNFA(iniState, state2Map);
                            }
                            delete rhsNFA;
                        }
                        
                    }
                    else if(basicChar -> isCatOpt())
                    {
                        stack.pop();
                        if(!rhsNFA)
                        {
                            NFA* lhsNFA = stack.top() -> getNFA();
                            if(!lhsNFA)
                            {
                                lhsNFA = new NFA();
                                NFAState* state = lhsNFA -> mkState();
                                lhsNFA -> mkInitialState() -> addNFATrans(stack.top() -> getChar(), state);
                                state -> addNFATrans(rhsChar -> getChar(), lhsNFA -> mkFinalState());
                                stack.top() -> setNFA(lhsNFA);
                            }
                            else
                            {
                                NFAState* state = lhsNFA -> mkState();
                                for(NFAState* finState : lhsNFA -> getFinalStateSet())
                                    finState -> addNFATrans(rhsChar -> getChar(), state);
                                lhsNFA -> clearFinalStateSet();
                                lhsNFA -> addFinalState(state);
                            }
                        }
                        else
                        {
                            NFA* lhsNFA = stack.top() -> getNFA();
                            if(!lhsNFA)
                            {
                                lhsNFA = new NFA();
                                NFAState* state = lhsNFA -> mkState();
                                lhsNFA -> mkInitialState() -> addNFATrans(stack.top() -> getChar(), state);
                                NFAState2Map state2Map;
                                NFAState* iniState = rhsNFA -> getInitialState();
                                state2Map[iniState] = state;
                                if(iniState -> isFinal())
                                    lhsNFA -> addFinalState(state);
                                lhsNFA -> cpTransByNFA(iniState, state2Map);
                                stack.top() -> setNFA(lhsNFA);
                            }
                            else
                            {
                                NFAState* state = lhsNFA -> mkState();
                                for(NFAState* finState : lhsNFA -> getFinalStateSet())
                                    finState -> addEpsilonTrans(state);
                                lhsNFA -> clearFinalStateSet();                                
                                NFAState2Map state2Map;
                                NFAState* iniState = rhsNFA -> getInitialState();
                                state2Map[iniState] = state;
                                if(iniState -> isFinal())
                                    lhsNFA -> addFinalState(state);
                                lhsNFA -> cpTransByNFA(iniState, state2Map);
                            }
                            delete rhsNFA;
                        }
                    }
                }
            }
            return stack.top() -> getNFA();
        }
        
    };
}

#endif /* RegularExp_hpp */


