#include "ush.h"

void mx_set_r_infile(t_shell *m_s, t_job *job, t_process *p) {
    t_redir *r;
    int j = 0;

//    p->r_infile = (int *) malloc(sizeof(int) * (p->c_input));
    p->r_infile = (int *) realloc(p->r_infile, sizeof(int) * (p->c_input));
    p->r_infile[0] = job->infile;
    if (p->redirect) {
        for (r = p->redirect; r; r = r->next) {
            if (r->input_path) {
                if (r->redir_delim == R_INPUT)
                    m_s->redir = mx_red_in(job, p, r->input_path, j);
                if (r->redir_delim == R_INPUT_DBL) {
                    m_s->redir = mx_red_in_d(job, p, r->input_path, j);
                }
                j++;
            }
        }
        job->infile = p->r_infile[0];
    }
}


int mx_red_in(t_job *job, t_process *p, char *input_path, int j) {
    int status_redir = 0;
    int fd;

    if ((fd = open(input_path, O_RDONLY, 0666)) < 0) {
        mx_printerr("ush :");
        perror(input_path);
        status_redir = 1;
        job->exit_code = 1;
        return status_redir;
    }
    p->r_infile[j] = fd;
    return status_redir;
}

int mx_red_in_d(t_job *job, t_process *p, char *input_path, int j) {
    int status_redir = 0;
    int fd;
    char *line;
    int count;

    if ((fd = open(input_path, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0 ) {
        mx_printerr("ush :");
        perror(input_path);
        status_redir = 1;
        job->exit_code = 1;
        return status_redir;
    }
    line = strdup("");
    count = 0;
    while (strcmp(line, input_path) != 0) {
        p->pipe ? printf("pipe heredoc> ") : printf("heredoc> ");
        write(fd, line, mx_strlen(line));
        free(line);  ////////////////////
        if (count)
            write(fd, "\n", 1);
//        line = mx_ush_read_line(m_s);
        count++;
    }
    free(line);
    close(fd);
    p->r_infile[j] = open(input_path, O_RDONLY, 0666);
    remove(input_path);
    return status_redir;
}



