1.仓库中只包含了代码文件 
2.light_shell 实现了简单的cd、ls、cat、pwd、管道功能 
3.缺陷：命令输入有些空格没有处理，可能造成错误（比如: cat file.txt|grep string，管道|两旁不能有空格）
