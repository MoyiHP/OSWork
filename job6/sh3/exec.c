#include "root.h"
#include "lex.h"
#include "parse.h"
#include "exec.h"

// echo abc >log
//
// redirect
//   basic
//   >
//   log
//
// Inside child process
//
// cat <input >ouput
// redirct
//   redirect
//     basic
//       cat
//     <
//     input
//  >
//  output
/*
FUNC : tree_execute_redirect
     : ִ��һ�� �ض��� ָ��, �ض��� ����/���
*/
void tree_execute_redirect(tree_t *this)
{
    tree_t* body = tree_get_child(this, 0);                 /* body cmd */
    tree_t* operator = tree_get_child(this, 1);             /* op > | < */
    tree_t* file = tree_get_child(this, 2);                 /* red file */

    char* path = NULL;
    int fd = 0;
    int redirect_fd = 0;

    path = file->token;
    if (token_is(operator,"<"))                             /* �ض�������,�¿��ļ�FD,ʹ��Ϊ���� */
    {
        fd = open(path, O_RDONLY); 
        redirect_fd = 0;
    }
    else if (token_is(operator, ">"))                       /* �ض������,�´��ļ�FD,ʹ��Ϊ��� */
    {
        fd = creat(path, 0666);
        redirect_fd = 1;
    }

    assert(fd >= 0);
    dup2(fd, redirect_fd);
    close(fd);
    tree_execute(body);
}

#define MAX_ARGC 16
/*
FUNC : tree_execute_basic
     : ִ��һ�����������ȡ ARGC ARGV[]
*/
void tree_execute_basic(tree_t *this)
{
    int argc = 0;
    char* argv[MAX_ARGC];

    int i = 0;
    tree_t* child = NULL;
    vector_each(&this->child_vector, i, child)
        argv[argc++] = child->token;
    argv[argc] = NULL;
    execvp(argv[0], argv);

    perror("exec");
    exit(EXIT_FAILURE);
}

// echo abc | grep b
//
// pipe
//   basic
//     echo
//     abc
//   basic
//     grep
//     b
//
// cmdA | cmdB | cmdC
// pipe
//   pipe 
//     cmdA
//     cmdB
//   cmdC
/*
FUNC : tree_execute_pipe
     : ʵ�ֹܵ������ ��out -> ��in
other : ʵ�ֲ���Ϊ
    f0 - pipe -> f1
    out - pipe -> in
*/
void tree_execute_pipe(tree_t *this)
{
    int fd[2];
    pid_t pid = 0;
    tree_t *l = tree_get_child(this, 0);                /* cmd_l */
    tree_t* r = tree_get_child(this, 1);                /* cmd_r */

    pipe(fd);                                           /* f1 -> f0 */
    pid = fork();
    if (pid == 0)
    {
        close(1);                                       /* 1(std_out) close */
        dup(fd[1]);                                     /* out -> f0 */
        close(fd[0]);                   
        close(fd[1]);
        tree_execute(l);                    
        exit(EXIT_FAILURE);
    }

    close(0);                                           /* 0(std_int) close */
    dup(fd[0]);                                         /* f1 -> in */
    close(fd[0]); 
    close(fd[1]);
    tree_execute(r);
}

// # line 
// "cd /bin"
//
// # tree
// TREE_BASIC
//   cd
//   /bin
/*
FUNC : tree_execute_builtin
     : �ж�һ������(���νṹ)�Ƿ�Ϊ ���� ��������ִ����
*/
int tree_execute_builtin(tree_t *this)
{
    if (this->type != TREE_BASIC)
        return 0;

    int argc = this->child_vector.count;            /* �����������ȡ����0 - �������� */
    tree_t* child0 = tree_get_child(this, 0);
    char* arg0 = child0->token;

    // exit
    if (strcmp(arg0, "exit") == 0)
    {
        exit(EXIT_SUCCESS);
        return 1;
    }

    // pwd
    if (strcmp(arg0, "pwd") == 0)
    {
        char buf[128];
        getcwd(buf, sizeof(buf));
        printf("%s\n", buf);
        return 1;
    }

    // cd 
    if (strcmp(arg0, "cd") == 0)
    {
        if (argc == 1)
        {
            return 1;
        }

        tree_t* child1 = tree_get_child(this, 1);
        char* arg1 = child1->token;
        int error = chdir(arg1);
        if (error < 0)
        {
            perror("cd");
        }
        return 1;
    }

    return 0;
}

// cc a-very-large-file.c &
// async
//   basic
//     cc
//     a-very-large-file
//
// cat file | grep string &
// async
//   pipe
//     basic
//       cat
//       file
//     basic
//       grep
//       string
//
// child count == 1
/*
FUNC : tree_execute_async
     : ִ��һ�� �첽/��̨ ����
other :  ASYNC ���� = ���� + ASYNC����, ��̨������ ������ �����Ƿ�ȴ����, �˴�ֻ��Ҫִ�� ���� ����
*/
void tree_execute_async(tree_t *this)
{
    tree_t* body = tree_get_child(this, 0);
    tree_execute(body);
}

// Inside child process
/*
FUNC : tree_execute
     �� ���� �������� �� ִ��һ�� ��չ����
*/
void tree_execute(tree_t *this)
{
    switch (this->type) {
        case TREE_ASYNC:
            tree_execute_async(this); 
            break;

        case TREE_PIPE:
            tree_execute_pipe(this); 
            break;

        case TREE_REDIRICT:
            tree_execute_redirect(this); 
            break;

        case TREE_BASIC:
              tree_execute_basic(this); 
            break;
    }
}

// Inside parent(shell) process
/*
FUNC : tree_execute_wrapper
	 : ���� ���� �� ��չ ����, ������������,���ú���ֱ��ִ��,������չ����,�����ӽ���ִ��
PARAM ��
	@tree_t *this : ��Ҫִ�е�ָ��(���νṹ) 
*/
void tree_execute_wrapper(tree_t *this)
{
    if (tree_execute_builtin(this))								/* ���Ϊ ����ָ��,ֱ��ִ�� */ 
        return;

    int status;													/* ���Ϊ ��չָ��,�����ӽ���ִ�� */
    pid_t pid = fork();
    if (pid == 0) {
        tree_execute(this);
        exit(EXIT_FAILURE);
    }
   
    // cc a-large-file.c &
    if (this->type != TREE_ASYNC)								/* ���Ϊ �Ǻ�̨���� , �ȴ��ӽ���ִ����� */
    {
        wait(&status);
    }
}

// cmd1 <input | cmd2 | cmd3 >output &
