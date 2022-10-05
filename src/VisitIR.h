#pragma once

#include <cstring>
#include <string>
#include "koopa.h"

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &value);
void Visit(const koopa_raw_integer_t &value);
void Visit(const koopa_raw_slice_t &slice);

// 访问 raw program
void Visit(const koopa_raw_program_t &program){
    // koopa_raw_program_t
    /// Global values (global allocations only).
    // koopa_raw_slice_t values;
    /// Function definitions.
    // koopa_raw_slice_t funcs;

    // 执行一些其他的必要操作
    std::cout<<".text\n.globl main\n";

    // 访问所有全局变量-目前没有用
    // Visit(program.values);

    // 访问所有函数
    for (size_t i = 0; i < program.funcs.len; ++i){
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t)program.funcs.buffer[i];
        Visit(func);
    }
}

// 访问函数
void Visit(const koopa_raw_function_t &func){
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
    std::string tmp_name(func->name);

    std::cout << tmp_name.substr(1,strlen(func->name)) << ":\n";

    // 访问所有基本块
    for (size_t i = 0; i < func->bbs.len; ++i){
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        Visit(bb);
    }
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb){
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
    for (size_t i = 0; i < bb->insts.len; ++i){
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t inst = (koopa_raw_value_t)bb->insts.buffer[i];
        Visit(inst);
    }
}

// 访问指令
void Visit(const koopa_raw_value_t &value){
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
        Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        Visit(kind.data.integer);
        break;
    default:
        // 其他类型暂时遇不到
        assert(false);
    }
}

// 访问指令-return
void Visit(const koopa_raw_return_t &retInst){
    // koopa_raw_return_t
    /// Return value, null if no return value.
    // koopa_raw_value_t value;

    std::cout << "li a0,";
    Visit(retInst.value); 
    std::cout << "\nret\n";
}

// 访问指令-integer
void Visit(const koopa_raw_integer_t &intInst){
    // koopa_raw_integer_t：
    /// Value of integer.
    // int32_t value;

    std::cout << intInst.value;
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice){
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

