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

// reg
bool a[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool t[7] = {0, 0, 0, 0, 0, 0, 0};
// stack
deque<int> stack_frame;

// Var in symbol map
enum var_tag
{
    Var_in_Reg,           // a i32 var in reg
    Var_in_Stack_pos_int, // a var in stack at pos(sp)
    // alloc应该在stack上开一块空间pos1存变量，然后开一块空间pos2存pos1+sp当作指针，返回指针的pos
    Var_in_Global, // a pointer, point to a global var
};
struct Var
{
    var_tag tag;
    string name; // reg_name or global_name
    int pos;     // stack_pos
};

// symbol map : < inst , var >
map<koopa_raw_value_t, Var> risc_symbol_map;

// count and name br bb
int count_if_else = 0;

// count and name br bb
int count_jump_Label = 0;

// bb - Label
map<koopa_raw_basic_block_t, string> mem_map_block;

// count stack size - symbol map : <inst , int>
int stack_frame_count = 0;
map<koopa_raw_value_t, int> count_symbol_map;
set<koopa_raw_basic_block_t> count_mem_map_block;
// count stack size - if func has call inst
bool count_has_call = 0;

// stack size
deque<int> save_stack_size;

// if func push ra in stack
deque<bool> save_ra;

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
void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_get_ptr_t &get_ptrInst, const koopa_raw_value_t &super_value);
void Visit(const koopa_raw_aggregate_t &aggregateInst, const koopa_raw_value_t &super_value);

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
void Count_var(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value);
void Count_var(const koopa_raw_get_ptr_t &get_ptrInst, const koopa_raw_value_t &super_value);

void Visit(const koopa_raw_slice_t &slice);

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

void Free_reg(string reg)
{
    if (reg[0] == 'a')
        a[(reg[1] - '0')] = 0;
    else if (reg[0] == 't')
        t[(reg[1] - '0')] = 0;
}

int Find_stack()
{
    int now_stack_pos = stack_frame.back();
    stack_frame.pop_back();
    now_stack_pos -= 4;
    stack_frame.push_back(now_stack_pos);
    // 用到的时候应该是sp + pos
    return now_stack_pos;
}

int Add_stack_count(int num = 1)
{
    stack_frame_count += num;
    return stack_frame_count;
}

int count_type_size(koopa_raw_type_t ty)
{
    switch (ty->tag)
    {
    case KOOPA_RTT_INT32:
        return 4;
        break;
    case KOOPA_RTT_UNIT:
        return 4;
        break;
    case KOOPA_RTT_ARRAY:
        return count_type_size(ty->data.array.base) * ty->data.array.len;
        break;
    case KOOPA_RTT_POINTER:
        return 4;
        break;
    case KOOPA_RTT_FUNCTION:
        // can't count this
    default:
        assert(false);
    }
}

int count_get_elem_ptr_len(koopa_raw_type_t ty)
{
    assert(ty->tag == KOOPA_RTT_POINTER);
    assert(ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
    return count_type_size(ty->data.pointer.base->data.array.base);
}

int count_get_ptr_len(koopa_raw_type_t ty)
{
    assert(ty->tag == KOOPA_RTT_POINTER);
    return count_type_size(ty->data.pointer.base);
}

// visit raw program
void Visit(const koopa_raw_program_t &program)
{
    // koopa_raw_program_t
    /// Global values (global allocations only).
    // koopa_raw_slice_t values;
    /// Function definitions.
    // koopa_raw_slice_t funcs;

    // visit global value
    assert(program.values.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < program.values.len; i++)
    {
        koopa_raw_value_t inst = (koopa_raw_value_t)program.values.buffer[i];
        Visit(inst);
    }

    // visit func
    for (size_t i = 0; i < program.funcs.len; ++i)
    {
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t)program.funcs.buffer[i];
        Visit(func);
    }
}

// visit func
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

    // if this func is a declaration
    if (!func->bbs.len)
        return;

    string tmp_name(func->name);
    cout << "\n.text\n.globl " << tmp_name.substr(1, strlen(func->name)) << endl;
    cout << tmp_name.substr(1, strlen(func->name)) << ":\n";

    int stack_size = 0;

    // count siack size - var/func param/func return value
    count_symbol_map.clear();
    stack_frame_count = 0;
    count_mem_map_block.clear();
    count_has_call = 0;
    for (size_t i = 0; i < func->bbs.len; ++i)
    {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        Count_var(bb);
    }

    // stack_frame_count*4
    stack_size += stack_frame_count * 4;

    // if need to push ra
    if (count_has_call)
        stack_size += 4;

    // sp align 16
    if (stack_size % 16 != 0)
        stack_size += (16 - stack_size % 16);

    // empty_stack_pos：now which stack pos can be used
    int empty_stack_pos = stack_size;

    // need 4 to push ra
    if (count_has_call)
        empty_stack_pos = empty_stack_pos - 4;

    stack_frame.push_back(empty_stack_pos);

    // push stack size
    save_stack_size.push_back(stack_size);

    // move sp
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

    // push ra
    if (count_has_call)
    {
        if (stack_size - 4 < 2048)
        {
            cout << "sw ra, " << (stack_size - 4) << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << (stack_size - 4) << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "sw ra, 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    save_ra.push_back(count_has_call);

    // visit bb
    for (size_t i = 0; i < func->bbs.len; ++i)
    {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        if (!mem_map_block.count(bb))
            Visit(bb);
    }

    // pop save_ra
    save_ra.pop_back();

    // pop stack size
    save_stack_size.pop_back();

    // pop stack pos count
    stack_frame.pop_back();
}

// visit bb
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

    // visit value
    for (size_t i = 0; i < bb->insts.len; ++i)
    {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t inst = (koopa_raw_value_t)bb->insts.buffer[i];
        Visit(inst);
    }
}

