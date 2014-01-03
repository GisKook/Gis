#作用：将多个.h文件合并
#参数：所有.h文件所在文件夹 不包含'/'
#输出：GK+文件夹名称.hpp
#例子：python.exe E:\Gis\01_SourceCode\Include\GenerateInclude.py E:\Gis\01_SourceCode\Include\Engine
#关于：输出的文件使用的本地编码，所以在注释部分就使用英语吧，如果遇到的.h的头文件为utf-8再回头修改脚本，
#      换行用的'\n'而不是os.linesep
import sys
import string
import os
import time

def addtips(desfilename, filesname):
    f=file(desfilename,'w')
    f.truncate()
    f.write('/*'+'\n')
    f.write(' * This file is a collection of header files, easy to use caller :)'+'\n')
    f.write(' *'+'\n')
    f.write(' * There are header files: '+'\n')
    for i in filesname:
        f.write(' *    '+i+'\n')
    f.write(' * author : zhangkai'+'\n')
    cur_time=time.ctime(time.time())
    f.write(' * time : '+cur_time+'\n')
    f.write(' */'+'\n')
    f.close()

def add_file_content(desfilename, filespath,filesname):
    f=file(desfilename,'a')
    for i in filesname:
        src_file=file(os.path.join(filepath,i),'r')
        f.write('\n// source_file: :)' + i + '\n')
        while True: 
            line=src_file.readline()
            if len(line)==0:
                src_file.close()
                break
            f.write(line)
    f.close()

filepath=sys.argv[1]
headername=os.path.split(filepath)
headername=headername[len(headername)-1]
headername='GK'+headername+'.hpp'
headername=os.path.join(filepath, headername)
if os.path.exists(headername):
    os.remove(headername)
file_names=os.listdir(filepath)
addtips(headername,file_names)
add_file_content(headername, filepath, file_names)
