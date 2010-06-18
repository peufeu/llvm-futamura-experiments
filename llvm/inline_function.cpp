
/********************************************************
	A simple function inliner which always inlines
	functions whose name contains "rpn" and have
	at least 1 constant parameter.
 ********************************************************/

class SingleFunctionInliner : public FunctionPass {
public:
    static char ID;
    SingleFunctionInliner() : FunctionPass(&ID) {}


    virtual bool runOnFunction(Function &f)
    {
		printf( "SingleFunctionInliner : examining %s\n",  f.getName().data() );
		
        bool changed = false;
        // Scan through and identify all call sites ahead of time so
        // that we only inline call sites in the original functions,
        // not call sites that result from inlining other functions.
        std::vector<CallInst*> call_sites;
        for (Function::iterator bb = f.begin(), e = f.end(); bb != e; ++bb) 
		{
            for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); ++inst) 
			{
                assert(!isa<InvokeInst>(inst) && "We don't expect any invoke instructions in Python.");
				
                CallInst *call = dyn_cast<CallInst>(inst);
                if (call == NULL)
                    continue;
				
                // This may miss inlining indirect calls that become
                // direct after inlining something else.
                Function *called_function = call->getCalledFunction();
                if (called_function == NULL)
                    continue;
						
		// filter on names to avoid inlining printf etc
		if( called_function->getName().find( "rpn" ) == StringRef::npos )
			continue;
		
		printf( "SingleFunctionInliner : found call site %p for %s\n", call, called_function->getName().data() );
		
		// lifted from original code for this class (grabbed from PyPy)
                if (called_function->isMaterializable()) 
		{
                    if (called_function->Materialize())
                        continue;
                }
		
                if( called_function->isDeclaration() )
			continue;
	
		// only inline functions which have at least a constant argument 
		// which we may be able to use to futamurize the inlined function
		
		// first operand 0 of call is the function itself, args start at 1
		int n_call_args = call->getNumOperands() - 1;
		
		// if the function has no args, and it was worth inlining, the compiler
		// already did it... so we don't need to look at it.
		if( !n_call_args ) 
			continue;
		
		// look for constant arguments
		bool has_constants = false;
		for( int i=1; i<n_call_args; i++ )
		{
			printf( "Got call argument\n" );
			const Value* arg = call->getOperand( i );
			arg->dump();
			const ConstantExpr * const_arg = dyn_cast<const ConstantExpr>( arg );
			if( const_arg )
			{
				printf( "argument is constant\n" );
				has_constants = true;
				break;
			}
		}
		
		if( !has_constants )
			continue;
		
		printf( "will inline call : \n" );
		call->dump();
		
		call_sites.push_back(call);
            }
        }

        // Actually inline the calls we found.
        for (size_t i = 0; i != call_sites.size(); ++i) {
            changed |= InlineFunction(call_sites[i]);
        }
        return changed;
    }
};

// The address of this variable identifies the pass.  See
// http://llvm.org/docs/WritingAnLLVMPass.html#basiccode.
char SingleFunctionInliner::ID = 0;

FunctionPass *createSingleFunctionInliningPass()
{
    return new SingleFunctionInliner();
}