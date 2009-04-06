#ifndef _JustFuck_Assemblers_
#define _JustFuck_Assemblers_

#include "AsmJit/AsmJitAssembler.h"
#include "AsmJit/AsmJitCompiler.h"

#include <vector>
#include <stack>

namespace JustFuck
{
	class AssemblerT
	{
	public:
		void InitAsm();
	protected:
		struct LabelPair
		{
			AsmJit::Label* begin;
			AsmJit::Label* end;
		};

		AsmJit::Compiler c;
		AsmJit::Function* f;

		std::vector<LabelPair*> all_labels;
		std::stack<LabelPair*> label_stack;
	} ;
}

#endif