#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include "koopa.h"

using namespace std;

// 寄存器使用情况
bool a[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool t[7] = {0, 0, 0, 0, 0, 0, 0};
// 栈帧使用情况
deque<int> stack_frame;

// 符号表的指令和寄存器或栈或全局变量的对应关系
map<koopa_raw_value_t, string> mem_map_reg;
map<koopa_raw_value_t, int> mem_map_stack;
map<koopa_raw_value_t, string> mem_map_global;

// 用于为br分支相关基本块计数并命名
int count_if_else = 0;

// 用于为jump相关基本块计数并命名
int count_jump_Label = 0;

// 基本块和Label的对应关系
map<koopa_raw_basic_block_t, string> mem_map_block;

// 计数部分，临时栈帧使用情况和临时符号表
// 暂时没发现寄存器部分使用原版会出什么问题，所以没有单开临时寄存器数组
int stack_frame_count = 0;
map<koopa_raw_value_t, string> count_mem_map_reg;
map<koopa_raw_value_t, int> count_mem_map_stack;
set<koopa_raw_basic_block_t> count_mem_map_block;
// 计数部分，用于记录函数内是否含有call
bool count_has_call = 0;

// // 记录函数调用的时候保存的寄存器
// deque<deque<string>> save_reg;

//记录函数开的栈的大小
deque<int> save_stack_size;

//记录函数有没有保存ra
deque<bool> save_ra;

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_basic_block_t &bb, string Label);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_global_alloc_t &global_allocInst, const koopa_raw_value_t &super_value);
void Visit_alloc(const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value);

void Count_var(const koopa_raw_basic_block_t &bb);
void Count_var(const koopa_raw_value_t &value);
void Count_var(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value);
void Count_var_alloc(const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value);

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
        a[(reg[1] - '0')] = 0;
    else
        t[(reg[1] - '0')] = 0;
}

// 查找一个可用的栈位置
int Find_stack()
{
    int now_stack_pos = stack_frame.back();
    stack_frame.pop_back();
    now_stack_pos -= 4;
    stack_frame.push_back(now_stack_pos);
    // 用到的时候应该是sp + pos
    return now_stack_pos;
}

// 查找一个可用的栈位置-count环节
int Add_stack_count(int num = 1)
{
    stack_frame_count += num;
    return stack_frame_count;
}

// // 查找有多少个寄存器需要保存
// void count_Push_reg()
// {
//     deque<string> tmp;
//     for (int i = 0; i < 7; i++)
//     {
//         if (t[i] == 1)
//         {
//             string reg_name = "t" + to_string(i);
//             tmp.push_back(reg_name);
//         }
//     }
//     for (int i = 0; i < 8; i++)
//     {
//         if (a[i] == 1)
//         {
//             string reg_name = "a" + to_string(i);
//             tmp.push_back(reg_name);
//         }
//     }
//     save_reg.push_back(tmp);
// }

// // 保存寄存器
// void Push_reg()
// {
//     int stack_size = save_stack_size.back();
//     deque<string> tmp = save_reg.back();
//     int reg_n = tmp.size();
//     for (int i = 0; i < reg_n; i++)
//     {
//         string reg_name = tmp.at(i);
//         Free_reg(reg_name);
//         int pian_yi = stack_size - (i + 1) * 4;
//         cout << "sw " << reg_name << ", " << pian_yi << "(sp)" << endl;
//     }
// }

// // 真的还原寄存器
// void Pop_reg()
// {
//     deque<string> tmp = save_reg.back();
//     save_reg.pop_back();
//     int reg_n = tmp.size();
//     for (int i = 0; i < reg_n; i++)
//     {
//         string reg_name = tmp.at(i);
//         if (reg_name[0] == 'a')
//             a[(reg_name[1] - '0')] = 1;
//         else
//             t[(reg_name[1] - '0')] = 1;
//     }
// }

// // 还原寄存器-print
// void Pop_reg_print()
// {
//     int stack_size = save_stack_size.back();
//     deque<string> tmp = save_reg.back();
//     int reg_n = tmp.size();
//     for (int i = 0; i < reg_n; i++)
//     {
//         string reg_name = tmp.at(i);
//         int pian_yi = stack_size - (i + 1) * 4;
//         cout << "lw " << reg_name << ", " << pian_yi << "(sp)" << endl;
//     }
// }

