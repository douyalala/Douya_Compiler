# douya的编译实践项目

它曾经是：**基于 Makefile 的 SysY 编译器项目模板(https://github.com/pku-minic/sysy-make-template)**

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

Lv7结束

Lv7及以前的本地测试全部PASS

TODO List：
1. Lv8 函数和全局变量
2. Lv9 数组

注意：
1. Lv6.2 短路求值：目前没有相关测试，可能有未知错误
2. Lv6 和 Lv7 如果if和while语句结束以后没有任何东西就会崩，因为默认生成了%end，后面如果没东西koopa会报错（但是看了看本地测试集似乎后面都会有东西，我不确定这不是某种语义规则，在doc里面也没找到）
3. Lv6 和 Lv7 改的匆忙，可能还有点问题
   1. 基本块最后只能为br，return，jump，其中br后面一定接的是我定义的新基本块，所以暂时没啥问题，但是return和jump可能冲突
   2. 现在的实现，基本思想是如果if或else或while的内部stmt含有return指令，就不生成jump到%end的koopa，又因为blockAST中我让所有ret指令后的指令不生成koopa，所以此时return一定是最后一条指令