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

    struct
    {
        struct Obj *car;
        struct Obj *cdr;
    };

    struct Obj *(*pfn)(struct Obj *);

    char *stringValue;
    
    struct Obj *next;
} Obj;

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
    // TODO: check if token is integer

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
    pair->car = car;
    pair->cdr = cdr;
    return pair;
}

Obj *car(Obj *obj)
{
    return obj->car;
}

Obj *cdr(Obj *obj)
{
    return obj->cdr;
}

Obj *intern(Obj *env, char *name)
{
    for (Obj *p = env; p; p = p->cdr)
        if (strcmp(name, p->car->stringValue) == 0)
            return car(p);

    Obj *new_symbol = make_atom(name);
    env = cons(new_symbol, env);
    return car(env);
}

Obj *read_from_tokens(char **tokens)
{
    char *token = *tokens;

    if (strcmp("(", token) == 0)
    {
        Obj *list;
        Obj *root;
        list = make_obj(List);
        root = list;

        while (token && *token != ')') {
            list->next = read_from_tokens(++tokens);
            list = list->next;
            token = *(tokens + 1);
        }

        // pop off ")"
        token++;

        return root;
    }
    else
    {
        return make_atom(token);
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

int main(void)
{
    char **tokens = tokenize("(+ 3 5)");

    Obj* obj = read_from_tokens(tokens);

    //Obj *root = &(Obj){EnvType};
    //parse(root, tokens);

    freopen("test.txt", "r", stdin);

    Obj *env = 0;

    env = cons(cons(intern(env, "car"), cons(make_primitive(prim_car), 0)), 0);
}
