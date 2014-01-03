#调用GenerateInclude.py脚本一次生成所有头文件
import os
import sys
import subprocess

cur_path=os.path.split(sys.argv[0])[0]

files=os.listdir(cur_path)
for i in files:
    folder=os.path.join(cur_path,i)
    if os.path.isdir(folder):
        hppfiles=os.listdir(folder)
        for j in hppfiles:
            if j[0:2]=='GK' and j[len(j)-3:len(j)]=='hpp':
                os.remove(os.path.join(folder, j))
                print os.path.join(folder,j)+' ... removed'
