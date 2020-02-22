#include "ush.h"

static char *check_path(char **arr, char *command);
static char *get_error(char **name, char *command, int *status);
static void print_error(char *command, char *error);
static void child_work(t_shell *m_s, t_process *p, int job_id, int child_pid);

int mx_launch_process(t_shell *m_s, t_process *p, int job_id) {
//    int status = 0;
    pid_t child_pid;
    p->status = MX_STATUS_RUNNING;
    int shell_is_interactive = isatty(STDIN_FILENO);  //!!
    child_pid = fork();
    if (child_pid < 0) {
        perror("error fork");
        exit(1);
    }
    else if (child_pid == 0)
        child_work(m_s, p, job_id, child_pid);
    else {
        p->pid = child_pid;
        if (shell_is_interactive) {
            pid_t pid = child_pid;
            if (m_s->jobs[job_id]->pgid == 0)
                m_s->jobs[job_id]->pgid = pid;
            setpgid (pid, m_s->jobs[job_id]->pgid);
        }
    }
    return p->status >> 8;  //WEXITSTATUS(status)
}


static void child_work(t_shell *m_s, t_process *p, int job_id, int child_pid) {
    int shell_is_interactive = isatty(STDIN_FILENO);

    if (shell_is_interactive)
        mx_pgid(m_s, job_id, child_pid);
    mx_dup_fd(p);
    char **arr = mx_strsplit(m_s->jobs[job_id]->path, ':');
    char *command = p->argv[0];
    m_s->jobs[job_id]->path  = check_path(arr, command);
    mx_del_strarr(&arr);
    char *error = get_error(&m_s->jobs[job_id]->path, command, &p->status);
    if (execve(m_s->jobs[job_id]->path, p->argv, m_s->jobs[job_id]->env) < 0) {
        print_error(command, error);
        free(error);
        free(m_s->jobs[job_id]->path);
        _exit(p->status);
    }
    free(m_s->jobs[job_id]->path);
    free(error);
    exit(p->status);
}

static void read_dir(char *dir, char *command, int *flag, char **name) {
    DIR *dptr  = opendir(dir);
    struct dirent  *ds;

    if (dptr != NULL) {
        while ((ds = readdir(dptr)) != 0) {
            if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                (*flag)++;
                char *tmp = mx_strjoin(dir, "/");
                *name = mx_strjoin(tmp, command);
                free(tmp);
                break;
            }
        }
        closedir(dptr);
    }
}

static char *check_path(char **arr, char *command) {
    int i = 0;
    char *name = NULL;
    int flag = 0;

    while (arr[i] != NULL && !flag) {
        read_dir(arr[i], command, &flag, &name);
        i++;
    }
    return name;
}

static char *get_error(char **name, char *command, int *status) {
    char *error = NULL;

    *status = 127;
    if (strstr(command, "/")) {
        *name = command;
        struct stat buff;
        if (lstat(*name, &buff) < 0) {
            error = NULL;//strdup(": No such file or directory\n");
        }
        else {
            if (mx_get_type(buff) == 'd') {
                error = strdup(": is a directory\n");
                *status = 126;
            }
        }
    }
    else
        error = strdup(": command not found\n");
    return error;
}

static void print_error(char *command, char *error) {
    mx_printerr("ush: ");
    if (error) {
        mx_printerr(command);
        mx_printerr(error);
    }
    else
        perror(command);
}
