#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "parse.h"
#include "exec.h"

int verbose = 0;

/*
FUNC : execute_line
	 : 执行一行指令 
PARAM :
	@line : 指令字符串 
*/ 
void execute_line(char *line)
{
    tree_t *tree;				
    lex_init(line);
    tree = parse_tree();									/* 将字符串指令转换为树形结构描述 */ 
    if (verbose)											/* 如果启用了 -v 选项, 输出显示该树形结构 */
    {
        printf("----- CMD TREE ---------------------------\n");
        tree_dump(tree, 0);
        printf("------------------------------------------\n");
    }
    if (tree != NULL) 										/* 以树形结构为根据,执行指令 */
        tree_execute_wrapper(tree);
    lex_destroy();
}

/*
FUNC : read_line
     : 读取一行命令到内存
PARAM :
    @line : 命令存放
    @size : 命令字符数
*/
void read_line(char *line, int size)
{
    int count;

    count = read(0, line, size);
    if (count == 0)
        exit(EXIT_SUCCESS);
    assert(count > 0);
    if ((count > 0) && (line[count - 1] == '\n'))           /* 排除句末\n影响 */
        line[count - 1] = 0;
    else
        line[count] = 0;
}

/*
FUNC : read_and_execute
	 : 读取一行指令 并 执行
 */ 
void read_and_execute()
{
    char line[128];

    write(1, getcwd(NULL, 128), strlen(getcwd(NULL, 128))); /* 打印提示符# */
    write(1, " - sh3 # ", 9);

    read_line(line, sizeof(line));							/* 读取一行 */ 
    execute_line(line);										/* 执行一行 */ 
}

void test()
{
    execute_line("cat /etc/passwd | sort | grep root >log"); 
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "-v") == 0) 			/* 判断 -v 选项 -- 在执行指令前，输出指令的树形结构 */ 
        verbose = 1;
    while (1)
        read_and_execute();
    return 0;
}
