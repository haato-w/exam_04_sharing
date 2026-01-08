#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct node {
    enum {
        ADD,
        MULTI,
        VAL
    }   type; // typeの定義。ADD, MULTI, VALのどれかが入っている。
    int val; // typeがvalの時に値(value)が入っている。
    struct node *l; // 左の子要素のポインタ
    struct node *r; // 右の子要素のポインタ
}   node;


// nodeをヒープ領域に確保する関数。
// 内容は引数nに初期化する。
node    *new_node(node n)
{
    node *ret = calloc(1, sizeof(n));
    if (!ret)
        return (NULL);
    *ret = n;
    return (ret);
}

// ツリーをfreeする関数。
// 一番上のnodeを渡せば全部消してくれる。
void    destroy_tree(node *n)
{
    if (!n)
        return ;
    if (n->type != VAL)
    {
        destroy_tree(n->l);
        destroy_tree(n->r);
    }
    free(n);
}



// エラー文を出す関数。
void    unexpected(char c)
{
    if (c)
        printf("Unexpected token '%c'\n", c);
    else
        printf("Unexpected end of input\n"); //+++++++++++++++
}

// char **sの内容がchar cであればsのポインタを一つ進めて1を返す。
// そうでなければ0を返す。
int accept(char **s, char c)
{
    if (**s == c)   //+++++++++++++++
    {
        (*s)++;
        return (1);
    }
    return (0);
}

// char **sの内容がchar cであればsのポインタを一つ進めて1を返す。
// そうでなければエラー文を出力して0を返す。
int expect(char **s, char c)
{
    if (accept(s, c))
        return (1);
    unexpected(**s);
    return (0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 以下、ツリーを構築する関数。
// parse_addition → parse_multiplication → parse_number_or_group → parse_addition → ...
// のように循環していく。
// この関数呼び出しの順番は計算の優先度が (値, カッコ) → (掛け算) → (足し算) の順になっていることによる。
// つまり、additionの前にmultiplicationの確認をし、multiplicationの前に値とカッコの確認をする、カッコの中でまた add, multi, value or カッコ の確認...
// というように計算している。

int      check_balance(char *s);
node    *parse_number_or_group(char **s);
node    *parse_addition(char **s);
node    *parse_multiplication(char **s);


node    *parse_number_or_group(char **s)
{
    node    *res;
    node    tmp;

    res = NULL;
    if (**s == '(')
    {
        (*s)++;
        res = parse_addition(s);
        if (!res || **s != ')')
        {
            destroy_tree(res);
            unexpected(**s);
            return (NULL);
        }
        (*s)++;
        return (res);
    }
    if (isdigit(**s))
    {
        tmp.type = VAL;
        tmp.val = **s - '0';
        res = new_node(tmp);
        (*s)++;
        return (res);
    }
    unexpected(**s);
    return (NULL);
}

node    *parse_addition(char **s)
{
    node    *left;
    node    *right;
    node    tmp;

    // 掛け算かvalue, カッコが無いか確認する
    // 例：2 + 5 の場合、leftには 2 の node が入る。
    // 例：(1 + 2) + (2 * 4) の場合、leftには (1 + 2) の node が入る。
    left = parse_multiplication(s);
    if (!left)
        return (NULL);
    // 次の文字が + なら計算する
    // (1 + 5) + 2 + 3 + ... のように羅列している場合、ここで全部の
    while (**s == '+')
    {
        (*s)++;
        // 右に掛け算かvalue, カッコが無いか確認する。
        right = parse_multiplication(s);
        if (!right)
        {
            destroy_tree(left);
            return (NULL);
        }
        tmp.type = ADD;
        tmp.l = left;
        tmp.r = right;
        left = new_node(tmp);
    }
    return (left);
}


node    *parse_multiplication(char **s)
{
    node    *left;
    node    *right;
    node    tmp;

    left = parse_number_or_group(s);
    if (!left)
        return (NULL);
    while (**s == '*')
    {
        (*s)++;
        right = parse_number_or_group(s);
        if (!right)
        {
            destroy_tree(left);
            return (NULL);
        }
        tmp.type = MULTI;
        tmp.l = left;
        tmp.r = right;
        left = new_node(tmp);
    }
    return (left);
}

// カッコが崩壊していないかチェックする関数。
// ツリーを構築する前に確認しておく。
int check_balance(char *s)
{
    int balance;
    int i;

    balance = 0;
    i = 0;
    while (s[i])
    {
        if (s[i] == '(')
            balance++;
        else if (s[i] == ')')
        {
            balance--;
            if (balance < 0)
                return (-1);
        }
        i++;
    }
    return (balance);
}

// ツリーを探索して計算する関数。
// 一番上のnodeを入れると最終的な計算結果の値が返ってくる。
int eval_tree(node *tree)
{
    switch (tree->type)
    {
        case ADD:
            return (eval_tree(tree->l) + eval_tree(tree->r));
        case MULTI:
            return (eval_tree(tree->l) * eval_tree(tree->r));
        case VAL:
            return (tree->val);
    }
    return (0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return (1);
    if (check_balance(argv[1]) == -1)
        return(printf("Unexpected token ')'"), 1);
    node *tree = parse_addition(&argv[1]);
    if (!tree)
        return (1);
    printf("%d\n", eval_tree(tree));
    destroy_tree(tree);
}