// visit bb, creat Label
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

    cout << endl
         << Label << ":\n";

    // visit value
    for (size_t i = 0; i < bb->insts.len; ++i)
    {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t inst = (koopa_raw_value_t)bb->insts.buffer[i];
        Visit(inst);
    }
}

// visit value
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

    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // visit return
        Visit(kind.data.ret, value);
        break;
    case KOOPA_RVT_INTEGER:
        // visit integer
        Visit(kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        // visit binary
        Visit(kind.data.binary, value);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        // visit global_alloc
        Visit(kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_ALLOC:
        // visit alloc
        Visit_alloc(value);
        break;
    case KOOPA_RVT_LOAD:
        // visit load
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // visit store
        Visit(kind.data.store, value);
        break;
    case KOOPA_RVT_BRANCH:
        // visit br
        Visit(kind.data.branch, value);
        break;
    case KOOPA_RVT_JUMP:
        // visit jump
        Visit(kind.data.jump, value);
        break;
    case KOOPA_RVT_CALL:
        // visit jump
        Visit(kind.data.call, value);
        break;
    case KOOPA_RVT_AGGREGATE:
        Visit(kind.data.aggregate, value);
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        Visit(kind.data.get_elem_ptr, value);
        break;
    case KOOPA_RVT_GET_PTR:
        Visit(kind.data.get_ptr, value);
        break;
    case KOOPA_RVT_FUNC_ARG_REF:
        // visit func_arg
        Visit(kind.data.func_arg_ref, value);
        break;
    default:
        assert(false);
    }
}

