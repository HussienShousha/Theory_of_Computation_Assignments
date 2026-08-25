// IDs and Names:
// 20226035 - Hussien Ahmed Abdel-Wahab
// 20226038 - Khaled Ahmed Mohamed
// 20226018 - Andrew Akram William
// 20226019 - Andrew Makram Ishak

// Extended BNF (EBNF) Grammar:
// ----------------------------
// The expressions are from the group operations on pages 459-460 of James Hein Book.
// Variables are: x, y, z  (elements of a group)
// Operations: . (product), ^-1 (inverse)
//
// <expr>    -> <term>
// <term>    -> <factor> { . <factor> }
// <factor>  -> <newexpr> { ^-1 }
// <newexpr> -> ( <expr> ) | id
//
// Notes:
// - '.' is the group product operator (left associative)
// - '^-1' is the inverse operator (postfix, applied to a newexpr)
// - 'id' is one of: x, y, z or even any single character (A - Z) or (a - z)

#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

struct InFile
{
    FILE* file;

    InFile(const char* str)
    {
        file = fopen(str, "r");
    }

    ~InFile()
    {
        if (file)
        {
            fclose(file);
        }
    }

    char GetNextTokenChar()
    {
        int ch = fgetc(file);
        if (ch == EOF)
        {
            return 0;
        }
        return ch;
    }
};

struct OutFile
{
    FILE* file;

    OutFile(const char* str)
    {
        file = 0;
        if (str)
        {
            file = fopen(str, "w");
        }
    }

    ~OutFile()
    {
        if (file)
        {
            fclose(file);
        }
    }

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s);
        fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
                : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType
{
    ID,
    DOT, POWER, MINUS, ONE,
    LEFT_PAREN, RIGHT_PAREN,
    ERROR, ENDFILE
};

const char* TokenTypeStr[] =
{
    "ID",
    "DOT", "POWER", "MINUS", "ONE",
    "LeftParen", "RightParen",
    "Error", "EndFile"
};

struct Token
{
    TokenType type;
    char ch;

    Token()
    {
        ch = 0;
        type = ERROR;
    }

    Token(TokenType _type, const char _ch)
    {
        type = _type;
        ch = _ch;
    }
};

const Token symbolic_tokens[] =
{
    Token(ID, 0),
    Token(DOT, '.'),
    Token(POWER, '^'),
    Token(MINUS, '-'),
    Token(ONE, '1'),
    Token(LEFT_PAREN, '('),
    Token(RIGHT_PAREN, ')')
};

