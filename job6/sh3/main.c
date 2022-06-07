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
	 : ִ��һ��ָ�� 
PARAM :
	@line : ָ���ַ��� 
*/ 
void execute_line(char *line)
{
    tree_t *tree;				
    lex_init(line);
    tree = parse_tree();									/* ���ַ���ָ��ת��Ϊ���νṹ���� */ 
    if (verbose)											/* ��������� -v ѡ��, �����ʾ�����νṹ */
    {
        printf("----- CMD TREE ---------------------------\n");
        tree_dump(tree, 0);
        printf("------------------------------------------\n");
    }
    if (tree != NULL) 										/* �����νṹΪ����,ִ��ָ�� */
        tree_execute_wrapper(tree);
    lex_destroy();
}

/*
FUNC : read_line
     : ��ȡһ������ڴ�
PARAM :
    @line : ������
    @size : �����ַ���
*/
void read_line(char *line, int size)
{
    int count;

    count = read(0, line, size);
    if (count == 0)
        exit(EXIT_SUCCESS);
    assert(count > 0);
    if ((count > 0) && (line[count - 1] == '\n'))           /* �ų���ĩ\nӰ�� */
        line[count - 1] = 0;
    else
        line[count] = 0;
}

/*
FUNC : read_and_execute
	 : ��ȡһ��ָ�� �� ִ��
 */ 
void read_and_execute()
{
    char line[128];

    write(1, getcwd(NULL, 128), strlen(getcwd(NULL, 128))); /* ��ӡ��ʾ��# */
    write(1, " - sh3 # ", 9);

    read_line(line, sizeof(line));							/* ��ȡһ�� */ 
    execute_line(line);										/* ִ��һ�� */ 
}

void test()
{
    execute_line("cat /etc/passwd | sort | grep root >log"); 
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "-v") == 0) 			/* �ж� -v ѡ�� -- ��ִ��ָ��ǰ�����ָ������νṹ */ 
        verbose = 1;
    while (1)
        read_and_execute();
    return 0;
}
