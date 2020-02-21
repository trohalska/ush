#include "ush.h"

static char *get_result(char *buff1, char *buff2,  char *replace) {
    char *tmp = NULL;
    char *res = NULL;

    tmp = strdup(buff1);
    res = mx_strjoin(tmp, replace);
    free(tmp);
    tmp = strdup(res);
    free(res);
    res = mx_strjoin(tmp, buff2);
    free(tmp);
    return res;
}

static char *replace_substr(char *str,  char *sub, char *replace) {
    char *res = str;
    char *buff1 = mx_strnew(mx_strlen(str));
    char *buff2 = mx_strnew(mx_strlen(str));
    while(mx_strstr(res,sub) != NULL) {
        int i = mx_get_substr_index(res,sub);
        mx_strncpy(buff1, res, i);
        for(int j = 0; j < i + mx_strlen(sub); j++)
            res++;
        mx_strcpy(buff2,res);
        //free(res);
        res = get_result(buff1, buff2, replace);
    }
    free(buff1);
    free(buff2);
    return res;
}

static void rep_x(int *i, int *len, char *res, char *str) {
    if (str[*i] == '\\' && str[*i + 1] == 'e') {
        if (str[*i + 2] != '\\')
            (*i) += 3;
        else
            (*i) += 2;
    }
    if (str[*i] == '\\' && str[*i + 1] == 'x') {
        if (!str[*i + 2])
            (*i) += 2;
        else {
            char rep = mx_hex_to_nbr(strndup(str + *i + 2, 2));
            res[*len] = rep;
            (*len)++;
            i += 4;
        }
    }
}

static char *replace_slash(char *str, echo_t *echo_options) {
    char *res = (char *)malloc(mx_strlen(str));
    int len = 0;

    for (int i = 0; i < mx_strlen(str); i++) {
        if (str[i] == '\\' && str[i + 1] == '\\')
            i++;
        rep_x(&i, &len, res, str);
        if (str[i] == '\\' && str[i + 1] == 'c') {
            echo_options->n = 1;
            break;
        }
        res[len] = str[i];
        len++;
    }
    res[len] = '\0';
    
    return res;
}

			
void mx_escape_seq(t_process *p, int i, echo_t echo_options) {
    char *tmp = replace_slash(p->argv[i], &echo_options);
    char *sequenses[] = {"\\a","\\b","\\f","\\n","\\r","\\t","\\v",NULL};
    char *escape[] = {"\a","\b","\f","\n","\r","\t","\v",NULL};

    free(p->argv[i]);
    p->argv[i] = strdup(tmp);
    free(tmp);
    for (int j = 0; sequenses[j] != NULL; j++) {
        if (strstr(p->argv[i],sequenses[j])){
            char *tmp = replace_substr(p->argv[i],sequenses[j], escape[j]);
            free(p->argv[i]);
            p->argv[i] = strdup(tmp);
            free(tmp);
        }
    }
}
