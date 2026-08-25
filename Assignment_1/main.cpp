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
// <factor>  -> <newexpr> [ ^-1 ]
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

    InFile(const char* str) {file=fopen(str, "r");}
    ~InFile(){if(file) fclose(file);}

    char GetNextTokenChar()
    {
        int ch=fgetc(file);
        if(ch==EOF) return 0;
        return ch;
    }
};

struct OutFile
{
    FILE* file;

    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
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

enum TokenType{
                ID,
                DOT, POWER, MINUS, ONE,
                LEFT_PAREN, RIGHT_PAREN,
                ERROR, ENDFILE
              };

const char* TokenTypeStr[]=
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

    Token(){ch=0; type=ERROR;}
    Token(TokenType _type, const char _ch) {type=_type; ch=_ch;}
};

const Token symbolic_tokens[]=
{
    Token(ID, 0),
    Token(DOT, '.'),
    Token(POWER, '^'),
    Token(MINUS, '-'),
    Token(ONE, '1'),
    Token(LEFT_PAREN, '('),
    Token(RIGHT_PAREN, ')')
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->ch=0;

    char s;
    do
    {
        s=pci->in_file.GetNextTokenChar();
        if(s==0)
        {
            ptoken->type=ENDFILE;
            return;
        }
    } while(s==' ' || s=='\n' || s=='\r' || s=='\t');

    for(int i=0;i<num_symbolic_tokens;i++)
    {
        if(s==symbolic_tokens[i].ch)
        {
            ptoken->type=symbolic_tokens[i].type;
            ptoken->ch=s;
            return;
        }
    }

    if((s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z'))
    {
        ptoken->type=ID;
        ptoken->ch=s;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Parser //////////////////////////////////////////////////////////////////////////

// <expr>    -> <term>
// <term>    -> <factor> { . <factor> }
// <factor>  -> <newexpr> [ ^-1 ]
// <newexpr> -> ( <expr> ) | id

enum NodeKind{
                OPER_NODE, ID_NODE
             };

const char* NodeKindStr[]=
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

    TreeNode() {for(int i=0;i<MAX_CHILDREN;i++) child[i]=0;}
};

struct ParseInfo
{
    Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_token_type)
{
    if(ppi->next_token.type!=expected_token_type) throw 0;
    GetNextToken(pci, &ppi->next_token);
}

TreeNode* Expr(CompilerInfo*, ParseInfo*);

// <newexpr> -> ( <expr> ) | id
TreeNode* NewExpr(CompilerInfo* pci, ParseInfo* ppi)
{
    if(ppi->next_token.type==ID)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=ID_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci, ppi, ID);
        return tree;
    }

    if(ppi->next_token.type==LEFT_PAREN)
    {
        Match(pci, ppi, LEFT_PAREN);
        TreeNode* tree=Expr(pci, ppi);
        Match(pci, ppi, RIGHT_PAREN);
        return tree;
    }

    throw 0;
}

// <factor> -> <newexpr> [ ^-1 ]
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
    TreeNode* tree=Factor(pci, ppi);

    while(ppi->next_token.type==DOT)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=OPER_NODE;
        new_tree->oper=DOT;

        new_tree->child[0]=tree;

        Match(pci, ppi, DOT);

        new_tree->child[1]=Factor(pci, ppi);

        tree=new_tree;
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

    TreeNode* syntax_tree=Expr(pci, &parse_info);

    return syntax_tree;
}



////////////////////////////////////////////////////////////////////////////////////
// Print Tree //////////////////////////////////////////////////////////////////////



void PrintTree(TreeNode* node, int level=0)
{
    if(node->node_kind==ID_NODE)
        printf("%c\n", node->id);
    else if(node->oper==DOT)
        printf("product\n");
    else
        printf("inverse\n");

    if(node->child[0])
    {
        for(int i=0; i<level; i++) printf("   ");
        printf("|--");
        PrintTree(node->child[0], level+1);
    }

    if(node->child[1])
    {
        for(int i=0; i<level; i++) printf("   ");
        printf("|--");
        PrintTree(node->child[1], level+1);
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Destroy Tree ////////////////////////////////////////////////////////////////////

void DestroyTree(TreeNode* node)
{
    for(int i=0; i<MAX_CHILDREN; i++)
    {
        if(node->child[i]) DestroyTree(node->child[i]);
    }
    delete node;
}

////////////////////////////////////////////////////////////////////////////////////
// Test Cases //////////////////////////////////////////////////////////////////////

// 20 test cases from pages 459-460 of James Hein Book
// Each test case is written to input.txt and parsed

void RunTestCase(int case_num, const char* expr)
{
    printf("Test %d: %s\n", case_num, expr);

    FILE* f=fopen("input.txt", "w");
    fprintf(f, "%s", expr);
    fclose(f);

    CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

    TreeNode* syntax_tree=0;

    try
    {
        syntax_tree = Parse(&compiler_info);

        PrintTree(syntax_tree);

        DestroyTree(syntax_tree);
    }
    catch(...)
    {
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
        const char* test_cases[]=
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
        int num_cases=sizeof(test_cases)/sizeof(test_cases[0]);

        for(int i=0;i<num_cases;i++) RunTestCase(i+1, test_cases[i]);
    }

    else if (choice == 2)
    {
        FILE* f = fopen("input.txt", "r");

        if(!f)
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
            PrintTree(syntax_tree);
            DestroyTree(syntax_tree);
        }
        catch(...)
        {
            printf("Parse error\n");
        }
    }


    return 0;
}

////////////////////////////////////////////////////////////////////////////////////