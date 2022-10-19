# Douya's Compiler For PKU compiler course

它曾经是：**基于 Makefile 的 SysY 编译器项目模板(https://github.com/pku-minic/sysy-make-template)**

It used to be **SysY Make Template(https://github.com/pku-minic/sysy-make-template)**

## Usage

1. 挂载到docker
   
   run in docker

```sh
docker run -it --rm -v [Directory of this folder]:/root/compiler maxxing/compiler-dev bash
cd compiler/
```

2. make并运行build/compiler -koopa/riscv hello.c -o hello.koopa
   
   make and run build/compiler -koopa/riscv hello.c -o hello.koopa

```sh
python3 make_run.py -r/k
```

3. 本地测试所有目前做完的部分
   
   local test

```sh
python3 test_old.py
```

## TODO List：

1. Lv9 array