// 访问 raw program
void Visit(const koopa_raw_program_t &program)
{
    // koopa_raw_program_t
    /// Global values (global allocations only).
    // koopa_raw_slice_t values;
    /// Function definitions.
    // koopa_raw_slice_t funcs;

    // 执行一些其他的必要操作

    // 访问所有全局变量

    assert(program.values.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < program.values.len; i++)
    {
        koopa_raw_value_t inst = (koopa_raw_value_t)program.values.buffer[i];
        Visit(inst);
    }

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
    if (!func->bbs.len)
        return;
    // 输出函数头头的那些东西：
    string tmp_name(func->name);
    cout << "\n.text\n.globl " << tmp_name.substr(1, strlen(func->name)) << endl;
    cout << tmp_name.substr(1, strlen(func->name)) << ":\n";

    // 准备扩展sp
    int stack_size = 0;

    // // 记录需要保存的寄存器数量和名字
    // count_Push_reg();
    // // 需要为保存寄存器分配的栈的大小
    // stack_size += (save_reg.back().size()) * 4;

    // 计算需要为变量/参数/返回值分配的栈大小
    // 清空临时栈帧和临时符号表
    count_mem_map_reg.clear();
    count_mem_map_stack.clear();
    stack_frame_count = 0;
    count_mem_map_block.clear();
    count_has_call = 0;
    // 计算所有基本块
    for (size_t i = 0; i < func->bbs.len; ++i)
    {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        Count_var(bb);
    }
    // 得到需要为变量开的栈的大小 max_count_stack*4
    stack_size += stack_frame_count * 4;
    // 如果需要存返回值
    if (count_has_call)
        stack_size += 4;

    // sp对齐到16
    if (stack_size % 16 != 0)
        stack_size += (16 - stack_size % 16);

    // 栈帧使用情况计数empty_stack_pos：表示的是现在的栈有多少个位置还空着，用于计算把变量放在什么位置
    int empty_stack_pos = stack_size;
    // 存返回值占用一个4位
    if (count_has_call)
        empty_stack_pos = empty_stack_pos - 4;
    // // 每个寄存器4位
    // empty_stack_pos = empty_stack_pos - 4 * (save_reg.back().size());
    // 剩下的可用
    // cout << stack_size << " " << empty_stack_pos << endl;
    stack_frame.push_back(empty_stack_pos);

    // 保存当前栈帧大小
    // 为return时候挪sp准备
    save_stack_size.push_back(stack_size);

    // 挪sp
    if (stack_size != 0)
    {
        if (stack_size >= -2048 && stack_size <= 2047)
            cout << "addi sp, sp, " << -stack_size << endl;
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << -stack_size << endl;
            cout << "add sp, sp, " << tmp_reg << endl;
            Free_reg(tmp_reg);
        }
    }

    // 保存ra
    if (count_has_call)
    {
        cout << "sw ra, " << (stack_size - 4) << "(sp)" << endl;
    }
    // 记录是否保存了ra
    save_ra.push_back(count_has_call);

    // // 保存寄存器
    // Push_reg();

    // 访问所有基本块
    for (size_t i = 0; i < func->bbs.len; ++i)
    {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        if (!mem_map_block.count(bb))
            Visit(bb);
    }

    // 还原ra
    save_ra.pop_back();

    // // 还原保存的寄存器
    // Pop_reg();

    // 栈大小不需要了，pop掉
    save_stack_size.pop_back();

    // 把栈帧记录pop掉
    stack_frame.pop_back();
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

