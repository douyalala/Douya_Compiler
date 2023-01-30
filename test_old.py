import os
import math
count = 0
cmd_lines = ["autotest -koopa -s lv1 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv1 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv3 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv3 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv4 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv4 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv5 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv5 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv6 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv6 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv7 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv7 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -koopa -s lv8 /root/compiler 1>/dev/null 2> test_logs/",
             "autotest -riscv -s lv8 /root/compiler 1>/dev/null 2> test_logs/", 
             "autotest -koopa -s lv9 /root/compiler 1>/dev/null 2> test_logs/", 
             "autotest -riscv -s lv9 /root/compiler 1>/dev/null 2> test_logs/"]

ind = ["koopa", "riscv"]

for cmd_line in cmd_lines:
    if (count <= 1):
        test_name = "Lv1"+ind[count % 2]
    else:
        test_name = "Lv"+str(math.floor((count+4)/2))+ind[(count % 2)]
    test_name = test_name+".txt"

    print("now running: "+cmd_lines[count]+test_name + " -- " +
          str(count+1)+"/"+str(len(cmd_lines)))
    count = count+1

    os.system(cmd_line+test_name)

    fb = open(file="./test_logs/"+test_name, mode='r', encoding=None)
    lines = fb.readlines()
    last_line = lines[len(lines)-1]
    print(last_line.rstrip())