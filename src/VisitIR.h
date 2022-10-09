#pragma once

#include <cstring>
#include <string>
#include <map>
#include "koopa.h"

bool a[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool t[7] = {0, 0, 0, 0, 0, 0, 0};

using namespace std;

map<koopa_raw_value_t, string> mem_map;

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value);

void Visit(const koopa_raw_slice_t &slice);

// 查找一个可用的寄存器
string Find_reg()
{
    for (int i = 0; i < 7; i++)
    {
        if (t[i] == 0)
        {
            t[i] = 1;
            string ret = "t" + to_string(i);
            return ret;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        if (a[i] == 0)
        {
            a[i] = 1;
            string ret = "a" + to_string(i);
            return ret;
        }
    }
    return "out_of_limit_reg";
}

// 释放一个寄存器，让它重新变成可用
void Free_reg(string reg)
{
    if (reg[0] == 'a')
    {
        a[(reg[1] - '0')] = 0;
    }
    else
    {
        t[(reg[1] - '0')] = 0;
    }
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program)
{
    // koopa_raw_program_t
    /// Global values (global allocations only).
    // koopa_raw_slice_t values;
    /// Function definitions.
    // koopa_raw_slice_t funcs;

    // 执行一些其他的必要操作
    cout << ".text\n.globl main\n";

    // 访问所有全局变量-目前没有用
    // Visit(program.values);

    // 访问所有函数
    for (size_t i = 0; i < program.funcs.len; ++i)
    {
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t)program.funcs.buffer[i];
        Visit(func);
    }
}

// 访问函数
void Visit(const koopa_raw_function_t &func)
{
    // koopa_raw_function_data_t
    /// Type of function.
    // koopa_raw_type_t ty;
    /// Name of function.
    // const char *name;
    /// Parameters.
    // koopa_raw_slice_t params;
    /// Basic blocks, empty if is a function declaration.
    // koopa_raw_slice_t bbs;

    // 执行一些其他的必要操作
    string tmp_name(func->name);

    cout << tmp_name.substr(1, strlen(func->name)) << ":\n";

    // 访问所有基本块
    for (size_t i = 0; i < func->bbs.len; ++i)
    {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        Visit(bb);
    }
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb)
{
    // koopa_raw_basic_block_data_t
    /// Name of basic block, null if no name.
    // const char *name;
    /// Parameters.
    // koopa_raw_slice_t params;
    /// Values that this basic block is used by.
    // koopa_raw_slice_t used_by;
    /// Instructions in this basic block.
    // koopa_raw_slice_t insts;

    // 执行一些其他的必要操作
    // ...

    // 访问所有指令
    for (size_t i = 0; i < bb->insts.len; ++i)
    {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t inst = (koopa_raw_value_t)bb->insts.buffer[i];
        Visit(inst);
    }
}

// 访问指令
void Visit(const koopa_raw_value_t &value)
{
    // koopa_raw_value_data
    // koopa_raw_type_t ty;
    /// Name of value, null if no name.
    // const char *name;
    /// Values that this value is used by.
    // koopa_raw_slice_t used_by;
    /// Kind of value.
    // koopa_raw_value_kind_t kind;

    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        Visit(kind.data.ret, value);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        Visit(kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        // 访问 binary 指令
        Visit(kind.data.binary, value);
        break;
    default:
        // 其他类型暂时遇不到
        assert(false);
    }
}