// 访问基本块 并创建Label
void Visit(const koopa_raw_basic_block_t &bb, string Label)
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

    cout << endl
         << Label << ":\n";

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
    case KOOPA_RVT_GLOBAL_ALLOC:
        // 访问 global_alloc 指令
        Visit(kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_ALLOC:
        // 访问 alloc 指令
        Visit_alloc(value);
        break;
    case KOOPA_RVT_LOAD:
        // 访问 load 指令
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // 访问 store 指令
        Visit(kind.data.store, value);
        break;
    case KOOPA_RVT_BRANCH:
        // 访问 br 指令
        Visit(kind.data.branch, value);
        break;
    case KOOPA_RVT_JUMP:
        // 访问 jump 指令
        Visit(kind.data.jump, value);
        break;
    case KOOPA_RVT_CALL:
        // 访问 jump 指令
        Visit(kind.data.call, value);
        break;
    case KOOPA_RVT_FUNC_ARG_REF:
        // 访问 func_arg 指令
        Visit(kind.data.func_arg_ref, value);
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

    // 设置返回值：如果是常数，就li，否则mv
    if (retInst.value != nullptr)
    {
        if (retInst.value->kind.tag == KOOPA_RVT_INTEGER)
            cout << "li a0, " << retInst.value->kind.data.integer.value << endl;
        else
        {
            if (!(mem_map_reg.count(retInst.value) || mem_map_stack.count(retInst.value) || mem_map_global.count(retInst.value)))
                Visit(retInst.value);
            assert(mem_map_reg.count(retInst.value) || mem_map_stack.count(retInst.value) || mem_map_global.count(retInst.value));
            if (mem_map_reg.count(retInst.value))
            {
                cout << "mv a0, " << mem_map_reg.find(retInst.value)->second << endl;
                Free_reg(mem_map_reg.find(retInst.value)->second);
                mem_map_reg.erase(retInst.value);
            }
            if (mem_map_stack.count(retInst.value))
            {
                cout << "lw a0, " << mem_map_stack.find(retInst.value)->second << "(sp)" << endl;
            }
            if (mem_map_global.count(retInst.value))
            {
                cout << "la a0, " << mem_map_global.find(retInst.value)->second << endl;
                cout << "lw a0, 0(a0)\n";
            }
        }
    }

    // // 返回之前，解放保存的寄存器
    // Pop_reg_print();

    // 还原ra
    int stack_size = save_stack_size.back();
    bool has_call = save_ra.back();
    if (has_call)
    {
        cout << "lw ra, " << (stack_size - 4) << "(sp)" << endl;
    }
    // 把sp挪回去
    if (stack_size != 0)
    {
        if (stack_size >= -2048 && stack_size <= 2047)
            cout << "addi sp, sp, " << stack_size << endl;
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << stack_size << endl;
            cout << "add sp, sp, " << tmp_reg << endl;
            Free_reg(tmp_reg);
        }
    }

    cout << "ret\n";
}

