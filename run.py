import os
import sys
import time

def verify(cmd, dir_name, window, flag, name):
    file_list = sorted(os.listdir(dir_name))
    for i in range(0,len(file_list),2):
	if os.path.getsize(dir_name + file_list[i + 1]) == 0:
	    continue
        commond = "./asm " + cmd + " " + dir_name + file_list[i] + " " + dir_name + file_list[i + 1] + " " + window + " " + flag + " " + name 
        os.system(commond)
	print(file_list[i])

def main(arg1, arg2, arg3, arg4, arg5):
    verify(arg1, arg2, arg3, arg4, arg5)
    
if __name__== "__main__":
    start = time.time()
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
    print time.time() - start, "s"
        
    

