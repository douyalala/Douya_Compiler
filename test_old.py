import os
os.system("autotest -koopa -s lv1 /root/compiler")
variable = input("continue")
os.system("autotest -riscv -s lv1 /root/compiler")
variable = input("continue")
os.system("autotest -koopa -s lv3 /root/compiler")
variable = input("continue")
os.system("autotest -riscv -s lv3 /root/compiler")