// 访问指令-integer
void Visit(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    // koopa_raw_integer_t：
    /// Value of integer.
    // int32_t value;
    if (intInst.value == 0)
        mem_map_reg.insert(make_pair(super_value, "x0"));
    else
    {
        string reg = Find_reg();
        cout << "li " << reg << ", " << intInst.value << "\n";
        mem_map_reg.insert(make_pair(super_value, reg));
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

    if (!(mem_map_reg.count(binaryInst.lhs) || mem_map_stack.count(binaryInst.lhs) || mem_map_global.count(binaryInst.lhs)))
        Visit(binaryInst.lhs);
    assert(mem_map_reg.count(binaryInst.lhs) || mem_map_stack.count(binaryInst.lhs) || mem_map_global.count(binaryInst.lhs));

    if (mem_map_reg.count(binaryInst.lhs))
    {
        left_val = mem_map_reg.find(binaryInst.lhs)->second;
        mem_map_reg.erase(binaryInst.lhs);
    }
    if (mem_map_stack.count(binaryInst.lhs))
    {
        int left_pos = mem_map_stack.find(binaryInst.lhs)->second;
        left_val = Find_reg();
        cout << "lw " << left_val << ", " << left_pos << "(sp)" << endl;
    }
    if (mem_map_global.count(binaryInst.lhs))
    {
        left_val = Find_reg();
        cout << "la " << left_val << ", " << mem_map_global.find(binaryInst.lhs)->second << endl;
        cout << "lw " << left_val << ", 0(" << left_val << ")\n";
    }

    if (!(mem_map_reg.count(binaryInst.rhs) || mem_map_stack.count(binaryInst.rhs) || mem_map_global.count(binaryInst.rhs)))
        Visit(binaryInst.rhs);
    assert(mem_map_reg.count(binaryInst.rhs) || mem_map_stack.count(binaryInst.rhs) || mem_map_global.count(binaryInst.rhs));
    if (mem_map_reg.count(binaryInst.rhs))
    {
        right_val = mem_map_reg.find(binaryInst.rhs)->second;
        mem_map_reg.erase(binaryInst.rhs);
    }
    if (mem_map_stack.count(binaryInst.rhs))
    {
        int left_pos = mem_map_stack.find(binaryInst.rhs)->second;
        right_val = Find_reg();
        cout << "lw " << right_val << ", " << left_pos << "(sp)" << endl;
    }
    if (mem_map_global.count(binaryInst.rhs))
    {
        right_val = Find_reg();
        cout << "la " << right_val << ", " << mem_map_global.find(binaryInst.rhs)->second << endl;
        cout << "lw " << right_val << ", 0(" << right_val << ")\n";
    }

    Free_reg(left_val);
    Free_reg(right_val);

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

    int stack_pos = Find_stack();
    cout << "sw " << reg << ", " << stack_pos << "(sp)\n";
    Free_reg(reg);
    mem_map_stack.insert(make_pair(super_value, stack_pos));
}

// 访问指令-global_alloc
void Visit(const koopa_raw_global_alloc_t &global_allocInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Initializer.
    // koopa_raw_value_t init;
    // } koopa_raw_global_alloc_t;

    // 盲猜global_allocInst.init是INTEGER或者下面这个
    //  /// Zero initializer.
    // KOOPA_RVT_ZERO_INIT,

    string var_name(super_value->name);
    var_name = var_name.substr(1, strlen(super_value->name));

    // for (auto i = mem_map_global.begin(); i != mem_map_global.end(); i++)
    // {
    //     if (i->second == var_name)
    //     {
    //         mem_map_global.insert(make_pair(super_value, var_name));
    //         return;
    //     }
    // }

    cout << ".data\n";

    cout << ".globl " << var_name << endl;
    cout << var_name << ":\n";

    if (global_allocInst.init->kind.tag == KOOPA_RVT_INTEGER)
    {
        cout << ".word " << global_allocInst.init->kind.data.integer.value << endl;
    }
    else
    {
        cout << ".zero 4\n";
    }

    mem_map_global.insert(make_pair(super_value, var_name));
}

// 访问指令-alloc
void Visit_alloc(const koopa_raw_value_t &super_value)
{
    // 申请一块栈上的位置
    int pos = Find_stack();
    mem_map_stack.insert(make_pair(super_value, pos));
}

// 访问指令-load
void Visit(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value)
{
    // /// Source.
    // koopa_raw_value_t src;

    if (!(mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src)))
        Visit(loadInst.src);
    assert(mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src));

    if (mem_map_stack.count(loadInst.src))
    {
        // 如果load的东西是栈上的变量
        // 返回值也要存到栈上其实等于啥也没干
        // 所以直接把pair（super_value，src的pos）存进mem_map_stack就行了
        mem_map_stack.insert(make_pair(super_value, mem_map_stack.find(loadInst.src)->second));
    }
    if (mem_map_global.count(loadInst.src))
    {
        // 如果load的是全局变量
        // 先la到reg，然后sw到栈上
        string reg = Find_reg();
        int pos = Find_stack();
        cout << "la " << reg << ", " << mem_map_global.find(loadInst.src)->second << endl;
        cout << "lw " << reg << ", 0(" << reg << ")\n";
        cout << "sw " << reg << ", " << pos << "(sp)\n";
        Free_reg(reg);
        mem_map_stack.insert(make_pair(super_value, pos));
    }
}

