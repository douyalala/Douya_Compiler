#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <deque>
#include <assert.h>
#include <map>

using namespace std;

enum VarKind
{
    var_kind_CONST,
    var_kind_VAR,
    var_kind_FUNC,
    var_kind_ERROR
};

struct VarUnion
{
    VarKind kind;
    string type;
    int def_block_id;
    union
    {
        int const_val;
        int var_is_func_param = 0;
    };
};

class Multi_Symbol_Map
{
public:
    Multi_Symbol_Map *outer_map = nullptr;
    map<string, VarUnion> symbol_map;

    void insert(string name, VarUnion var_u)
    {
        symbol_map.insert(make_pair(name, var_u));
    }

    void erase(string name)
    {
        if (symbol_map.count(name))
        {
            symbol_map.erase(name);
        }
        else
        {
            if (outer_map != nullptr)
            {
                outer_map->erase(name);
            }
        }
    }

    VarUnion find(string name)
    {
        if (symbol_map.count(name))
        {
            return symbol_map.find(name)->second;
        }
        else
        {
            if (outer_map != nullptr)
            {
                return outer_map->find(name);
            }
            else
            {
                VarUnion tmp;
                tmp.kind = var_kind_ERROR;
                return tmp;
            }
        }
    }
};
