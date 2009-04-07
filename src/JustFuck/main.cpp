#include <AsmJit/AsmJitAssembler.h>
#include <AsmJit/AsmJitCompiler.h>

#include <cstdio>
#include <vector>
#include <stack>
#include <ctime>
#include <iostream>

using namespace AsmJit;
using namespace std;

static const int MAX_SLOT = 30000;

void Nothing()
{
    return;
}

struct LabelPair
{
    Label* begin;
    Label* end;
};

class Asm8
{
public:
    Asm8();

    void Go();

    void Inc();
    void Dec();

    void Add(unsigned char ch);
    void Move(int i);

    void Left();
    void Right();

    void BeginLoop();
    void EndLoop();

    void Put();

private:
    char buf[MAX_SLOT];

    Compiler c;
    Function* f;

    std::vector<LabelPair*> all_labels;
    std::stack<LabelPair*> label_stack;
};

Asm8::Asm8()
{
    memset(buf, 0, 30000);

    f = c.newFunction(CALL_CONV_DEFAULT, BuildFunction0());
    f->setNaked(true);

    // init pointer position

    c.pushad();

    c.pushad();
    c.call(Nothing);
    c.popad();

    // ebx = data pointer
    // al = current value
    c.mov(ebx, (UInt32)buf);
    c.mov(eax, Immediate(0) );
}

void Asm8::Go()
{
    c.popad();
    c.endFunction();

    //// Make JIT function
    typedef void (*FuncT)();
    FuncT fn = function_cast<FuncT>(c.make());

    //// Ensure that everything is ok
    if (!fn)
    {
        printf("Error making jit function (%u).\n", c.error());
        return;
    }

    //// Call JIT function
    fn();

    //// If you don't need the function anymore, it should be freed
    MemoryManager::global()->free((void*)fn);
}

void Asm8::Inc()
{
    c.inc(al);
}

void Asm8::Dec()
{
    c.dec(al);
}

void Asm8::Left()
{
    c.mov( ptr(ebx), al );
    c.dec( ebx );
    c.mov( al, ptr(ebx) );
}

void Asm8::Right()
{
    c.mov( ptr(ebx), al );
    c.inc( ebx );
    c.mov( al, ptr(ebx) );
}

void Asm8::BeginLoop()
{
    LabelPair* pPair = new LabelPair();
    pPair->begin = c.newLabel();
    pPair->end = c.newLabel();

    all_labels.push_back(pPair);
    label_stack.push(pPair);

    c.test(al, al);
    c.jz( pPair->end );

    c.bind( pPair->begin ) ;
}

void Asm8::EndLoop()
{
    LabelPair *pPair = label_stack.top();

    c.test(al, al);
    c.jnz( pPair->begin );

    c.bind( pPair->end );
    label_stack.pop();
}

void Asm8::Put()
{
    c.pushad();

    c.push( eax );
    c.call(putchar);
    c.add( esp, 4 ); // pop parameter from stack

    c.popad();
}

void Asm8::Add( unsigned char ch )
{
    if (ch==1)
        Inc();
    else if (ch==-1)
        Dec();
    else
        c.add(al, Immediate(ch) );
}

void Asm8::Move( int i )
{  
    c.mov( ptr(ebx), al );
    c.add(ebx, Immediate(i) );
    c.mov( al, ptr(ebx) );
    return;

    if (i==1)
        Right();
    else if (i==-1)
        Left();
    else
    {
        c.mov( ptr(ebx), al );
        c.add(ebx, Immediate(i) );
        c.mov( al, ptr(ebx) );
    }
}

void test()
{
    Asm8 a;
    for (int i=0; i<65; ++i)
        a.Inc();
    a.Put();

    a.Go();
}

class Parser
{
public:
    enum EState
    {
        STATE_NORMAL, 
        STATE_VALUE, 
        STATE_MOVE, 
    };

    Parser()
    {
        state = STATE_NORMAL;
    }

    void DoParse()
    {
        while(!cin.eof())
        {
            char c;
            cin.get(c);

            switch(c)
            {
            case '+':
                _SwitchState(STATE_VALUE);
                ++cValue;
                break;
            case '-':
                _SwitchState(STATE_VALUE);
                --cValue;
                break;
            case '>':
                _SwitchState(STATE_MOVE);
                ++iMove;
                break;
            case '<':
                _SwitchState(STATE_MOVE);
                --iMove;
                break;
            case '[' :
                _SwitchState(STATE_NORMAL);
                jit.BeginLoop(); 
                break;
            case ']':
                _SwitchState(STATE_NORMAL);
                jit.EndLoop();
                break;
            case '.':
                _SwitchState(STATE_NORMAL);
                jit.Put();
                break;
            }
        }
        _SwitchState(STATE_NORMAL);
        cin.clear();
        jit.Go();
    }

private:
    void _SwitchState(EState newState)
    {
        if (state==newState)
            return;

        switch (state)
        {
        case STATE_VALUE:
            jit.Add(cValue);
            break;
        case STATE_MOVE:
            jit.Move(iMove);
            break;

        }
        state = newState;
        switch (state)
        {
        case STATE_VALUE:
            cValue = 0;
            break;
        case STATE_MOVE:
            iMove = 0;
            break;
        }
    }

    EState state;
    Asm8 jit;

    int iMove;
    char cValue;
};

void main()
{
    clock_t start_time, end_time;
    start_time = clock();

    Parser p;
    p.DoParse();

    end_time = clock();
    printf("\nTime : %f", (static_cast<float>(end_time-start_time))/CLOCKS_PER_SEC);
    return;
}