// 访问指令-store
void Visit(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // /// Value.
    //   koopa_raw_value_t value;
    // /// Destination.
    //   koopa_raw_value_t dest;

    string value_reg;

    // 先把value弄到reg里面（也可能本来就在reg里面）
    if (!(mem_map_reg.count(storeInst.value) || mem_map_stack.count(storeInst.value) || mem_map_global.count(storeInst.value)))
        Visit(storeInst.value);
    assert(mem_map_reg.count(storeInst.value) || mem_map_stack.count(storeInst.value) || mem_map_global.count(storeInst.value));
    if (mem_map_reg.count(storeInst.value))
    {
        value_reg = mem_map_reg.find(storeInst.value)->second;
        mem_map_reg.erase(storeInst.value);
    }
    if (mem_map_stack.count(storeInst.value))
    {
        value_reg = Find_reg();
        cout << "lw " << value_reg << ", " << mem_map_stack.find(storeInst.value)->second << "(sp)" << endl;
    }
    if (mem_map_global.count(storeInst.value))
    {
        value_reg = Find_reg();
        cout << "la " << value_reg << ", " << mem_map_global.find(storeInst.value)->second << endl;
        cout << "lw " << value_reg << ", 0(" << value_reg << ")\n";
    }

    // 然后sw进dest（目前dest应该只能是栈上的变量或者全局变量）
    if (!(mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest)))
        Visit(storeInst.dest);
    assert(mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest));

    if (mem_map_stack.count(storeInst.dest))
    {
        cout << "sw " << value_reg << ", " << mem_map_stack.find(storeInst.dest)->second << "(sp)" << endl;
    }
    if (mem_map_global.count(storeInst.dest))
    {
        string tmp_reg = Find_reg();
        Free_reg(tmp_reg);
        cout << "la " << tmp_reg << ", " << mem_map_global.find(storeInst.dest)->second << endl;
        cout << "sw " << value_reg << ", 0(" << tmp_reg << ")\n";
    }

    Free_reg(value_reg);
}

// 访问指令-br
void Visit(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    //   /// Condition.
    //   koopa_raw_value_t cond;
    //   /// Target if condition is `true`.
    //   koopa_raw_basic_block_t true_bb;
    //   /// Target if condition is `false`.
    //   koopa_raw_basic_block_t false_bb;
    //   /// Arguments of `true` target..
    //   koopa_raw_slice_t true_args;
    //   /// Arguments of `false` target..
    //   koopa_raw_slice_t false_args;
    // } koopa_raw_branch_t;

    int if_else_id = count_if_else;
    count_if_else++;

    string br_cond;

    if (!(mem_map_reg.count(brInst.cond) || mem_map_stack.count(brInst.cond) || mem_map_global.count(brInst.cond)))
        Visit(brInst.cond);
    assert(mem_map_reg.count(brInst.cond) || mem_map_stack.count(brInst.cond) || mem_map_global.count(brInst.cond));
    if (mem_map_reg.count(brInst.cond))
    {
        br_cond = mem_map_reg.find(brInst.cond)->second;
        mem_map_reg.erase(brInst.cond);
    }
    if (mem_map_stack.count(brInst.cond))
    {
        int br_cond_pos = mem_map_stack.find(brInst.cond)->second;
        br_cond = Find_reg();
        cout << "lw " << br_cond << ", " << br_cond_pos << "(sp)" << endl;
    }
    if (mem_map_global.count(brInst.cond))
    {
        br_cond = Find_reg();
        cout << "la " << br_cond << ", " << mem_map_global.find(brInst.cond)->second << endl;
        cout << "lw " << br_cond << ", 0(" << br_cond << ")\n";
    }

    Free_reg(br_cond);

    string true_Label;
    string false_Label;
    bool new_true_bb = 0;
    bool new_false_bb = 0;

    if (!mem_map_block.count(brInst.true_bb))
    {
        true_Label = "if_" + to_string(if_else_id);
        new_true_bb = 1;
        mem_map_block.insert(make_pair(brInst.true_bb, true_Label));
    }
    else
        true_Label = mem_map_block.find(brInst.true_bb)->second;

    if (!mem_map_block.count(brInst.false_bb))
    {
        false_Label = "else_" + to_string(if_else_id);
        new_false_bb = 1;
        mem_map_block.insert(make_pair(brInst.false_bb, false_Label));
    }
    else
        false_Label = mem_map_block.find(brInst.false_bb)->second;

    cout << "bnez " << br_cond << ", " << true_Label << endl;
    cout << "j " << false_Label << endl;

    if (new_true_bb)
        Visit(brInst.true_bb, true_Label);
    if (new_false_bb)
        Visit(brInst.false_bb, false_Label);
}

