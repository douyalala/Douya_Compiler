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

// value - reg/stack/global
map<koopa_raw_value_t, string> mem_map_reg;
map<koopa_raw_value_t, int> mem_map_stack;
map<koopa_raw_value_t, string> mem_map_global;

// count and name br bb
int count_if_else = 0;

// count and name br bb
int count_jump_Label = 0;

// bb - Label
map<koopa_raw_basic_block_t, string> mem_map_block;

// count stack size - tmp map and stack
int stack_frame_count = 0;
map<koopa_raw_value_t, string> count_mem_map_reg;
map<koopa_raw_value_t, int> count_mem_map_stack;
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
void Visit(const koopa_raw_aggregate_t &get_elem_ptrInst, const koopa_raw_value_t &super_value);

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
void Count_var(const koopa_raw_aggregate_t &get_elem_ptrInst, const koopa_raw_value_t &super_value);

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
    else
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
    count_mem_map_reg.clear();
    count_mem_map_stack.clear();
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
        cout << "sw ra, " << (stack_size - 4) << "(sp)" << endl;
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
            if (!(mem_map_reg.count(retInst.value) || mem_map_stack.count(retInst.value)))
                Visit(retInst.value);
            assert(mem_map_reg.count(retInst.value) || mem_map_stack.count(retInst.value));
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
        }
    }

    // pop ra
    int stack_size = save_stack_size.back();
    bool has_call = save_ra.back();
    if (has_call)
    {
        cout << "lw ra, " << (stack_size - 4) << "(sp)" << endl;
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

    if (intInst.value == 0)
        mem_map_reg.insert(make_pair(super_value, "x0"));
    else
    {
        string reg = Find_reg();
        cout << "li " << reg << ", " << intInst.value << "\n";
        mem_map_reg.insert(make_pair(super_value, reg));
    }
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

    if (!(mem_map_reg.count(binaryInst.lhs) || mem_map_stack.count(binaryInst.lhs)))
        Visit(binaryInst.lhs);
    assert(mem_map_reg.count(binaryInst.lhs) || mem_map_stack.count(binaryInst.lhs));

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

    if (!(mem_map_reg.count(binaryInst.rhs) || mem_map_stack.count(binaryInst.rhs)))
        Visit(binaryInst.rhs);
    assert(mem_map_reg.count(binaryInst.rhs) || mem_map_stack.count(binaryInst.rhs));
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
    cout << "sw " << reg << ", " << stack_pos << "(sp)\n";
    Free_reg(reg);
    mem_map_stack.insert(make_pair(super_value, stack_pos));
}

// visit value - global_alloc
void Visit(const koopa_raw_global_alloc_t &global_allocInst, const koopa_raw_value_t &super_value)
{
    // typedef struct {
    // /// Initializer.
    // koopa_raw_value_t init;
    // } koopa_raw_global_alloc_t;

    // I guess global_allocInst.init is integer or this:
    //  /// Zero initializer.
    // KOOPA_RVT_ZERO_INIT,

    string var_name(super_value->name);
    var_name = var_name.substr(1, strlen(super_value->name));

    cout << ".data\n";

    cout << ".globl " << var_name << endl;
    cout << var_name << ":\n";

    if (global_allocInst.init->kind.tag == KOOPA_RVT_INTEGER)
        cout << ".word " << global_allocInst.init->kind.data.integer.value << endl;
    else
        cout << ".zero 4\n";

    mem_map_global.insert(make_pair(super_value, var_name));
}

// visit value - alloc
void Visit_alloc(const koopa_raw_value_t &super_value)
{
    int pos = Find_stack();
    mem_map_stack.insert(make_pair(super_value, pos));
}

// visit value - load
void Visit(const koopa_raw_load_t &loadInst, const koopa_raw_value_t &super_value)
{
    // /// Source.
    // koopa_raw_value_t src;

    if (!(mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src)))
        Visit(loadInst.src);
    assert(mem_map_stack.count(loadInst.src) || mem_map_global.count(loadInst.src));

    if (mem_map_stack.count(loadInst.src))
    {
        // if load var on stack
        // the return var will push back to stack
        // do nothing but insert mem_map_stack
        mem_map_stack.insert(make_pair(super_value, mem_map_stack.find(loadInst.src)->second));
    }
    if (mem_map_global.count(loadInst.src))
    {
        string reg = Find_reg();
        int pos = Find_stack();
        cout << "la " << reg << ", " << mem_map_global.find(loadInst.src)->second << endl;
        cout << "lw " << reg << ", 0(" << reg << ")\n";
        cout << "sw " << reg << ", " << pos << "(sp)\n";
        Free_reg(reg);
        mem_map_stack.insert(make_pair(super_value, pos));
    }
}

// visit value - store
void Visit(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // /// Value.
    //   koopa_raw_value_t value;
    // /// Destination.
    //   koopa_raw_value_t dest;

    string value_reg;

    // put value in reg
    if (!(mem_map_reg.count(storeInst.value) || mem_map_stack.count(storeInst.value)))
        Visit(storeInst.value);
    assert(mem_map_reg.count(storeInst.value) || mem_map_stack.count(storeInst.value));
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

    // sw reg to dest
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

    if (!(mem_map_reg.count(brInst.cond) || mem_map_stack.count(brInst.cond)))
        Visit(brInst.cond);
    assert(mem_map_reg.count(brInst.cond) || mem_map_stack.count(brInst.cond));
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
        if (!(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst)))
        {
            // do call first, or other param in reg will broken
            // call's return value will store in stack, so it's safe
            if (arg_inst->kind.tag != KOOPA_RVT_CALL)
                continue;
            Visit(arg_inst);
        }
    }

    for (int i = 0; i < callInst.args.len; i++)
    {
        koopa_raw_value_t arg_inst = (koopa_raw_value_t)callInst.args.buffer[i];
        if (!(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst)))
            Visit(arg_inst);
        assert(mem_map_reg.count(arg_inst) || mem_map_stack.count(arg_inst));

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
                // push to stack top like this:
                // ...
                // arg9
                // arg8
                // next func stack frame
                // pos is sp+(i-8)*4
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
                // pos is sp+(i-8)*4
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

    // store return value in stack
    int pos = Find_stack();
    cout << "sw a0, " << pos << "(sp)\n";
    mem_map_stack.insert(make_pair(super_value, pos));

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
        mem_map_reg.insert(make_pair(super_value, reg));
    }
    else
    {
        // (sp + stack size) + (index-8)*4
        int pos = save_stack_size.back() + (func_arg_refInst.index - 8) * 4;
        mem_map_stack.insert(make_pair(super_value, pos));
    }
    return;
}

