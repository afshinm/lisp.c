#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>


#define MAX_TOKEN_SIZE 64
#define MAX_TOKENS_SIZE 128
#define MAX_SYMBOL_SIZE 32

#define error(_msg_, ...)                                                     \
    {                                                                         \
        fprintf(stderr, "panic: %s:%d:%s(): " _msg_ "\n", __FILE__, __LINE__, \
                __FUNCTION__, __VA_ARGS__);                                   \
        exit(1);                                                              \
    }

typedef struct Obj
{
    enum ObjType
    {
        Symbol,
        Primitive,
        List,
        Cell,
        Char,
        Integer
    } type;

    struct Obj *data;
    struct Obj *next;
    char *stringValue;
    int integerValue;
    struct Obj *(*pfn)(struct Obj *);
} Obj;

static Obj *Symbols = 0;

// TODO: should we add period "." as well?
char* VALID_SYMBOLS = "+-*/@$%^&_=<>~";

char **tokenize(char *program)
{
    char current_word[MAX_TOKEN_SIZE];
    memset(current_word, 0, strlen(current_word));

    char **tokens = malloc(MAX_TOKENS_SIZE * sizeof(char *));
    int i = 0;

    while (*program)
    {
        char token = *program++;

        if (token == ' ' || token == '(' || token == ')')
        {
            if (strlen(current_word) > 0)
            {
                tokens[i] = malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[i++], current_word);
                memset(current_word, 0, strlen(current_word));
            }

            if (token == '(' || token == ')')
            {
                tokens[i++] = strdup(&token);
            }
        }
        else
        {
            strncat(current_word, &token, 1);
        }
    }

    return tokens;
}

Obj *make_obj(int type)
{
    Obj* obj = calloc(1, sizeof(Obj));
    obj->type = type;
    return obj;
}

Obj *make_atom(char *token)
{
    Obj* atom = make_obj(Symbol);
    atom->stringValue = token;
    return atom;
}

Obj *make_primitive(Obj *(*fn)(Obj *))
{
    Obj *obj = make_obj(Primitive);
    obj->pfn = fn;
    return obj;
}

Obj *cons(Obj *car, Obj *cdr)
{
    Obj *pair = make_obj(Cell);
    pair->data = car;
    pair->next = cdr;
    return pair;
}

Obj *car(Obj *obj)
{
    return obj->data;
}

Obj *cdr(Obj *obj)
{
    return obj->next;
}

Obj *intern(char *name)
{
    for (Obj *p = Symbols; p; p = cdr(p))
        if (car(p)->type == Symbol && strcmp(name, car(p)->stringValue) == 0)
            return car(p);

    Obj *new_symbol = make_atom(name);
    Symbols = cons(new_symbol, Symbols);
    return car(Symbols);
}

Obj* assoc(Obj *exp, Obj* env)
{
    while (env != 0) {
        if (car(car(env)) == exp) {
            return cdr(car(env));
        }
        env = cdr(env);
    }
    error("error: cannot find symbol: %s\n", exp->stringValue);
}

int obj_length(Obj* obj) {
    int len = 0;

    while (obj) {
        len++;
        obj = cdr(obj);
    }

    return len;
}

Obj *read_from_tokens(char **tokens)
{
    char *token = *tokens;

    if (*token == '(')
    {
        Obj *list;
        Obj *root;
        list = make_obj(List);
        root = list;

        while (token && *token != ')') {
            Obj* obj = read_from_tokens(++tokens);
            list->next = obj;
            list = list->next;
            token = *(tokens + obj_length(obj));
        }

        // pop off ")"
        token++;

        return root;
    }
    else if (isdigit(*token))
    {
        Obj* integer = make_obj(Integer);
        integer->integerValue = atoi(token);
        return integer;
    }
    else
    {
        return intern(token);
    }
}

char peek()
{
    char c = getchar();
    ungetc(c, stdin);
    return c;
}

int read_int(const char c)
{
    char buff[1024] = { c };
    int i = 1;
    while(isdigit(peek())) {
        buff[i] = getchar();
        ++i;
    }

    return atoi(buff);
}

Obj *prim_car(Obj *obj) {
    return car(car(obj));
}

Obj *prim_cdr(Obj *obj) {
    return car(cdr(obj));
}

Obj *prim_plus(Obj *obj) {
    Obj* result = make_obj(Integer);
    while (obj) {
        result->integerValue = result->integerValue + car(obj)->integerValue;
        obj = cdr(obj);
    }
    return result;
}

Obj *eval(Obj *exp, Obj *env);

Obj *evlist(Obj *list, Obj *env)
{
    if (list != 0)
    {
        return cons(
            eval(list, env),
            evlist(cdr(list), env));
    }

    return 0;
}

Obj *eval(Obj *exp, Obj *env) {
    if (exp->type == Symbol) {
        return assoc(exp, env);
    }

    if (exp->type == Integer) {
        return exp;
    }

    if (exp->type == Primitive) {
        return exp;
    }

    if (exp->type == List) {
        Obj* pfn = eval(cdr(exp), env);
        return pfn->pfn(evlist(cdr(cdr(exp)), env));
    }
}

int main(void)
{
    char **tokens = tokenize("(+ 3 (+ 5 5))");

    Obj *env = cons(
        cons(intern("car"), make_primitive(prim_car)),
        cons(cons(intern("+"), make_primitive(prim_plus)),
             0));

    Obj* obj = read_from_tokens(tokens);

    Obj* evald = eval(obj, env);

    printf('eval type: %s', evald->type);
}