const int num_symbolic_tokens = sizeof(symbolic_tokens) / sizeof(symbolic_tokens[0]);

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type = ERROR;
    ptoken->ch = 0;

    char s;
    do
    {
        s = pci->in_file.GetNextTokenChar();
        if (s == 0)
        {
            ptoken->type = ENDFILE;
            return;
        }
    } while (s == ' ' || s == '\n' || s == '\r' || s == '\t');

    for (int i = 0; i < num_symbolic_tokens; i++)
    {
        if (s == symbolic_tokens[i].ch)
        {
            ptoken->type = symbolic_tokens[i].type;
            ptoken->ch = s;
            return;
        }
    }

    if ((s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z'))
    {
        ptoken->type = ID;
        ptoken->ch = s;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Parser //////////////////////////////////////////////////////////////////////////

// <expr>    -> <term>
// <term>    -> <factor> { . <factor> }
// <factor>  -> <newexpr> { ^-1 }
// <newexpr> -> ( <expr> ) | id

enum NodeKind
{
    OPER_NODE, ID_NODE
};

const char* NodeKindStr[] =
{
    "Oper", "ID"
};

#define MAX_CHILDREN 2

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    NodeKind node_kind;
    char id;
    TokenType oper;

    TreeNode()
    {
        for (int i = 0; i < MAX_CHILDREN; i++)
        {
            child[i] = 0;
        }
    }
};

struct ParseInfo
{
    Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_token_type)
{
    if (ppi->next_token.type != expected_token_type)
    {
        throw 0;
    }
    GetNextToken(pci, &ppi->next_token);
}

TreeNode* Expr(CompilerInfo*, ParseInfo*);

// <newexpr> -> ( <expr> ) | id
TreeNode* NewExpr(CompilerInfo* pci, ParseInfo* ppi)
{
    if (ppi->next_token.type == ID)
    {
        TreeNode* tree = new TreeNode;
        tree->node_kind = ID_NODE;
        tree->id = ppi->next_token.ch;
        Match(pci, ppi, ID);
        return tree;
    }

    if (ppi->next_token.type == LEFT_PAREN)
    {
        Match(pci, ppi, LEFT_PAREN);
        TreeNode* tree = Expr(pci, ppi);
        Match(pci, ppi, RIGHT_PAREN);
        return tree;
    }

    throw 0;
}

// <factor> -> <newexpr> { ^-1 }
TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree = NewExpr(pci, ppi);

    // allow multiple ^-1^-1^-1 ...
    while (ppi->next_token.type == POWER)
    {
        TreeNode* new_tree = new TreeNode;
        new_tree->node_kind = OPER_NODE;
        new_tree->oper = POWER;
        new_tree->child[0] = tree;

        Match(pci, ppi, POWER);
        Match(pci, ppi, MINUS);
        Match(pci, ppi, ONE);

        tree = new_tree;
    }

    return tree;
}

// <term> -> <factor> { . <factor> }
TreeNode* Term(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree = Factor(pci, ppi);

    while (ppi->next_token.type == DOT)
    {
        TreeNode* new_tree = new TreeNode;
        new_tree->node_kind = OPER_NODE;
        new_tree->oper = DOT;
        new_tree->child[0] = tree;

        Match(pci, ppi, DOT);

        new_tree->child[1] = Factor(pci, ppi);
        tree = new_tree;
    }

    return tree;
}

// <expr> -> <term>
TreeNode* Expr(CompilerInfo* pci, ParseInfo* ppi)
{
    return Term(pci, ppi);
}

TreeNode* Parse(CompilerInfo* pci)
{
    ParseInfo parse_info;
    GetNextToken(pci, &parse_info.next_token);

    TreeNode* syntax_tree = Expr(pci, &parse_info);

    // Error handling: ensure the entire input was consumed
    if (parse_info.next_token.type != ENDFILE)
    {
        throw 0;
    }

    return syntax_tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Print Tree //////////////////////////////////////////////////////////////////////

void PrintTree(TreeNode* node, int level = 0)
{
    if (node->node_kind == ID_NODE)
    {
        printf("%c\n", node->id);
    }
    else if (node->oper == DOT)
    {
        printf("product\n");
    }
    else
    {
        printf("inverse\n");
    }

    if (node->child[0])
    {
        for (int i = 0; i < level; i++)
        {
            printf("   ");
        }
        printf("|--");
        PrintTree(node->child[0], level + 1);
    }

    if (node->child[1])
    {
        for (int i = 0; i < level; i++)
        {
            printf("   ");
        }
        printf("|--");
        PrintTree(node->child[1], level + 1);
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Destroy Tree ////////////////////////////////////////////////////////////////////

void DestroyTree(TreeNode* node)
{
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        if (node->child[i])
        {
            DestroyTree(node->child[i]);
        }
    }
    delete node;
}

////////////////////////////////////////////////////////////////////////////////////
// Apply inference rules ///////////////////////////////////////////////////////////

// Rule: (A . B)^-1  ->  B^-1 . A^-1
TreeNode* ApplyInverseProduct(TreeNode* node, int& changed)
{
    if (node == NULL)
    {
        return node;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = ApplyInverseProduct(node->child[i], changed);
    }

    if (node->node_kind == OPER_NODE && node->oper == POWER)
    {
        TreeNode* inside = node->child[0];

        if (inside && inside->node_kind == OPER_NODE && inside->oper == DOT)
        {
            TreeNode* A = inside->child[0];
            TreeNode* B = inside->child[1];

            TreeNode* invB = new TreeNode;
            invB->node_kind = OPER_NODE;
            invB->oper = POWER;
            invB->child[0] = B;

            TreeNode* invA = new TreeNode;
            invA->node_kind = OPER_NODE;
            invA->oper = POWER;
            invA->child[0] = A;

            TreeNode* result = new TreeNode;
            result->node_kind = OPER_NODE;
            result->oper = DOT;
            result->child[0] = invB;
            result->child[1] = invA;

            inside->child[0] = 0;
            inside->child[1] = 0;
            delete inside;
            node->child[0] = 0;
            delete node;

            changed = 1;
            return result;
        }
    }

    return node;
}

// Rule: (A^-1)^-1  ->  A
TreeNode* RuleDoubleInverse(TreeNode* node, int& changed)
{
    if (node == NULL)
    {
        return node;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = RuleDoubleInverse(node->child[i], changed);
    }

    if (node->node_kind == OPER_NODE && node->oper == POWER)
    {
        TreeNode* inside = node->child[0];

        if (inside && inside->node_kind == OPER_NODE && inside->oper == POWER)
        {
            TreeNode* result = inside->child[0];

            inside->child[0] = 0;
            delete inside;
            node->child[0] = 0;
            delete node;

            changed = 1;
            return result;
        }
    }

    return node;
}

// Rule: A . e  ->  A   and   e . A  ->  A
TreeNode* RuleIdentity(TreeNode* node, int& changed)
{
    if (node == NULL)
    {
        return node;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = RuleIdentity(node->child[i], changed);
    }

    if (node->node_kind == OPER_NODE && node->oper == DOT)
    {
        TreeNode* A = node->child[0];
        TreeNode* B = node->child[1];

        // x . e -> x
        if (B && B->node_kind == ID_NODE && B->id == 'e')
        {
            node->child[0] = 0;
            node->child[1] = 0;
            delete node;
            delete B;
            changed = 1;
            return A;
        }

        // e . x -> x
        if (A && A->node_kind == ID_NODE && A->id == 'e')
        {
            node->child[0] = 0;
            node->child[1] = 0;
            delete node;
            delete A;
            changed = 1;
            return B;
        }
    }

    return node;
}

// Rule: x^-1 . (x . y)  ->  y   and   (x . y) . y^-1  ->  x
TreeNode* RuleCancellation(TreeNode* node, int& changed)
{
    if (node == NULL)
    {
        return node;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = RuleCancellation(node->child[i], changed);
    }

    if (node->node_kind == OPER_NODE && node->oper == DOT)
    {
        TreeNode* A = node->child[0];
        TreeNode* B = node->child[1];

        // x^-1 . (x . y) -> y
        if (A && A->node_kind == OPER_NODE && A->oper == POWER)
        {
            TreeNode* x = A->child[0];

            if (B && B->node_kind == OPER_NODE && B->oper == DOT)
            {
                TreeNode* B1 = B->child[0];
                TreeNode* B2 = B->child[1];

                if (B1 && B1->node_kind == ID_NODE &&
                    x && x->node_kind == ID_NODE &&
                    B1->id == x->id)
                {
                    B->child[1] = 0;
                    node->child[0] = 0;
                    node->child[1] = 0;
                    delete node;
                    DestroyTree(A);
                    B->child[0] = 0;
                    delete B;
                    delete B1;
                    changed = 1;
                    return B2;
                }
            }
        }

        // (x . y) . y^-1 -> x
        if (B && B->node_kind == OPER_NODE && B->oper == POWER)
        {
            TreeNode* y = B->child[0];

            if (A && A->node_kind == OPER_NODE && A->oper == DOT)
            {
                TreeNode* A1 = A->child[0];
                TreeNode* A2 = A->child[1];

                if (A2 && A2->node_kind == ID_NODE &&
                    y && y->node_kind == ID_NODE &&
                    A2->id == y->id)
                {
                    A->child[0] = 0;
                    node->child[0] = 0;
                    node->child[1] = 0;
                    delete node;
                    DestroyTree(B);
                    A->child[1] = 0;
                    delete A;
                    delete A2;
                    changed = 1;
                    return A1;
                }
            }
        }
    }

    return node;
}

// Rule: (x . y) . z  ->  x . (y . z)
TreeNode* RuleAssociativity(TreeNode* node, int& changed)
{
    if (node == NULL)
    {
        return node;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = RuleAssociativity(node->child[i], changed);
    }

    if (node->node_kind == OPER_NODE && node->oper == DOT)
    {
        TreeNode* A = node->child[0];

        if (A && A->node_kind == OPER_NODE && A->oper == DOT)
        {
            TreeNode* x = A->child[0];
            TreeNode* y = A->child[1];
            TreeNode* z = node->child[1];

            TreeNode* newRight = new TreeNode;
            newRight->node_kind = OPER_NODE;
            newRight->oper = DOT;
            newRight->child[0] = y;
            newRight->child[1] = z;

            TreeNode* newRoot = new TreeNode;
            newRoot->node_kind = OPER_NODE;
            newRoot->oper = DOT;
            newRoot->child[0] = x;
            newRoot->child[1] = newRight;

            A->child[0] = 0;
            A->child[1] = 0;
            delete A;
            node->child[0] = 0;
            node->child[1] = 0;
            delete node;

            changed = 1;
            return newRoot;
        }
    }

    return node;
}

////////////////////////////////////////////////////////////////////////////////////
// Simplify ////////////////////////////////////////////////////////////////////////

void Simplify(TreeNode*& root)
{
    int step = 0;
    int changed = 1;

    printf("Step %d:\n", step++);
    PrintTree(root);
    printf("\n");

    while (changed)
    {
        changed = 0;

        root = ApplyInverseProduct(root, changed);
        root = RuleDoubleInverse(root, changed);
        root = RuleIdentity(root, changed);
        root = RuleCancellation(root, changed);
        root = RuleAssociativity(root, changed);

        if (changed)
        {
            printf("Step %d:\n", step++);
            PrintTree(root);
            printf("\n");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Test Cases //////////////////////////////////////////////////////////////////////


void RunTestCase(int case_num, const char* expr)
{
    printf("Test %d: %s\n", case_num, expr);

    FILE* f = fopen("input.txt", "w");
    fprintf(f, "%s", expr);
    fclose(f);

    CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

    TreeNode* syntax_tree = 0;

    try
    {
        syntax_tree = Parse(&compiler_info);
        Simplify(syntax_tree);
        DestroyTree(syntax_tree);
    }
    catch (...)
    {
        if (syntax_tree)
        {
            DestroyTree(syntax_tree);
        }
        printf("Parse error\n");
    }

    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////////

int main()
{
    int choice;
    printf("1) Hardcoded tests\n2) Read from input.txt\nChoice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        const char* test_cases[] =
        {
            "x",
            "y",
            "z",
            "x.y",
            "y.z",
            "x.y.z",
            "x^-1",
            "y^-1",
            "z^-1",
            "x.y^-1",
            "x^-1.y",
            "(x.y)^-1",
            "(x^-1.y)^-1",
            "(x.y.z)^-1",
            "((x.y).z)^-1",
            "(x.(y.z))^-1",
            "((x.y^-1).z)^-1",
            "((x^-1.y).(z.x^-1))^-1",
            "(x.(y^-1.z))^-1",
            "((x.y).(z^-1.x))^-1",
            "((x.(y.z^-1)).(x.y))^-1",
            "x^-1^-1",
            "x^-1^-1",
            "(((x.y)^-1)^-1)^-1",
            "(x.y.z.x.y)^-1",
            "((x^-1.y).(z.x^-1))^-1",
            "x^-1.(x.y)",
            "(x.y).y^-1",
            "x.e",
            "e.x",
            "(((x.y).(z.x)).((y.z).x))^-1",
            "x^-1^-1^-1^-1",
            "((x.(y^-1.z)).(x.y))^-1",
            "(((x.y).z).(x.(y.z)))^-1",
            "(x^-1.(x.(y.z)))^-1",
            "(x.x^-1.y.y^-1.z)^-1",
            "(x.y.z.x.y.z)^-1",
            "((((x.y)^-1).z)^-1)^-1",
            "((x^-1.(y.(z.x))).((y.z)^-1))^-1"
        };

        int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

        for (int i = 0; i < num_cases; i++)
        {
            RunTestCase(i + 1, test_cases[i]);
        }
    }

    else if (choice == 2)
    {
        FILE* f = fopen("input.txt", "r");

        if (!f)
        {
            f = fopen("input.txt", "w");
            printf("input.txt created. Please add an expression.\n");
        }

        fclose(f);

        CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

        TreeNode* syntax_tree = 0;

        try
        {
            syntax_tree = Parse(&compiler_info);
            Simplify(syntax_tree);
            DestroyTree(syntax_tree);
        }
        catch (...)
        {
            if (syntax_tree)
            {
                DestroyTree(syntax_tree);
            }
            printf("Parse error\n");
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
