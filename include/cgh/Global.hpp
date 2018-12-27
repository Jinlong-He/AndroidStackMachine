//
//  Global.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/3.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef Global_hpp
#define Global_hpp

#include <climits>
#include <math.h>
#include <set>
#include <list>
#include <queue>
#include <stack>
#include <regex>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
//#include <fiostream>
using namespace std;
namespace std {
    template<typename T, typename U>
    class hash<pair<T, U> >
    {
    public:
        size_t operator()(const pair<T, U> &p) const {
            return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
        }
    };
    template<typename T>
    class hash<unordered_set<T> >
    {
    public:
        size_t operator()(const unordered_set<T> &hashSet) const {
            size_t size = 0;
            for(auto item : hashSet) {
                size = size ^ std::hash<T>()(item);
            }
            return size;
        }
    };
    template<typename T>
    class hash<vector<T> >
    {
    public:
        size_t operator()(const vector<T> &vec) const {
           size_t size = 0;
            for(auto item : vec) {
                size = size ^ std::hash<T>()(item);
            }
            return size; 
        }
    };
}
namespace cgh
{
    typedef size_t ID;
    typedef char Flag;
    typedef char Size;
    
    template <class Character> class FA;
    template <class Character> class DFA;
    template <class Character> class NFA;
    template <class Character> class DFAState;
    template <class Character> class NFAState;
    template <class Character> class PDSTrans;
    template <class Character> class PopPDSTrans;
    template <class Character> class PushPDSTrans;
    template <class Character> class ReplacePDSTrans;

    /****************** PDSState ******************/

    class PDSState;
    typedef unordered_set<PDSState*> PDSStateSet;
    
    /****************** Global ******************/

    template <class Character>
    class Global
    {
        
    public:
        /***************** Character  *****************/
        
        static Character epsilon;
        typedef vector<Character> Word;
        typedef pair<Character, Character> Char2;
        typedef unordered_set<Character> CharacterSet;
        
        /***************** NFAState  *****************/
        
        typedef unordered_set<NFAState<Character>*> NFAStateSet;
        typedef unordered_map<NFAState<Character>*, NFAState<Character>*> NFAState2Map;
        typedef unordered_map<Character, NFAStateSet> NFATransMap;
        typedef unordered_map<NFAState<Character>*, DFAState<Character>*> NFAState2DFAStateMap;
        typedef unordered_map<NFAState<Character>*, NFAStateSet> NFAState2NFAStateSetMap;
        typedef unordered_map<NFAStateSet, DFAState<Character>*> NFAStateSet2DFAStateMap;
        
        /***************** DFAState  *****************/
        
        typedef pair<DFAState<Character>*, DFAState<Character>*> DFAState2;
        typedef unordered_set<DFAState<Character>*> DFAStateSet;
        typedef unordered_map<Character, DFAState<Character>*> DFATransMap;
        typedef unordered_map<DFAState<Character>*, DFAState<Character>*> DFAState2Map;
        typedef unordered_map<DFAState2, DFAState<Character>*> DFAStatePairMap;
        typedef unordered_map<DFAStateSet, DFAState<Character>*> DFAStateSetMap;
        typedef unordered_map<Character, DFAState2> Char2DFAState2Map;
        typedef unordered_map<DFAState<Character>*, NFAState<Character>*> DFAState2NFAStateMap;
        typedef unordered_map<Character, DFAStateSet> Char2DFAStateSetMap;
        typedef unordered_map<DFAState<Character>*, DFAStateSet> DFAState2DFAStateSetMap;
        
                /***************** FA  *****************/
        
        typedef list<FA<Character>*> FAList;
        typedef unordered_set<FA<Character>*> FASet;
        typedef unordered_set<DFA<Character>*> DFASet;
        typedef unordered_set<NFA<Character>*> NFASet;
        typedef unordered_map<Character, ID> Char2IDMap;
        
        /***************** PDSTrans  *****************/
        
        typedef list<PDSTrans<Character>*> PDSTransList;
        typedef list<PopPDSTrans<Character>*> PopPDSTransList;
        typedef list<PushPDSTrans<Character>*> PushPDSTransList;
        typedef list<ReplacePDSTrans<Character>*> ReplacePDSTransList;
        typedef unordered_map<PDSState*, NFAState<Character>*> PDSState2NFAStateMap;
   };
    
};
#endif /* Global_hpp */

