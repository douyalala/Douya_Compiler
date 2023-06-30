import os
import sys

mode=sys.argv[1]
if(mode[1]=='r'):
    print("make and run mode: -riscv")
    os.system("make")
    variable = input("continue")
    os.system("build/compiler -riscv hello.c -o hello.o")
elif(mode[1]=='k'):
    print("make and run mode: -koopa")
    os.system("make")
    variable = input("continue")
    os.system("build/compiler -koopa hello.c -o hello.koopa")