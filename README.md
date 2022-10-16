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

### 总体进度

Lv8.1 Lv8.2 koopa部分完成

### 本地测试情况

koopa：

Lv8-08_lib_funcsji及以前的本地测试全部PASS

riscv:

Lv7及以前的本地测试全部PASS

## TODO List：

1. Lv8 函数和全局变量
2. Lv9 数组

## 注意：

1. Lv6.2 短路求值：目前没有相关测试，可能有未知错误
2. Lv6 和 Lv7 如果if和while语句结束以后没有任何东西就会崩，因为默认生成了%end，后面如果没东西koopa会报错
   - 这个现在理论上来说if和while不会有问题了：如果if或while不是最后一个（或者被套在别的if或while内部）后边一定会有东西，如果是最后一个，我在func的AST里面加了判断，后面会接一个return，所以不会出现空基本块（大概）
3. Lv6 和 Lv7 改的匆忙，不排除还有点问题
   1. 基本块最后只能为br，return，jump，其中br后面一定接的是我定义的新基本块，所以暂时没啥问题，但是return和jump可能冲突
   2. 现在的实现，基本思想是如果if或else或while的内部stmt含有return指令，就不生成jump到%end的koopa，又因为blockAST中我让所有ret指令后的指令不生成koopa，所以此时return一定是最后一条指令
4. 关于符号表：
   1. 是一个链表，内部块指向外部块
   2. 进入每个函数时创建新的符号表，此时一般存入函数参数
   3. 进入每个block时创建新的符号表
5. 现在临时变量命名和基本块标号较为浪费
   1. 进入函数时基本块标号+1
      1. 这个有没有必要还不好说，主要是害怕函数内定义的变量和函数参数同名
   2. 进入形如{}的基本块时标号+1
   3. 导致了一个函数至少会令基本块标号+2
   4. 无论处于哪个基本块中，临时变量标号递增，标号很容易飞到几十
      1. 也许可以考虑在每个函数中临时变量分别计数？
      2. 当然不管也没啥问题就是比较丑
   5. 变量名为原名+定义它的基本块标号，由于基本块标号起飞，变量名也会起飞