// visit value - return
void Visit(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value)
{
    // koopa_raw_return_t
    /// Return value, null if no return value.
    // koopa_raw_value_t value;

    // if const li, else mv
    if (retInst.value != nullptr)
    {
        if (retInst.value->kind.tag == KOOPA_RVT_INTEGER)
            cout << "li a0, " << retInst.value->kind.data.integer.value << endl;
        else
        {
            if (!risc_symbol_map.count(retInst.value))
                Visit(retInst.value);
            assert(risc_symbol_map.count(retInst.value));
            Var ret_var = risc_symbol_map.find(retInst.value)->second;

            if (ret_var.tag == Var_in_Reg)
            {
                cout << "mv a0, " << ret_var.name << endl;
                Free_reg(ret_var.name);
                risc_symbol_map.erase(retInst.value);
            }
            else if (ret_var.tag == Var_in_Stack_pos_int)
            {
                if (ret_var.pos < 2048)
                {
                    cout << "lw a0, " << ret_var.pos << "(sp)" << endl;
                }
                else
                {
                    string tmp_reg = Find_reg();
                    cout << "li " << tmp_reg << ", " << ret_var.pos << endl;
                    cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
                    cout << "lw a0, 0(" + tmp_reg + ")" << endl;
                    Free_reg(tmp_reg);
                }
            }
            else
            {
                // func don't return pointer
                assert(false);
            }
        }
    }

    // pop ra
    int stack_size = save_stack_size.back();
    bool has_call = save_ra.back();
    if (has_call)
    {
        if ((stack_size - 4) < 2048)
        {
            cout << "lw ra, " << (stack_size - 4) << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << (stack_size - 4) << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw ra, 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }

    // move sp back
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

// visit value - integer
void Visit(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    // koopa_raw_integer_t：
    /// Value of integer.
    // int32_t value;

    Var new_var;
    new_var.tag = Var_in_Reg;

    if (intInst.value == 0)
    {
        new_var.name = "x0";
    }
    else
    {
        string reg = Find_reg();
        new_var.name = reg;
        cout << "li " << reg << ", " << intInst.value << "\n";
    }

    // cout << "insert: " << intInst.value << " " << new_var.name;
    risc_symbol_map.insert(make_pair(super_value, new_var));
}

// visit value - binary
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

    if (!risc_symbol_map.count(binaryInst.lhs))
        Visit(binaryInst.lhs);
    assert(risc_symbol_map.count(binaryInst.lhs));

    Var left_var = risc_symbol_map.find(binaryInst.lhs)->second;
    // cout << "left_var: " << left_var.tag << " " << left_var.name << " " << left_var.pos << endl;

    if (left_var.tag == Var_in_Reg)
    {
        left_val = left_var.name;
        risc_symbol_map.erase(binaryInst.lhs);
    }
    else if (left_var.tag == Var_in_Stack_pos_int)
    {
        int left_pos = left_var.pos;
        left_val = Find_reg();
        if (left_pos < 2048)
        {
            cout << "lw " << left_val << ", " << left_pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << left_pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + left_val + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        // pointer can't calculate here
        assert(false);
    }

    if (!risc_symbol_map.count(binaryInst.rhs))
        Visit(binaryInst.rhs);
    assert(risc_symbol_map.count(binaryInst.rhs));

    Var right_var = risc_symbol_map.find(binaryInst.rhs)->second;
    // cout << "right_var: " << right_var.tag << " " << right_var.name << " " << right_var.pos << endl;

    if (right_var.tag == Var_in_Reg)
    {
        right_val = right_var.name;
        risc_symbol_map.erase(binaryInst.rhs);
    }
    else if (right_var.tag == Var_in_Stack_pos_int)
    {
        int right_pos = right_var.pos;
        right_val = Find_reg();
        if (right_pos < 2048)
        {
            cout << "lw " << right_val << ", " << right_pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << right_pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + right_val + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        // pointer can't calculate here
        assert(false);
    }

    Free_reg(left_val);
    Free_reg(right_val);

    string reg = Find_reg();

    switch (binaryInst.op)
    {
    case KOOPA_RBO_NOT_EQ:
        // xor, if not 0 (snez)
        cout << "xor " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "snez " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_EQ:
        // xor, if is 0 (seqz)
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
        // not slt
        cout << "slt " << reg << ", " << left_val << ", " << right_val << "\n";
        cout << "seqz " << reg << ", " << reg << "\n";
        break;
    case KOOPA_RBO_LE:
        // not sgt
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
    if (stack_pos < 2048)
    {
        cout << "sw " << reg << ", " << stack_pos << "(sp)\n";
    }
    else
    {
        string tmp_reg = Find_reg();
        cout << "li " << tmp_reg << ", " << stack_pos << endl;
        cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
        cout << "sw " + reg + ", 0(" + tmp_reg + ")" << endl;
        Free_reg(tmp_reg);
    }

    Free_reg(reg);
    Var new_var;
    new_var.pos = stack_pos;
    new_var.tag = Var_in_Stack_pos_int;
    risc_symbol_map.insert(make_pair(super_value, new_var));
}

// visit value - global_alloc
void Visit(const koopa_raw_global_alloc_t &global_allocInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Initializer.
    // koopa_raw_value_t init;
    // } koopa_raw_global_alloc_t;

    // I guess global_allocInst.init is
    // 1. integer
    // 2. Zero initializer: KOOPA_RVT_ZERO_INIT
    // 3. aggregate

    string var_name(super_value->name);
    var_name = var_name.substr(1, strlen(super_value->name));

    cout << ".data\n";

    cout << ".globl " << var_name << endl;
    cout << var_name << ":\n";

    if (global_allocInst.init->kind.tag == KOOPA_RVT_INTEGER)
        cout << ".word " << global_allocInst.init->kind.data.integer.value << endl;
    else if (global_allocInst.init->kind.tag == KOOPA_RVT_AGGREGATE)
        Visit(global_allocInst.init);
    else
    {
        // cerr << super_value->ty->data.pointer.base->tag << endl;
        cout << ".zero " << count_type_size(super_value->ty->data.pointer.base) << "\n";
    }

    Var new_var;
    new_var.name = var_name;
    new_var.tag = Var_in_Global;
    risc_symbol_map.insert(make_pair(super_value, new_var));
}

// visit value - alloc
void Visit_alloc(const koopa_raw_value_t &super_value)
{
    int alloc_size = count_type_size(super_value->ty->data.pointer.base);

    // 数组a的位置在pos(sp)，则a[i]在(pos+4*i)(sp)
    int pos = 0;
    for (int i = 0; i < alloc_size / 4; i++)
    {
        pos = Find_stack();
    }

    // alloc 返回的是指针
    // stack[pointer_pos(sp)] = pos + sp

    int pointer_pos = Find_stack();
    string tmp_reg = Find_reg();

    // pos+sp
    cout << "li " << tmp_reg << ", " << pos << endl;
    cout << "add " << tmp_reg << ", " << tmp_reg << ", sp" << endl;

    // pos+sp -> stack[pointer(sp)]
    if (pointer_pos < 2048)
    {
        cout << "sw " << tmp_reg << ", " << pointer_pos << "(sp)\n";
    }
    else
    {
        string tmp_reg_2 = Find_reg();
        cout << "li " << tmp_reg_2 << ", " << pointer_pos << endl;
        cout << "add " + tmp_reg_2 + ", sp, " << tmp_reg_2 << endl;
        cout << "sw " + tmp_reg + ", 0(" + tmp_reg_2 + ")" << endl;
        Free_reg(tmp_reg_2);
    }

    Free_reg(tmp_reg);

    Var new_var;
    new_var.pos = pointer_pos;
    new_var.tag = Var_in_Stack_pos_int;
    risc_symbol_map.insert(make_pair(super_value, new_var));
}

// visit value - load
void Visit(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value)
{
    // /// Source.
    // koopa_raw_value_t src;

    if (!(risc_symbol_map.count(loadInst.src)))
        Visit(loadInst.src);
    assert(risc_symbol_map.count(loadInst.src));

    Var src_var = risc_symbol_map.find(loadInst.src)->second;

    if (src_var.tag == Var_in_Stack_pos_int)
    {
        // M(stack[pos+sp]) -> new_pos(sp)
        // stack[pos+sp]
        string reg = Find_reg();

        if (src_var.pos < 2048)
        {
            cout << "lw " << reg << ", " << src_var.pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << src_var.pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
        // M(stack[pos+sp]) -> reg
        cout << "lw " << reg << ", 0(" << reg << ")\n";

        // reg -> new_pos(sp)
        int pos = Find_stack();

        if (pos < 2048)
        {
            cout << "sw " << reg << ", " << pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "sw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }

        Free_reg(reg);

        Var new_var;
        new_var.pos = pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else if (src_var.tag == Var_in_Global)
    {
        string reg = Find_reg();
        int pos = Find_stack();
        cout << "la " << reg << ", " << src_var.name << endl;
        cout << "lw " << reg << ", 0(" << reg << ")\n";

        if (pos < 2048)
        {
            cout << "sw " << reg << ", " << pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "sw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }

        Free_reg(reg);
        Var new_var;
        new_var.pos = pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else
    {
        // TODO: var in reg can't be a pointer?
        assert(false);
    }
}

// visit value - store
void Visit(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // /// Value.
    //   koopa_raw_value_t value;
    // /// Destination.
    //   koopa_raw_value_t dest;

    // put value in reg
    string value_reg;
    if (!risc_symbol_map.count(storeInst.value))
        Visit(storeInst.value);
    assert(risc_symbol_map.count(storeInst.value));

    Var put_value = risc_symbol_map.find(storeInst.value)->second;
    // cout << "put_value: " << put_value.tag << " " << put_value.name<<endl;

    if (put_value.tag == Var_in_Reg)
    {
        value_reg = put_value.name;
        risc_symbol_map.erase(storeInst.value);
    }
    else if (put_value.tag == Var_in_Stack_pos_int)
    {
        value_reg = Find_reg();
        if (put_value.pos < 2048)
        {
            cout << "lw " << value_reg << ", " << put_value.pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << put_value.pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + value_reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        // global need load
        assert(false);
    }

    // sw reg to dest
    if (!risc_symbol_map.count(storeInst.dest))
        Visit(storeInst.dest);
    assert(risc_symbol_map.count(storeInst.dest));

    Var dest_var = risc_symbol_map.find(storeInst.dest)->second;

    if (dest_var.tag == Var_in_Stack_pos_int)
    {
        // val -> M(stack[dest.pos+sp])
        // stack[pos+sp]
        string reg = Find_reg();
        if (dest_var.pos < 2048)
        {
            cout << "lw " << reg << ", " << dest_var.pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << dest_var.pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
        // val -> M(stack[pos+sp])
        cout << "sw " << value_reg << ", 0(" << reg << ")\n";
        Free_reg(reg);
    }
    else if (dest_var.tag == Var_in_Global)
    {
        string tmp_reg = Find_reg();
        Free_reg(tmp_reg);
        cout << "la " << tmp_reg << ", " << dest_var.name << endl;
        cout << "sw " << value_reg << ", 0(" << tmp_reg << ")\n";
    }
    else
    {
        // can't store to a reg
        assert(false);
    }

    Free_reg(value_reg);
}

// visit value - br
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

    if (!risc_symbol_map.count(brInst.cond))
        Visit(brInst.cond);
    assert(risc_symbol_map.count(brInst.cond));

    Var cond_var = risc_symbol_map.find(brInst.cond)->second;

    if (cond_var.tag == Var_in_Reg)
    {
        br_cond = cond_var.name;
        risc_symbol_map.erase(brInst.cond);
    }
    else if (cond_var.tag == Var_in_Stack_pos_int)
    {
        int br_cond_pos = cond_var.pos;
        br_cond = Find_reg();
        if (br_cond_pos < 2048)
        {
            cout << "lw " << br_cond << ", " << br_cond_pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << br_cond_pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + br_cond + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        assert(false);
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

// visit value - jump
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

// visit value - call
void Visit(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Callee.
    // koopa_raw_function_t callee;
    // /// Arguments.
    // koopa_raw_slice_t args;
    // } koopa_raw_call_t;

    assert(callInst.args.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < callInst.args.len; i++)
    {
        koopa_raw_value_t arg_inst = (koopa_raw_value_t)callInst.args.buffer[i];
        if (!(risc_symbol_map.count(arg_inst)))
        {
            // if don't process call_inst first, other arg in reg will be damaged
            // call's return value will store in stack, so it's safe
            if (arg_inst->kind.tag != KOOPA_RVT_CALL)
                continue;
            Visit(arg_inst);
        }
    }

    for (int i = 0; i < callInst.args.len; i++)
    {
        koopa_raw_value_t arg_inst = (koopa_raw_value_t)callInst.args.buffer[i];
        if (!(risc_symbol_map.count(arg_inst)))
            Visit(arg_inst);
        assert(risc_symbol_map.count(arg_inst));

        Var arg_var = risc_symbol_map.find(arg_inst)->second;

        if (arg_var.tag == Var_in_Reg)
        {
            Free_reg(arg_var.name);
            risc_symbol_map.erase(arg_inst);
            if (i < 8)
            {
                string reg = "a0";
                reg[1] += i;
                cout << "mv " << reg << ", " << arg_var.name << endl;
                a[i] = 1;
            }
            else
            {
                // push to stack top like this:
                // ...
                // arg9
                // arg8
                // next func stack frame
                // pos is sp+(i-8)*4
                if ((i - 8) * 4 < 2048)
                {
                    cout << "sw " << arg_var.name << ", " << (i - 8) * 4 << "(sp)" << endl;
                }
                else
                {
                    string tmp_reg = Find_reg();
                    cout << "li " << tmp_reg << ", " << ((i - 8) * 4) << endl;
                    cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
                    cout << "sw " + arg_var.name + ", 0(" + tmp_reg + ")" << endl;
                    Free_reg(tmp_reg);
                }
            }
        }
        else if (arg_var.tag == Var_in_Stack_pos_int)
        {
            if (i < 8)
            {
                string reg = "a0";
                reg[1] += i;
                int pos = arg_var.pos;
                if (pos < 2048)
                {
                    cout << "lw " << reg << ", " << pos << "(sp)" << endl;
                }
                else
                {
                    string tmp_reg = Find_reg();
                    cout << "li " << tmp_reg << ", " << pos << endl;
                    cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
                    cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
                    Free_reg(tmp_reg);
                }
            }
            else
            {
                string reg = Find_reg();
                int pos = arg_var.pos;
                if (pos < 2048)
                {
                    cout << "lw " << reg << ", " << pos << "(sp)" << endl;
                }
                else
                {
                    string tmp_reg = Find_reg();
                    cout << "li " << tmp_reg << ", " << pos << endl;
                    cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
                    cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
                    Free_reg(tmp_reg);
                }
                // pos is sp+(i-8)*4
                if ((i - 8) * 4 < 2048)
                {
                    cout << "sw " << reg << ", " << (i - 8) * 4 << "(sp)" << endl;
                }
                else
                {
                    string tmp_reg = Find_reg();
                    cout << "li " << tmp_reg << ", " << ((i - 8) * 4) << endl;
                    cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
                    cout << "sw " + reg + ", 0(" + tmp_reg + ")" << endl;
                    Free_reg(tmp_reg);
                }
                Free_reg(reg);
            }
        }
        else
        {
            cerr << arg_var.tag << endl;
            assert(false);
        }
    }

    string callee_name(callInst.callee->name);
    cout << "call " << callee_name.substr(1, strlen(callInst.callee->name)) << endl;

    for (int i = 0; i < callInst.args.len; i++)
    {
        if (i < 8)
            a[i] = 0;
    }

    // store return value in stack
    int pos = Find_stack();
    if (pos < 2048)
    {
        cout << "sw a0, " << pos << "(sp)\n";
    }
    else
    {
        string tmp_reg = Find_reg();
        cout << "li " << tmp_reg << ", " << pos << endl;
        cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
        cout << "sw a0, 0(" + tmp_reg + ")" << endl;
        Free_reg(tmp_reg);
    }

    Var new_var;
    new_var.pos = pos;
    new_var.tag = Var_in_Stack_pos_int;
    risc_symbol_map.insert(make_pair(super_value, new_var));

    return;
}

// visit value - func_arg
void Visit(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Index.
    // size_t index;
    // } koopa_raw_func_arg_ref_t;

    if (func_arg_refInst.index < 8)
    {
        // a0~a7
        string reg = "a0";
        reg[1] += func_arg_refInst.index;
        Var new_var;
        new_var.name = reg;
        new_var.tag = Var_in_Reg;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else
    {
        // (sp + stack size) + (index-8)*4
        int pos = save_stack_size.back() + (func_arg_refInst.index - 8) * 4;
        Var new_var;
        new_var.pos = pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    return;
}

// visit value - get_elem_ptr
void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Source.
    // koopa_raw_value_t src;
    // /// Index.
    // koopa_raw_value_t index;
    // } koopa_raw_get_elem_ptr_t;

    // index
    string reg_for_index;
    if (!(risc_symbol_map.count(get_elem_ptrInst.index)))
        Visit(get_elem_ptrInst.index);
    assert(risc_symbol_map.count(get_elem_ptrInst.index));

    Var index_var = risc_symbol_map.find(get_elem_ptrInst.index)->second;

    if (index_var.tag == Var_in_Reg)
    {
        reg_for_index = index_var.name;
        risc_symbol_map.erase(get_elem_ptrInst.index);
    }
    else if (index_var.tag == Var_in_Stack_pos_int)
    {
        int ind_pos = index_var.pos;
        reg_for_index = Find_reg();
        if (ind_pos < 2048)
        {
            cout << "lw " << reg_for_index << ", " << ind_pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << ind_pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg_for_index + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        assert(false);
    }

    // elem size in src
    string reg_for_size = Find_reg();
    // cerr << count_get_elem_ptr_len(get_elem_ptrInst.src->ty) << endl;
    cout << "li " << reg_for_size << ", " << count_get_elem_ptr_len(get_elem_ptrInst.src->ty) << endl;

    // src
    if (!risc_symbol_map.count(get_elem_ptrInst.src))
        Visit(get_elem_ptrInst.src);
    assert(risc_symbol_map.count(get_elem_ptrInst.src));

    Var src_var = risc_symbol_map.find(get_elem_ptrInst.src)->second;

    if (src_var.tag == Var_in_Stack_pos_int)
    {
        // 返回的是一个指向 M(stack[pos+sp]+size*index) 的指针
        // stack[pointer_pos(sp)] = stack[pos+sp]+size*index

        // stack[pos+sp]
        string reg = Find_reg();
        if (src_var.pos < 2048)
        {
            cout << "lw " << reg << ", " << src_var.pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << src_var.pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }

        Free_reg(reg);
        Free_reg(reg_for_index);

        // reg_for_size = stack[pos+sp]+size*index
        cout << "mul " + reg_for_size + ", " + reg_for_index + ", " + reg_for_size << endl;
        cout << "add " + reg_for_size + ", " + reg_for_size + ", " + reg << endl;

        // pointer_pos(sp) <- reg_for_size
        int pointer_pos = Find_stack();
        if (pointer_pos < 2048)
        {
            cout << "sw " << reg_for_size << ", " << pointer_pos << "(sp)\n";
        }
        else
        {
            string tmp_reg_2 = Find_reg();
            cout << "li " << tmp_reg_2 << ", " << pointer_pos << endl;
            cout << "add " + tmp_reg_2 + ", sp, " << tmp_reg_2 << endl;
            cout << "sw " + reg_for_size + ", 0(" + tmp_reg_2 + ")" << endl;
            Free_reg(tmp_reg_2);
        }

        Free_reg(reg_for_size);

        Var new_var;
        new_var.pos = pointer_pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else if (src_var.tag == Var_in_Global)
    {
        // 返回的是一个指向 M(addr+size*index) 的指针
        // stack[pointer_pos(sp)] = addr+size*index

        string reg_for_sec_pos = Find_reg();
        Free_reg(reg_for_sec_pos);
        Free_reg(reg_for_index);

        // addr
        cout << "la " << reg_for_sec_pos << ", " << src_var.name << endl;

        // reg_for_size = addr+size*index
        cout << "mul " + reg_for_size + ", " + reg_for_index + ", " + reg_for_size << endl;
        cout << "add " + reg_for_size + ", " + reg_for_size + ", " + reg_for_sec_pos << endl;

        // pointer_pos(sp) <- reg_for_size
        int pointer_pos = Find_stack();
        if (pointer_pos < 2048)
        {
            cout << "sw " << reg_for_size << ", " << pointer_pos << "(sp)\n";
        }
        else
        {
            string tmp_reg_2 = Find_reg();
            cout << "li " << tmp_reg_2 << ", " << pointer_pos << endl;
            cout << "add " + tmp_reg_2 + ", sp, " << tmp_reg_2 << endl;
            cout << "sw " + reg_for_size + ", 0(" + tmp_reg_2 + ")" << endl;
            Free_reg(tmp_reg_2);
        }

        Free_reg(reg_for_size);

        Var new_var;
        new_var.pos = pointer_pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else
    {
        cerr << src_var.tag << endl;
        assert(false);
    }
}

// visit value - get_ptr
void Visit(const koopa_raw_get_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Source.
    // koopa_raw_value_t src;
    // /// Index.
    // koopa_raw_value_t index;
    // } koopa_raw_get_ptr_t;

    // same as get_elem_ptr
    // index
    string reg_for_index;
    if (!(risc_symbol_map.count(get_elem_ptrInst.index)))
        Visit(get_elem_ptrInst.index);
    assert(risc_symbol_map.count(get_elem_ptrInst.index));

    Var index_var = risc_symbol_map.find(get_elem_ptrInst.index)->second;

    if (index_var.tag == Var_in_Reg)
    {
        reg_for_index = index_var.name;
        risc_symbol_map.erase(get_elem_ptrInst.index);
    }
    else if (index_var.tag == Var_in_Stack_pos_int)
    {
        int ind_pos = index_var.pos;
        reg_for_index = Find_reg();
        if (ind_pos < 2048)
        {
            cout << "lw " << reg_for_index << ", " << ind_pos << "(sp)" << endl;
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << ind_pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg_for_index + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }
    }
    else
    {
        assert(false);
    }

    // elem size in src
    string reg_for_size = Find_reg();
    // cerr << count_get_ptr_len(get_elem_ptrInst.src->ty) << endl;
    cout << "li " << reg_for_size << ", " << count_get_ptr_len(get_elem_ptrInst.src->ty) << endl;

    // src
    if (!risc_symbol_map.count(get_elem_ptrInst.src))
        Visit(get_elem_ptrInst.src);
    assert(risc_symbol_map.count(get_elem_ptrInst.src));

    Var src_var = risc_symbol_map.find(get_elem_ptrInst.src)->second;

    if (src_var.tag == Var_in_Stack_pos_int)
    {
        // 返回的是一个指向 M(stack[pos+sp]+size*index) 的指针
        // stack[pointer_pos(sp)] = stack[pos+sp]+size*index

        // stack[pos+sp]
        string reg = Find_reg();
        if (src_var.pos < 2048)
        {
            cout << "lw " << reg << ", " << src_var.pos << "(sp)\n";
        }
        else
        {
            string tmp_reg = Find_reg();
            cout << "li " << tmp_reg << ", " << src_var.pos << endl;
            cout << "add " + tmp_reg + ", sp, " << tmp_reg << endl;
            cout << "lw " + reg + ", 0(" + tmp_reg + ")" << endl;
            Free_reg(tmp_reg);
        }

        Free_reg(reg);
        Free_reg(reg_for_index);

        // reg_for_size = stack[pos+sp]+size*index
        cout << "mul " + reg_for_size + ", " + reg_for_index + ", " + reg_for_size << endl;
        cout << "add " + reg_for_size + ", " + reg_for_size + ", " + reg << endl;

        // pointer_pos(sp) <- reg_for_size
        int pointer_pos = Find_stack();
        if (pointer_pos < 2048)
        {
            cout << "sw " << reg_for_size << ", " << pointer_pos << "(sp)\n";
        }
        else
        {
            string tmp_reg_2 = Find_reg();
            cout << "li " << tmp_reg_2 << ", " << pointer_pos << endl;
            cout << "add " + tmp_reg_2 + ", sp, " << tmp_reg_2 << endl;
            cout << "sw " + reg_for_size + ", 0(" + tmp_reg_2 + ")" << endl;
            Free_reg(tmp_reg_2);
        }

        Free_reg(reg_for_size);

        Var new_var;
        new_var.pos = pointer_pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else if (src_var.tag == Var_in_Global)
    {
        // 返回的是一个指向 M(addr+size*index) 的指针
        // stack[pointer_pos(sp)] = addr+size*index

        string reg_for_sec_pos = Find_reg();
        Free_reg(reg_for_sec_pos);
        Free_reg(reg_for_index);

        // addr
        cout << "la " << reg_for_sec_pos << ", " << src_var.name << endl;

        // reg_for_size = addr+size*index
        cout << "mul " + reg_for_size + ", " + reg_for_index + ", " + reg_for_size << endl;
        cout << "add " + reg_for_size + ", " + reg_for_size + ", " + reg_for_sec_pos << endl;

        // pointer_pos(sp) <- reg_for_size
        int pointer_pos = Find_stack();
        if (pointer_pos < 2048)
        {
            cout << "sw " << reg_for_size << ", " << pointer_pos << "(sp)\n";
        }
        else
        {
            string tmp_reg_2 = Find_reg();
            cout << "li " << tmp_reg_2 << ", " << pointer_pos << endl;
            cout << "add " + tmp_reg_2 + ", sp, " << tmp_reg_2 << endl;
            cout << "sw " + reg_for_size + ", 0(" + tmp_reg_2 + ")" << endl;
            Free_reg(tmp_reg_2);
        }

        Free_reg(reg_for_size);

        Var new_var;
        new_var.pos = pointer_pos;
        new_var.tag = Var_in_Stack_pos_int;
        risc_symbol_map.insert(make_pair(super_value, new_var));
    }
    else
    {
        cerr << src_var.tag << endl;
        assert(false);
    }
}

// visit value - aggregate
void Visit(const koopa_raw_aggregate_t &aggregateInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Elements.
    // koopa_raw_slice_t elems;
    // } koopa_raw_aggregate_t;

    for (int i = 0; i < aggregateInst.elems.len; i++)
    {
        koopa_raw_value_t elem_inst = (koopa_raw_value_t)aggregateInst.elems.buffer[i];
        if (elem_inst->kind.tag == KOOPA_RVT_AGGREGATE)
        {
            Visit(elem_inst->kind.data.aggregate, elem_inst);
        }
        else if (elem_inst->kind.tag == KOOPA_RVT_INTEGER)
        {
            cout << ".word " << elem_inst->kind.data.integer.value << endl;
        }
        else
        {
            assert(false);
        }
    }
}

// visit raw slice
void Visit(const koopa_raw_slice_t &slice)
{
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];

        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // visit func
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // visit bb
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // visit value
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
        }
    }
}

// count stack size - bb
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

// count stack size - value
void Count_var(const koopa_raw_value_t &value)
{
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // visit return
        Count_var(kind.data.ret, value);
        break;
    case KOOPA_RVT_INTEGER:
        // visit integer
        Count_var(kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        // visit binary
        Count_var(kind.data.binary, value);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        // visit global_alloc
        // do nothing
        break;
    case KOOPA_RVT_ALLOC:
        // visit alloc
        Count_var_alloc(value);
        break;
    case KOOPA_RVT_LOAD:
        // visit load
        Count_var(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // visit store
        Count_var(kind.data.store, value);
        break;
    case KOOPA_RVT_BRANCH:
        // visit br
        Count_var(kind.data.branch, value);
        break;
    case KOOPA_RVT_JUMP:
        // visit jump
        Count_var(kind.data.jump, value);
        break;
    case KOOPA_RVT_CALL:
        // visit call
        Count_var(kind.data.call, value);
        break;
    case KOOPA_RVT_FUNC_ARG_REF:
        // visit func_arg
        Count_var(kind.data.func_arg_ref, value);
        break;
    case KOOPA_RVT_AGGREGATE:
        // visit aggregate
        // do nothing
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        Count_var(kind.data.get_elem_ptr, value);
        break;
    case KOOPA_RVT_GET_PTR:
        Count_var(kind.data.get_ptr, value);
        break;
    default:
        // what are these?
        // KOOPA_RVT_BLOCK_ARG_REF
        // KOOPA_RVT_UNDEF
        cout << kind.tag << endl;
        assert(false);
    }
}

// count stack size - value - return
void Count_var(const koopa_raw_return_t &retInst, const koopa_raw_value_t &super_value)
{
    if (!retInst.value)
        return;

    // if const, will not use stack
    if (retInst.value->kind.tag == KOOPA_RVT_INTEGER)
        return;

    // count size for return value
    if (!count_symbol_map.count(retInst.value))
        Count_var(retInst.value);
    assert(count_symbol_map.count(retInst.value));
}

// count stack size - value - integer
void Count_var(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    count_symbol_map.insert(make_pair(super_value, 0));
}

// count stack size - value - binary
void Count_var(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value)
{
    if (!count_symbol_map.count(binaryInst.lhs))
        Count_var(binaryInst.lhs);
    assert(count_symbol_map.count(binaryInst.lhs));

    if (!count_symbol_map.count(binaryInst.rhs))
        Count_var(binaryInst.rhs);
    assert(count_symbol_map.count(binaryInst.rhs));

    int stack_pos = Add_stack_count();
    count_symbol_map.insert(make_pair(super_value, stack_pos));
}

// count stack size - value - alloc
void Count_var_alloc(const koopa_raw_value_t &super_value)
{
    int alloc_size = count_type_size(super_value->ty->data.pointer.base);

    Add_stack_count(alloc_size / 4);

    // stack for pointer
    int stack_pos = Add_stack_count();
    count_symbol_map.insert(make_pair(super_value, stack_pos));
}

// count stack size - value - load
void Count_var(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value)
{
    if (!(count_symbol_map.count(loadInst.src) || risc_symbol_map.count(loadInst.src)))
        Count_var(loadInst.src);
    assert(count_symbol_map.count(loadInst.src) || risc_symbol_map.count(loadInst.src));

    int stack_pos = Add_stack_count();
    count_symbol_map.insert(make_pair(super_value, stack_pos));
}

// count stack size - value - store
void Count_var(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // count size for put value in reg
    if (!count_symbol_map.count(storeInst.value))
        Count_var(storeInst.value);
    assert(count_symbol_map.count(storeInst.value));

    // count size for sw reg in dest
    if (!(count_symbol_map.count(storeInst.dest) || risc_symbol_map.count(storeInst.dest)))
        Count_var(storeInst.dest);
    assert(count_symbol_map.count(storeInst.dest) || risc_symbol_map.count(storeInst.dest));
}

// count stack size - value - br
void Count_var(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value)
{
    // cond
    if (!count_symbol_map.count(brInst.cond))
        Count_var(brInst.cond);
    assert(count_symbol_map.count(brInst.cond));

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

// count stack size - value - jump
void Count_var(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value)
{
    // target
    if (!count_mem_map_block.count(jumpInst.target))
    {
        count_mem_map_block.insert(jumpInst.target);
        Count_var(jumpInst.target);
    }
}

// count stack size - value - call
void Count_var(const koopa_raw_call_t &callInst, const koopa_raw_value_t &super_value)
{
    // args
    count_has_call = 1;
    if (callInst.args.len > 8)
        Add_stack_count(callInst.args.len - 8);

    // store return value need a pos
    int pos = Add_stack_count();
    count_symbol_map.insert(make_pair(super_value, pos));
}

// count stack size - value - func_arg
void Count_var(const koopa_raw_func_arg_ref_t &func_arg_refInst, const koopa_raw_value_t &super_value)
{
    // 0~7 - in reg
    // >7 - in pre caller's stack
    count_symbol_map.insert(make_pair(super_value, 0));
}

// count stack size - value - get_elem_ptr
void Count_var(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // index
    if (!(count_symbol_map.count(get_elem_ptrInst.index)))
        Count_var(get_elem_ptrInst.index);
    assert(count_symbol_map.count(get_elem_ptrInst.index));

    // src
    if (!(count_symbol_map.count(get_elem_ptrInst.src) || risc_symbol_map.count(get_elem_ptrInst.src)))
        Count_var(get_elem_ptrInst.src);
    assert(count_symbol_map.count(get_elem_ptrInst.src) || risc_symbol_map.count(get_elem_ptrInst.src));

    // stack for pointer
    int pos = Add_stack_count();

    count_symbol_map.insert(make_pair(super_value, pos));
}

// count stack size - value - get_ptr
void Count_var(const koopa_raw_get_ptr_t &get_ptrInst, const koopa_raw_value_t &super_value)
{
    // index
    if (!(count_symbol_map.count(get_ptrInst.index)))
        Count_var(get_ptrInst.index);
    assert(count_symbol_map.count(get_ptrInst.index));

    // src
    if (!(count_symbol_map.count(get_ptrInst.src) || risc_symbol_map.count(get_ptrInst.src)))
        Count_var(get_ptrInst.src);
    assert(count_symbol_map.count(get_ptrInst.src) || risc_symbol_map.count(get_ptrInst.src));

    // stack for pointer
    int pos = Add_stack_count();

    count_symbol_map.insert(make_pair(super_value, pos));
}