// 访问指令-return
void Visit(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value)
{
    // koopa_raw_return_t
    /// Return value, null if no return value.
    // koopa_raw_value_t value;

    // 如果是常数，就li，否则mv

    if(retInst.value->kind.tag==KOOPA_RVT_INTEGER){
        cout << "li a0, " << retInst.value->kind.data.integer.value;
        cout << "\nret\n";
        return;
    }

    // 这个里面用到的寄存器也是用完就删，寄存器太珍贵了呜呜

    if (mem_map.count(retInst.value))
    {
        string ret_val = mem_map.find(retInst.value)->second;
        cout << "mv a0, " << mem_map.find(retInst.value)->second;
        cout << "\nret\n";
        Free_reg(mem_map.find(retInst.value)->second);
        mem_map.erase(retInst.value);
    }
    else
    {
        Visit(retInst.value);
        if (!mem_map.count(retInst.value))
        {
            cout << "err: can't find return val\n";
        }
        else
        {
            cout << "mv a0, " << mem_map.find(retInst.value)->second;
            cout << "\nret\n";
            Free_reg(mem_map.find(retInst.value)->second);
            mem_map.erase(retInst.value);
        }
    }
}

// 访问指令-integer
void Visit(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    // koopa_raw_integer_t：
    /// Value of integer.
    // int32_t value;
    if (intInst.value == 0)
        mem_map.insert(make_pair(super_value, "x0"));
    else
    {
        string reg = Find_reg();
        cout << "li " << reg << ", " << intInst.value << "\n";
        mem_map.insert(make_pair(super_value, reg));
    }
}

// 访问指令-binary
void Visit(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value)
{
    // typedef struct
    // {
    //     /// Operator.
    //     koopa_raw_binary_op_t op;
    //     /// Left-hand side value.
    //     koopa_raw_value_t lhs;
    //     /// Right-hand side value.
    //     koopa_raw_value_t rhs;
    // } koopa_raw_binary_t;

    string left_val;
    string right_val;

    if (mem_map.count(binaryInst.lhs))
        left_val = mem_map.find(binaryInst.lhs)->second;
    else
    {
        Visit(binaryInst.lhs);
        if (!mem_map.count(binaryInst.lhs))
            cout << "err: can't find binary val\n";
        else
            left_val = mem_map.find(binaryInst.lhs)->second;
    }

    if (mem_map.count(binaryInst.rhs))
        right_val = mem_map.find(binaryInst.rhs)->second;
    else
    {
        Visit(binaryInst.rhs);
        if (!mem_map.count(binaryInst.rhs))
            cout << "err: can't find binary val\n";
        else
            right_val = mem_map.find(binaryInst.rhs)->second;
    }

    string reg = Find_reg();

    switch (binaryInst.op)
    {
    case KOOPA_RBO_NOT_EQ:
        //先把俩东西按位异或（xor），然后看看不是0（seqz）
        cout << "xor " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "snez " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_EQ:
        //先把俩东西按位异或（xor），然后看看是0吗（seqz）
        cout << "xor " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "seqz " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_GT:
        cout << "sgt " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_LT:
        cout << "slt " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_GE:
        //就是不小于：先比一下第一个是不是小于第二个，在取一个逻辑not（和0是否一样）
        cout << "slt " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "seqz " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_LE:
        //同上，就是不大于
        cout << "sgt " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "seqz " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_ADD:
        cout << "add " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_SUB:
        cout << "sub " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_MUL:
        cout << "mul " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_DIV:
        cout << "div " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_MOD:
        cout << "rem " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_AND:
        cout << "and " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    case KOOPA_RBO_OR:
        cout << "or " << reg << ", " << left_val << ", " << right_val << "\n";
        break;
    // case KOOPA_RBO_XOR: cout << "xor "; break;
    // case KOOPA_RBO_SHL: cout << "sll "; break;
    // case KOOPA_RBO_SHR: cout << "srl "; break;
    // case KOOPA_RBO_SAR: cout << "sra "; break;
    default:
        cout << "err op:" << binaryInst.op << "\n";
        break;
    }

    mem_map.insert(make_pair(super_value, reg));

    //删掉左右值，不让他们占着寄存器
    Free_reg(left_val);
    Free_reg(right_val);
    mem_map.erase(binaryInst.lhs);
    mem_map.erase(binaryInst.rhs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice)
{
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
        }
    }
}
