/********************************************************
	TODO :

	Say we have a function like this :

int func( MyLinkedListNode *node, Param *param )
{
	int r = 0;

	while( node )
	{
		r += crunch_something( node, param );
		node = node->next_ptr;
	}
	return r;
}

	Let's suppose the list will be constant for a while and we want to call func() with it and
	lots of values of "param". So we want to specialize func() on the list contents. 
	
	Say the list contains 3 elements whose addresses are:

node = &elem1
node->next_ptr = &elem2
node->next_ptr->next_ptr = &elem3
node->next_ptr->next_ptr->next_ptr = NULL

	Maybe the entire MyLinkedListNode structs are constants but perhaps only some of the 
	fields are constant and others can be modified by crunch_something(). This iswhy
	the first parameter of func() is not a const * but just a pointer.
	
	In any case we use the constant_addresses_set to store the addresses of the constant 
	data, then specialize func() with a constant parameter which is the address of the first node.

	Here, we'd like to unroll the loop, not to get a faster code but to be able to eliminate 
	as much code as possible. We vould do this :
	
	- specialize the function :

int func( Param *param )
{
	MyLinkedListNode *node = &elem1;	// specialized constant
	int r = 0;
	
	while( node )
	{
		r += crunch_something( node, param );
		node = node->next_ptr;
	}
	return r;
}

	- unroll only the first loop iteration which gives something like this :
	
int func( Param *param )
{
	MyLinkedListNode *node = &elem1;	// specialized constant
	int r = 0;
	
	if( node )	// first iteration of while becomes if
	{
		r += crunch_something( node, param );
		node = node->next_ptr;
		while( node )
		{
			r += crunch_something( node, param );
			node = node->next_ptr;
		}
	}
	return r;
}

	- propagate constants (including loads from constant memory locations which are 
	inside the list nodes) and optimize :

- node = &elem1 is not NULL => if() goes away
- elem1.next_ptr is a known constant, equal to &elem2

int func( Param *param )
{
	// first iteration becomes :
	int r = crunch_something( &elem1, param );
	MyLinkedListNode *node = &elem2;
	
	// remaining iterations
	while( node )
	{
		r += crunch_something( node, param );
		node = node->next_ptr;
	}
	return r;
}

	- start over and unroll all iterations of the loop until we get the final code :

int func( Param *param )
{
	r = crunch_something( &elem1, param )
	r += crunch_something( &elem2, param );
	r += crunch_something( &elem3, param );
	return r;
}


	Then we'd be able to inline crunch_something (or just make 3 specialized versions) and, 
	using the constant first parameters, eliminate lots of code inside.
	
	Currently though, this class just implements a super stupid unroller that unrolls all loops that
	the LLVM unroller can unroll, so it doesn't seem to work on loops with exit conditions either.
	
 ********************************************************/

class LoopUnroll2 : public LoopPass {
public:
	static char ID; // Pass ID, replacement for typeid
	LoopUnroll2() : LoopPass(&ID) {}

	bool runOnLoop(Loop *L, LPPassManager &LPM);

	/// This transformation requires natural loop information & requires that
	/// loop preheaders be inserted into the CFG...
	///
	virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		AU.addRequiredID(LoopSimplifyID);
		AU.addRequiredID(LCSSAID);
		AU.addRequired<LoopInfo>();
		AU.addPreservedID(LCSSAID);
		AU.addPreserved<LoopInfo>();
		// FIXME: Loop unroll requires LCSSA. And LCSSA requires dom info.
		// If loop unroll does not preserve dom info then LCSSA pass on next
		// loop will receive invalid dom info.
		// For now, recreate dom info, if loop is unrolled.
		AU.addPreserved<DominatorTree>();
		AU.addPreserved<DominanceFrontier>();
	}
};

char LoopUnroll2::ID = 0;
static RegisterPass<LoopUnroll2> X("loop-unroll-2", "Unroll loops (hack)");

Pass *createLoopUnroll2Pass() { return new LoopUnroll2(); }

bool LoopUnroll2::runOnLoop(Loop *L, LPPassManager &LPM) {
  assert(L->isLCSSAForm());
  LoopInfo *LI = &getAnalysis<LoopInfo>();

  Function *F = L->getHeader()->getParent();

  unsigned TripCount = L->getSmallConstantTripCount();
  printf( "Trying to unroll loop in function %s count %d\n", F->getName().data(), TripCount );
	
  if (TripCount == 0)
  {
    printf( "TripCount is not known.\n" );
    return false;
  }

  // Unroll the loop.
  if (!UnrollLoop(L, TripCount, LI, &LPM))
  {
    printf( "UnrollLoop failed.\n" );
    return false;
  }
  
  printf( "UnrollLoop succeeded.\n" );

  // FIXME: Reconstruct dom info, because it is not preserved properly.
  DominatorTree *DT = getAnalysisIfAvailable<DominatorTree>();
  if (DT) {
    DT->runOnFunction(*F);
    DominanceFrontier *DF = getAnalysisIfAvailable<DominanceFrontier>();
    if (DF)
      DF->runOnFunction(*F);
  }
  return true;
}
