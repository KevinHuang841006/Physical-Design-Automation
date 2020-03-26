--HOW TO COMPILE ：
使用make就可以直接編譯執行檔，會在目前目錄產生main執行檔，目錄中需存有 Makefile、main.cpp，而執行指令 make clear就可以清除。
執行的部分，執行命令為： ./main cell_file.cells net_file.nets out_file
argv[1] = cell file, argv[2] = net file, argv[3] = output file

--HOW TO RUN：
	主要有3個參數：cell, net, output_file
Usage：./main ../testcases/<.cell> ../testcases/<.nets> ../testcases/<.out>
ex： ./main ../testcases/p2-1.cells ../testcases/p2-2.nets output
