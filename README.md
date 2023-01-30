# Douya's Compiler For PKU Compiler Principles Course

它是基于：**基于 Makefile 的 SysY 编译器项目模板(https://github.com/pku-minic/sysy-make-template)**

It is based on **SysY Make Template(https://github.com/pku-minic/sysy-make-template)**

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
python3 make_run.py -k/r
```

3. 本地测试
   
   local test

```sh
python3 test_old.py
```