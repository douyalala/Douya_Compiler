import os
count = 0
cmd_lines = ["autotest -koopa -s lv1 /root/compiler",
             "autotest -riscv -s lv1 /root/compiler",
             "autotest -koopa -s lv3 /root/compiler",
             "autotest -riscv -s lv3 /root/compiler",
             "autotest -koopa -s lv4 /root/compiler",
             "autotest -riscv -s lv4 /root/compiler",
             "autotest -koopa -s lv5 /root/compiler",
             "autotest -riscv -s lv5 /root/compiler"]
for cmd_line in cmd_lines:
    count = count+1
    os.system(cmd_line)
    variable = input(str(count)+"/"+str(len(cmd_lines)))
