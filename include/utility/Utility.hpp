//
//  Utility.hpp
//  
//
//  Created by 何锦龙 on 2018/12/20.
//  Copyright © 2018年 何锦龙. All rights reserved.
//

#ifndef Utility_hpp
#define Utility_hpp

#include <unordered_set>
#include <iostream>
#include <vector>
using namespace std;

class Utility {
public:
    template<class T>
    static void Combination(vector<T>& datas, size_t len, vector<unordered_set<T> >& com) {
        sort(datas.begin(), datas.end());
        size_t n = 1 << len;
        for (size_t i = 1; i < n; i++) {
            unordered_set<T> newDatas;
            for (size_t j = 0; j < len; j++) {
                size_t temp = i;
                if (temp & (1 << j)) {
                    newDatas.insert(datas[j]);
                }
            }
            com.push_back(newDatas);
        }
    }

    template<class T>
    static void Combination(vector<T>& datas, size_t len, vector<vector<T> >& com) {
        sort(datas.begin(), datas.end());
        size_t n = 1 << len;
        for (size_t i = 1; i < n; i++) {
            vector<T> newDatas;
            for (size_t j = 0; j < len; j++) {
                size_t temp = i;
                if (temp & (1 << j)) {
                    newDatas.push_back(datas[j]);
                }
            }
            com.push_back(newDatas);
        }
    }

    template<class T>
    static void Combination(unordered_set<T>& datas, vector<unordered_set<T> >& com) {
        vector<T> dataVec(datas.begin(), datas.end());
        Combination(dataVec, com);
    }

    template<class T>
    static void Combination(vector<T>& datas, vector<unordered_set<T> >& com) {
        size_t len = datas.size();
        Combination(datas, len, com);
    }

    
    template<class T>
    static void Permutation_ALL(vector<T>& datas, size_t len, vector<vector<T> >& permit) {
        vector<vector<T> > com;
        Combination(datas, len, com);
        vector<T> n;
        for (size_t i = 0; i < com.size(); i++) {
            do {
                n.clear();
                for (size_t j = 0; j < com[i].size(); j++) {
                    n.push_back(com[i][j]);
                }
                permit.push_back(n);
            }
            while(next_permutation(com[i].begin(), com[i].end()));
        }
    }

    template<class T>
    static void Permutation(vector<T>& datas, vector<vector<T> >& permit) {
        sort(datas.begin(), datas.end());
        vector<T> n;
        do {
            n.clear();
            for (size_t j = 0; j < datas.size(); j++) {
                n.push_back(datas[j]);
            }
            permit.push_back(n);
        }
        while(next_permutation(datas.begin(), datas.end()));
    }

    template<class T>
    static void Permutation_ALL(vector<T>& datas, vector<vector<T> >& permit) {
        size_t len = datas.size();
        Permutation_ALL(datas, len, permit);
    }

    template<class T>
    static void Permutation(unordered_set<T>& datas, vector<vector<T> >& permit) {
        vector<T> dataVec(datas.begin(), datas.end());
        Permutation(dataVec, permit);
    }

    template<class T>
    static void Permutation_ALL(unordered_set<T>& datas, vector<vector<T> >& permit) {
        vector<T> dataVec(datas.begin(), datas.end());
        Permutation(dataVec, permit);
    }

    template<class T>
    static void Permutation_Order(vector<T>& datas, vector<vector<T> >& permit) {
        vector<vector<T> > com;
        vector<T> vec, n;
        for (size_t i = 1; i <= datas.size(); i++) {
            vec.clear();
            for (size_t j = 0; j < i; j++) {
                vec.push_back(datas[j]);
            }
            com.push_back(vec);
        }
        for (size_t i = 0; i < com.size(); i++) {
            do {
                n.clear();
                for (size_t j = 0; j < com[i].size(); j++) {
                    n.push_back(com[i][j]);
                }
                permit.push_back(n);
            }
            while(next_permutation(com[i].begin(), com[i].end()));
        }
    }

    template<class T>
    static void Permutation_Order_Null(vector<T>& datas, T null, vector<vector<T> >& permit) {
        vector<vector<T> > com;
        vector<T> vec;
        for (size_t i = 1; i <= datas.size(); i++) {
            vec.clear();
            for (size_t j = 0; j < datas.size(); j++) {
                if (j < i) {
                    vec.push_back(datas[j]);
                } else {
                    vec.push_back(null);
                }
            }
            sort(vec.begin(), vec.end());
            com.push_back(vec);
        }
        for (size_t i = 0; i < com.size(); i++) {
            do {
                vector<T> n;
                for (size_t j = 0; j < com[i].size(); j++) {
                    n.push_back(com[i][j]);
                }
                permit.push_back(n);
            }
            while(next_permutation(com[i].begin(), com[i].end()));
        }
    }

    static vector<string> split(const string& s, const string& seprate) {
        vector<string> res;
        size_t seprate_len = seprate.length();
        size_t start = 0;
        size_t index;
        if((index = s.find(seprate, start)) == s.npos)
        {
            res.push_back(s);
            return res;
        }
        while((index = s.find(seprate,start)) != s.npos)
        {
            res.push_back(s.substr(start,index - start));
            start = index + seprate_len;
        }
        if(start < s.length())
            res.push_back(s.substr(start,s.length()-start));
        return res;
    }

    static string ReplaceAll(string& str, const string& from, const string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); 
        }
        return str;
    }



};




#endif /* Utility_hpp */