// 访问指令-jump
void Visit(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    //     /// Target.
    //     koopa_raw_basic_block_t target;
    //     /// Arguments of target..
    //     koopa_raw_slice_t args;
    // } koopa_raw_jump_t;

    int jump_Label_id = count_jump_Label;
    count_jump_Label++;

    string target_Label;
    bool new_jm_bb = 0;

    if (!mem_map_block.count(jumpInst.target))
    {
        target_Label = "jump_" + to_string(jump_Label_id);
        new_jm_bb = 1;
        mem_map_block.insert(make_pair(jumpInst.target, target_Label));
    }
    else
        target_Label = mem_map_block.find(jumpInst.target)->second;

    cout << "j " << target_Label << endl;

    if (new_jm_bb)
        Visit(jumpInst.target, target_Label);
}

// 访问指令-call
void Visit(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Callee.
    // koopa_raw_function_t callee;
    // /// Arguments.
    // koopa_raw_slice_t args;
    // } koopa_raw_call_t;

    // TODO：
    // 盲猜koopa_raw_slice_t args里面是一大堆exp的value

    assert(callInst.args.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < callInst.args.len; i++)
    {
        koopa_raw_value_t arg_inst = (koopa_raw_value_t)callInst.args.buffer[i];
        if (!(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst) || mem_map_global.count(arg_inst)))
        {
            // 先执行call，不然其他寄存器会被弄坏
            // call回来的东西会直接存在栈上，所以不用担心返回值被弄坏
            if (arg_inst->kind.tag != KOOPA_RVT_CALL)
                continue;
            Visit(arg_inst);
        }
    }

    for (int i = 0; i < callInst.args.len; i++)
    {
        koopa_raw_value_t arg_inst = (koopa_raw_value_t)callInst.args.buffer[i];
        if (!(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst) || mem_map_global.count(arg_inst)))
            Visit(arg_inst);
        assert(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst) || mem_map_global.count(arg_inst));

        if (mem_map_reg.count(arg_inst))
        {
            Free_reg(mem_map_reg.find(arg_inst)->second);
            if (i < 8)
            {
                string reg = "a0";
                reg[1] += i;
                cout << "mv " << reg << ", " << mem_map_reg.find(arg_inst)->second << endl;
                a[i] = 1;
            }
            else
            {
                // TODO
                // 这次要往栈顶存，方便上个函数来找
                // 我的栈这样
                // ...
                // arg9
                // arg8
                // 下个函数的栈帧
                // 位置是sp+(i-8)*4
                cout << "sw " << mem_map_reg.find(arg_inst)->second << ", " << (i - 8) * 4 << "(sp)" << endl;
            }
        }
        if (mem_map_stack.count(arg_inst))
        {
            if (i < 8)
            {
                string reg = "a0";
                reg[1] += i;
                int pos = mem_map_stack.find(arg_inst)->second;
                cout << "lw " << reg << ", " << pos << "(sp)" << endl;
            }
            else
            {
                string reg = Find_reg();
                Free_reg(reg);
                int pos = mem_map_stack.find(arg_inst)->second;
                cout << "lw " << reg << ", " << pos << "(sp)" << endl;
                // 位置是sp+(i-8)*4
                cout << "sw " << reg << ", " << (i - 8) * 4 << "(sp)" << endl;
            }
        }
        if (mem_map_global.count(arg_inst))
        {
            if (i < 8)
            {
                string reg = "a0";
                reg[1] += i;
                cout << "la " << reg << ", " << mem_map_global.find(arg_inst)->second << endl;
                cout << "lw " << reg << ", 0(" << reg << ")\n";
            }
            else
            {
                string reg = Find_reg();
                Free_reg(reg);
                cout << "la " << reg << ", " << mem_map_global.find(arg_inst)->second << endl;
                cout << "lw " << reg << ", 0(" << reg << ")\n";
                // 位置是sp+(i-8)*4
                cout << "sw " << reg << ", " << (i - 8) * 4 << "(sp)" << endl;
            }
        }
    }

    string callee_name(callInst.callee->name);
    cout << "call " << callee_name.substr(1, strlen(callInst.callee->name)) << endl;

    for (int i = 0; i < callInst.args.len; i++)
    {
        if (i < 8)
            a[i] = 0;
    }

    // 存返回值-存到栈里保险一点
    int pos = Find_stack();
    cout << "sw a0, " << pos << "(sp)\n";
    mem_map_stack.insert(make_pair(super_value, pos));

    return;
}

