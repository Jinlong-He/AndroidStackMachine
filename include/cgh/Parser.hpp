//
//  Parser.hpp
//  CGH-T
//
//  Created by 何锦龙 on 2018/7/3.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef PARSER_HPP
#define PARSER_HPP
// #include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <tuple>

#include "NFA.hpp"

using namespace std;

namespace cgh {
    /**
     * Error Report Level
     */
    enum LogLevel {
        ERROR,
        DEBUG_,
        WARN,
        INFO,
    };
    
    /**
     * ErrorReport
     */
    class ErrorReport {
    public:
        /**
         * report some info, display in terminal
         * @param  info  [info string]
         * @param  level [report level, default INFO]
         * @return       []
         */
        void static report(string info, LogLevel level = INFO) {
            switch (level) {
                case ERROR:
                    cout << "ERROR: " << info << endl;
                    exit(-1);
                    break;
                case DEBUG_:
                    cout << "DEBUG: " << info << endl;
                    break;
                case WARN:
                    cout << "WARN: " << info << endl;
                    break;
                case INFO:
                    cout << "INFO: " << info << endl;
                    break;
                default:
                    break;
            }
        }
    };
    
    /**
     * Parser
     */
    template <class Character>
    class Parser {
    public:
        /**
         * constructor
         */
        Parser() { }
        /**
         * parse and get NFA<Character>
         * @return NFA<Character>
         */
        NFA<Character>* parse(string fileName);
        
    private:
        void parseComment(fstream& fin);
        void parseAlphabet(fstream& fin, unordered_set<Character>& alphabet);
        int parseStateNumber(fstream& fin);
        int parseInitialState(fstream& fin);
        void parseFinalStates(fstream& fin, unordered_set<int>& finalStates);
        void parseTransitions(fstream& fin, vector<tuple<int, Character, int> >& transitions);
        
    };
    
    
    /** implementation **/
    template<class Character>
    NFA<Character>* Parser<Character>::parse(string fileName) {
        fstream fin;
        fin.open(fileName, fstream::in);
        string info = fileName + " not found or open failed!";
        if (!fin.is_open()) {
            ErrorReport::report(info, ERROR);
            exit(-1);
        }
        
        NFA<Character>* result = new NFA<Character>();
        unordered_set<Character>& alphabet = result->getAlphabet();
        parseAlphabet(fin, alphabet);
        
        // parse state number
        int stateNumber = parseStateNumber(fin);
        
        // parse initial state
        int initialState = parseInitialState(fin);
        
        // parse final states
        unordered_set<int> finalStates;
        parseFinalStates(fin, finalStates);
        
        // parse transition info
        vector<tuple<int, Character, int> > transitions;
        parseTransitions(fin, transitions);
        
        fin.close();
        
        // construct NFA
        vector<NFAState<Character>* > stateVector;
        // update state info
        for(int pos = 0; pos < stateNumber; pos++)
        {
            if(pos == initialState) stateVector.push_back(result->mkInitialState());
            else if(finalStates.find(pos) != finalStates.end()) stateVector.push_back(result->mkFinalState());
            else stateVector.push_back(result->mkState());
        }
        if(finalStates.find(initialState) != finalStates.end())
        {
            NFAState<Character>* initState = result->getInitialState();
            initState->setFinalFlag(1);
            unordered_set<NFAState<Character>*>& finalStateSet = result->getFinalStateSet();
            finalStateSet.insert(initState);
        }
        // update transition info
        for(int i = 0; i < transitions.size(); i++)
        {
            int sourceState = get<0>(transitions[i]);
            int targetState = get<2>(transitions[i]);
            Character character = get<1>(transitions[i]);
            stateVector[sourceState]->addNFATrans(character, stateVector[targetState]);
        }
        
        return result;
    }
    
    
    /**
     * skip comment lines starting with '#'
     * @param fin input stream
     */
    template<class Character>
    void Parser<Character>::parseComment(fstream& fin) {
        string word;
        while (fin >> word) {
            if (word.find("#") == 0) {
                getline(fin, word);
            } else {
                break;
            }
        }
    }
    
    
    /**
     * parse line 2:
     *         sigma: 1 2 3 4
     * #assume line 2 is:  word: $state_list#
     * @param fin input stream
     * @param alphabet set
     */
    template<class Character>
    void Parser<Character>::parseAlphabet(fstream& fin, unordered_set<Character>& alphabet) {
        parseComment(fin);
        string line;
        getline(fin, line);
        stringstream stream;
        stream << line;
        Character ch;
        while(stream >> ch) {
            alphabet.insert(ch);
        }
    }
    
    /**
     * parse line 3:
     *         states: 5
     *     # assume line 3 is: word: $num_of_states
     * @param fin input stream
     * @return number of states
     */
    template<class Character>
    int Parser<Character>::parseStateNumber(fstream& fin) {
        parseComment(fin);
        int number;
        fin >> number;
        return number;
    }
    
    /**
     * parse line 4:
     *         initial: 0
     * # assume line 4 is: word: $initial_state
     * @param fin input stream
     * @return initial_state
     */
    template<class Character>
    int Parser<Character>::parseInitialState(fstream& fin) {
        parseComment(fin);
        int initial;
        fin >> initial;
        return initial;
    }
    
    /**
     * parse line 5:
     *         final: 3, 4
     * # assume line 5 is: word: $states_list
     * @param fin input stream
     * @param finalStates final states set
     */
    template<class Character>
    void Parser<Character>::parseFinalStates(fstream& fin, unordered_set<int>& finalStates) {
        parseComment(fin);
        string line;
        getline(fin, line);
        stringstream stream;
        stream << line;
        int state;
        while(stream >> state) {
            finalStates.insert(state);
        }
    }
    
    /**
     * parse left lines
     *         transitions lines is: 0 1 1
     *     # assume transition line is: $src_state $charater $dst_state
     * @param fin input stream
     * @param transitions transitions
     */
    template<class Character>
    void Parser<Character>::parseTransitions(fstream& fin, vector<tuple<int, Character, int> >& transitions) {
        parseComment(fin);
        int src;
        Character ch;
        int dst;
        while(fin >> src >> ch >> dst) {
            transitions.push_back(make_tuple(src, ch, dst));
        }
    }
}




#endif /* Parser_hpp */
