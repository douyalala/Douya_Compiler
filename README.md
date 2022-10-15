# douya的编译实践项目

它曾经是：**基于 Makefile 的 SysY 编译器项目模板**(#https://github.com/pku-minic/sysy-make-template)

## 使用方法

make并运行build/compiler -koopa/riscv hello.c -o hello.koopa
```sh
python3 make_run.py -r/k
```

本地测试所有目前做完的部分
```sh
python3 test_old.py
```

## 当前进度

Lv6.1结束
Lv6即以前的本地测试全部PASS
TODO List：
- Lv6.2 短路求值
- Lv7 While
- Lv8 函数和全局变量
- Lv9 数组