// 访问指令-func_arg
void Visit(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Index.
    // size_t index;
    // } koopa_raw_func_arg_ref_t;

    // cout << "douya "
    //      << to_string(func_arg_refInst.index) << endl;

    if (func_arg_refInst.index < 8)
    {
        // a0~a7
        string reg = "a0";
        reg[1] += func_arg_refInst.index;
        mem_map_reg.insert(make_pair(super_value, reg));
    }
    else
    {
        // sp+这个函数的大小+(index-8)*4
        int pos = save_stack_size.back() + (func_arg_refInst.index - 8) * 4;
        mem_map_stack.insert(make_pair(super_value, pos));
    }
    return;
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

// 计算变量数量-基本块
void Count_var(const koopa_raw_basic_block_t &bb)
{
    // 计算所有指令的变量数量
    for (size_t i = 0; i < bb->insts.len; ++i)
    {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t inst = (koopa_raw_value_t)bb->insts.buffer[i];
        Count_var(inst);
    }
}

// 计算变量数量-指令
void Count_var(const koopa_raw_value_t &value)
{
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        Count_var(kind.data.ret, value);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        Count_var(kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        // 访问 binary 指令
        Count_var(kind.data.binary, value);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        // 访问 global_alloc 指令
        // Count_var(kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_ALLOC:
        // 访问 alloc 指令
        Count_var_alloc(value);
        break;
    case KOOPA_RVT_LOAD:
        // 访问 load 指令
        Count_var(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // 访问 store 指令
        Count_var(kind.data.store, value);
        break;
    case KOOPA_RVT_BRANCH:
        // 访问 br 指令
        Count_var(kind.data.branch, value);
        break;
    case KOOPA_RVT_JUMP:
        // 访问 jump 指令
        Count_var(kind.data.jump, value);
        break;
    case KOOPA_RVT_CALL:
        // 访问 call 指令
        Count_var(kind.data.call, value);
        break;
    case KOOPA_RVT_FUNC_ARG_REF:
        // 访问 func_arg 指令
        // TODO：什么是KOOPA_RVT_FUNC_ARG_REF？
        Count_var(kind.data.func_arg_ref, value);
        break;
    default:
        // 其他类型暂时遇不到
        cout << kind.tag << endl;
        assert(false);
    }
}

// 计算指令变量-return
void Count_var(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value)
{
    if (!retInst.value)
        return;

    // 如果是常数，到时候直接li，不管了
    if (retInst.value->kind.tag == KOOPA_RVT_INTEGER)
        return;

    // 如果不是，则先计算获取返回值所需要的位置
    if (!(count_mem_map_reg.count(retInst.value) || count_mem_map_stack.count(retInst.value) || mem_map_global.count(retInst.value)))
        Count_var(retInst.value);
    assert(count_mem_map_reg.count(retInst.value) || count_mem_map_stack.count(retInst.value) || mem_map_global.count(retInst.value));

    // 用完了这个返回值就可以把它从寄存器里去掉了
    if (count_mem_map_reg.count(retInst.value))
    {
        Free_reg(count_mem_map_reg.find(retInst.value)->second);
        count_mem_map_reg.erase(retInst.value);
    }
}

// 计算指令变量-integer
void Count_var(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    string reg = Find_reg();
    count_mem_map_reg.insert(make_pair(super_value, reg));
    return;
}

// 计算指令变量-binary
void Count_var(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value)
{
    if (!(count_mem_map_reg.count(binaryInst.lhs) || count_mem_map_stack.count(binaryInst.lhs) || mem_map_global.count(binaryInst.lhs)))
        Count_var(binaryInst.lhs);
    assert(count_mem_map_reg.count(binaryInst.lhs) || count_mem_map_stack.count(binaryInst.lhs) || mem_map_global.count(binaryInst.lhs));

    if (!(count_mem_map_reg.count(binaryInst.rhs) || count_mem_map_stack.count(binaryInst.rhs) || mem_map_global.count(binaryInst.rhs)))
        Count_var(binaryInst.rhs);
    assert(count_mem_map_reg.count(binaryInst.rhs) || count_mem_map_stack.count(binaryInst.rhs) || mem_map_global.count(binaryInst.rhs));

    int stack_pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, stack_pos));

    //删掉左右值
    if (count_mem_map_reg.count(binaryInst.lhs))
    {
        Free_reg(count_mem_map_reg.find(binaryInst.lhs)->second);
        count_mem_map_reg.erase(binaryInst.lhs);
    }
    if (count_mem_map_reg.count(binaryInst.rhs))
    {
        Free_reg(count_mem_map_reg.find(binaryInst.rhs)->second);
        count_mem_map_reg.erase(binaryInst.rhs);
    }
}