// visit value - get_elem_ptr
void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // TODO

    // typedef struct {
    // /// Source.
    // koopa_raw_value_t src;
    // /// Index.
    // koopa_raw_value_t index;
    // } koopa_raw_get_elem_ptr_t;



    assert(false);
}

// visit value - aggregate
void Visit(const koopa_raw_aggregate_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // TODO

    // typedef struct {
    // /// Elements.
    // koopa_raw_slice_t elems;
    // } koopa_raw_aggregate_t;

    assert(false);
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
        // TODO;
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        // TODO;
        break;
    default:
        // what are these?
        // KOOPA_RVT_GET_PTR
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
    if (!(count_mem_map_reg.count(retInst.value) || count_mem_map_stack.count(retInst.value)))
        Count_var(retInst.value);
    assert(count_mem_map_reg.count(retInst.value) || count_mem_map_stack.count(retInst.value));

    // free reg
    if (count_mem_map_reg.count(retInst.value))
    {
        Free_reg(count_mem_map_reg.find(retInst.value)->second);
        count_mem_map_reg.erase(retInst.value);
    }
}

// count stack size - value - integer
void Count_var(const koopa_raw_integer_t &intInst, const koopa_raw_value_t &super_value)
{
    string reg = Find_reg();
    count_mem_map_reg.insert(make_pair(super_value, reg));
    return;
}

// count stack size - value - binary
void Count_var(const koopa_raw_binary_t &binaryInst, const koopa_raw_value_t &super_value)
{
    if (!(count_mem_map_reg.count(binaryInst.lhs) || count_mem_map_stack.count(binaryInst.lhs)))
        Count_var(binaryInst.lhs);
    assert(count_mem_map_reg.count(binaryInst.lhs) || count_mem_map_stack.count(binaryInst.lhs));

    if (!(count_mem_map_reg.count(binaryInst.rhs) || count_mem_map_stack.count(binaryInst.rhs)))
        Count_var(binaryInst.rhs);
    assert(count_mem_map_reg.count(binaryInst.rhs) || count_mem_map_stack.count(binaryInst.rhs));

    int stack_pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, stack_pos));

    // delete reg
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

// count stack size - value - alloc
void Count_var_alloc(const koopa_raw_value_t &super_value)
{
    int pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, pos));
}

// count stack size - value - load
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

// count stack size - value - store
void Count_var(const koopa_raw_store_t &storeInst, const koopa_raw_value_t &super_value)
{
    // count size for put value in reg
    if (!(count_mem_map_reg.count(storeInst.value) || count_mem_map_stack.count(storeInst.value)))
        Count_var(storeInst.value);
    assert(count_mem_map_reg.count(storeInst.value) || count_mem_map_stack.count(storeInst.value));

    if (count_mem_map_reg.count(storeInst.value))
    {
        Free_reg(count_mem_map_reg.find(storeInst.value)->second);
        count_mem_map_reg.erase(storeInst.value);
    }

    // count size for sw reg in dest
    if (!(count_mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest)))
        Count_var(storeInst.dest);
    assert(count_mem_map_stack.count(storeInst.dest) || mem_map_global.count(storeInst.dest));
}

// count stack size - value - br
void Count_var(const koopa_raw_branch_t &brInst, const koopa_raw_value_t &super_value)
{
    // cond
    if (!(count_mem_map_reg.count(brInst.cond) || count_mem_map_stack.count(brInst.cond)))
        Count_var(brInst.cond);

    // free cond reg
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

// count stack size - value - jump
void Count_var(const koopa_raw_jump_t &jumpInst, const koopa_raw_value_t &super_value)
{
    // target
    if (!count_mem_map_block.count(jumpInst.target))
    {
        count_mem_map_block.insert(jumpInst.target);
        Count_var(jumpInst.target);
    }

    return;
}

// count stack size - value - call
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

    // store return value need a pos
    int pos = Add_stack_count();
    count_mem_map_stack.insert(make_pair(super_value, pos));

    return;
}

// count stack size - value - func_arg
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
        // in pre func's stack, not important in count stack size part
        count_mem_map_stack.insert(make_pair(super_value, 0));
    }
    return;
}

// count stack size - value - get_elem_ptr
void Count_var(const koopa_raw_get_elem_ptr_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // TODO

    // typedef struct {
    // /// Source.
    // koopa_raw_value_t src;
    // /// Index.
    // koopa_raw_value_t index;
    // } koopa_raw_get_elem_ptr_t;
    
    assert(false);
}

// count stack size - value - aggregate
void Count_var(const koopa_raw_aggregate_t &get_elem_ptrInst, const koopa_raw_value_t &super_value)
{
    // TODO

    // typedef struct {
    // /// Elements.
    // koopa_raw_slice_t elems;
    // } koopa_raw_aggregate_t;

    assert(false);
}

