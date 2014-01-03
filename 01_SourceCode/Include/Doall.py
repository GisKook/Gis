#调用GenerateInclude.py脚本一次生成所有头文件
import os
import sys
import subprocess

cur_path=os.path.split(sys.argv[0])[0]

files=os.listdir(cur_path)
for i in files:
    if os.path.isdir(os.path.join(cur_path,i)):
        subprocess.call(["c:/Python26/python.exe",os.path.join(cur_path,"GenerateInclude.py"),os.path.join(cur_path,i)])
        print os.path.join(cur_path,i) + ' ... processed'