// 计算指令变量-alloc
void Count_var_alloc(const koopa_raw_value_t &super_value)
{
    int pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, pos));
}

// 计算指令变量-load
void Count_var(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value)
{
    if (!(count_mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src)))
        Count_var(loadInst.src);
    assert(count_mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src));

    if (count_mem_map_stack.count(loadInst.src))
    {
        count_mem_map_stack.insert(make_pair(super_value, count_mem_map_stack.find(loadInst.src)->second));
    }
    if (mem_map_global.count(loadInst.src))
    {
        int stack_pos = Add_stack_count();
        count_mem_map_stack.insert(make_pair(super_value, stack_pos));
    }
}

// 计算指令变量-store
void Count_var(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // 先把value弄到reg里面（也可能本来就在reg里面）
    if (!(count_mem_map_reg.count(storeInst.value) || count_mem_map_stack.count(storeInst.value) || mem_map_global.count(storeInst.value)))
        Count_var(storeInst.value);
    assert(count_mem_map_reg.count(storeInst.value) || count_mem_map_stack.count(storeInst.value) || mem_map_global.count(storeInst.value));

    if (count_mem_map_reg.count(storeInst.value))
    {
        Free_reg(count_mem_map_reg.find(storeInst.value)->second);
        count_mem_map_reg.erase(storeInst.value);
    }

    // 然后sw进dest（目前dest应该只能是栈上的变量或者全局变量）
    if (!(count_mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest)))
        Count_var(storeInst.dest);
    assert(count_mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest));
}

// 访问指令-br
void Count_var(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value)
{
    // cond
    if (!(count_mem_map_reg.count(brInst.cond) || count_mem_map_stack.count(brInst.cond) || mem_map_global.count(brInst.cond)))
        Count_var(brInst.cond);

    //删掉cond
    if (count_mem_map_reg.count(brInst.cond))
    {
        Free_reg(count_mem_map_reg.find(brInst.cond)->second);
        count_mem_map_reg.erase(brInst.cond);
    }

    // true/false_bb
    if (!count_mem_map_block.count(brInst.true_bb))
    {
        count_mem_map_block.insert(brInst.true_bb);
        Count_var(brInst.true_bb);
    }

    if (!count_mem_map_block.count(brInst.false_bb))
    {
        count_mem_map_block.insert(brInst.false_bb);
        Count_var(brInst.false_bb);
    }
}

// 访问指令-jump
void Count_var(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value)
{
    return;
}

// 访问指令-call
void Count_var(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Callee.
    // koopa_raw_function_t callee;
    // /// Arguments.
    // koopa_raw_slice_t args;
    // } koopa_raw_call_t;

    count_has_call = 1;
    if (callInst.args.len > 8)
        Add_stack_count(callInst.args.len - 8);

    //返回值也需要一个位置存（不一定有，但无所谓先存了再说）
    int pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, pos));

    return;
}

// 访问指令-func_arg
void Count_var(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value)
{
    if (func_arg_refInst.index < 8)
    {
        // a0~a7
        string reg = "a0";
        reg[1] += func_arg_refInst.index;
        count_mem_map_reg.insert(make_pair(super_value, reg));
    }
    else
    {
        // 在上一个函数的栈里面，反正就是一个位置，随便给他加进去
        count_mem_map_stack.insert(make_pair(super_value, 0));
    }
    return;
}
