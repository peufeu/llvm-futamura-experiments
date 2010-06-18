
Function * futamurize( const Function * orig_func, DenseMap<const Value*, Value*> &argmap, std::set<const unsigned char *> &constant_addresses_set )
{
	LLVMContext &context = getGlobalContext();
	
	
	// Make a copy of the function, removing constant arguments
	Function * specialized_func = CloneFunction( orig_func, argmap );
	specialized_func->setName( orig_func->getNameStr() + "_1" );
	
	// add it to our module
	LLVM_Module->getFunctionList().push_back( specialized_func );
	
	printf("\nspecialized_func = %p <%s>\n", specialized_func, specialized_func->getName().data());
	//~ specialized_func->dump();

	// Optimize it
	FunctionPassManager PM( LLVM_Module );
	createStandardFunctionPasses( &PM, 3 );
	
	PM.add(createScalarReplAggregatesPass());  // Break up aggregate allocas
	PM.add(createInstructionCombiningPass());  // Cleanup for scalarrepl.
	PM.add(createJumpThreadingPass());         // Thread jumps.
	PM.add(createCFGSimplificationPass());     // Merge & remove BBs
	PM.add(createInstructionCombiningPass());  // Combine silly seq's
	PM.add(createTailCallEliminationPass());   // Eliminate tail calls
	PM.add(createCFGSimplificationPass());     // Merge & remove BBs
	PM.add(createReassociatePass());           // Reassociate expressions
	PM.add(createLoopRotatePass());            // Rotate Loop
	PM.add(createLICMPass());                  // Hoist loop invariants
	PM.add(createLoopUnswitchPass( false ));
	PM.add(createInstructionCombiningPass());
	PM.add(createIndVarSimplifyPass());        // Canonicalize indvars
	PM.add(createLoopDeletionPass());          // Delete dead loops
	PM.add(createLoopUnroll2Pass());            // Unroll small loops
	PM.add(createInstructionCombiningPass());  // Clean up after the unroller
	PM.add(createGVNPass());                   // Remove redundancies
	PM.add(createMemCpyOptPass());             // Remove memcpy / form memset
	PM.add(createSCCPPass());                  // Constant prop with SCCP
	PM.add(createPromoteMemoryToRegisterPass()); 
	PM.add(createConstantPropagationPass());            
	PM.add(createDeadStoreEliminationPass());            
	PM.add(createAggressiveDCEPass());            
	PM.add(new MemoryDependenceAnalysis());            
	//~ PM.add(createAAEvalPass());              
	
	const PassInfo * pinfo = Pass::lookupPassInfo( "print-alias-sets" );
	if( !pinfo ) { printf( "print-alias-sets not found\n" ); exit(-1); }
	PM.add( pinfo->createPass() );
	
	FunctionPassManager PM_Inline( LLVM_Module );
	PM_Inline.add(createSingleFunctionInliningPass());            
	
	bool Changed = false;
	int iterations = 2;
	int inline_iterations = 6;
	
	do
	{
		Changed = false;
		
		// first do some optimizations
		PM.doInitialization();
		PM.run( *specialized_func );
		PM.doFinalization();
		
		// Load from Constant Memory detection
		const TargetData *TD = LLVM_EE->getTargetData();
		
		for (inst_iterator I = inst_begin(specialized_func), E = inst_end(specialized_func); I != E; ++I) 
		{
			Instruction * inst = (Instruction *) &*I;

			// get all Load instructions
			LoadInst * load = dyn_cast<LoadInst>( inst );
			if( !load ) continue;
			if( load->isVolatile() ) continue;

			if (load->use_empty()) continue;        // Don't muck with dead instructions...

			// get the address loaded by load instruction
			Value *ptr_value = load->getPointerOperand();
			
			// we're only interested in constant addresses
			ConstantExpr * ptr_constant_expr =  dyn_cast<ConstantExpr>( ptr_value );
			if( !ptr_constant_expr ) continue;			
			ptr_constant_expr->dump();
			
			// compute real address of constant pointer expression
			Constant * ptr_constant = ConstantFoldConstantExpression( ptr_constant_expr, TD );
			if( !ptr_constant ) continue;
			ptr_constant->dump();
			
			// convert to int constant
			ConstantInt *int_constant =  dyn_cast<ConstantInt>( ConstantExpr::getPtrToInt( ptr_constant, Type::getInt64Ty( context )));
			if( !int_constant ) continue;
			int_constant->dump();
			
			// get data size
			int data_length = TD->getTypeAllocSize( load->getType() );
			ptr_value->getType()->dump();
			
			// get real address (at last !)
			const unsigned char * c_ptr = (const unsigned char *) int_constant->getLimitedValue();
			
			printf( "%ld %d %d\n", c_ptr, constant_addresses_set.count( c_ptr ), data_length );
			
			// check what's in this address	
			int isconst = 1;
			for( int offset=0; offset<data_length; offset++ )
				isconst &= constant_addresses_set.count( c_ptr + offset );
			
			if( !isconst ) continue;
			printf( "It is constant.\n" );
			
			// make a LLVM const with the data
			Constant *new_constant = NULL;
			switch( data_length )
			{
				case 1:	new_constant = ConstantInt::get( Type::getInt8Ty( context ),  *(uint8_t*)c_ptr, false /* signed */ );	break;
				case 2:	new_constant = ConstantInt::get( Type::getInt16Ty( context ), *(uint16_t*)c_ptr, false /* signed */ );	break;
				case 4:	new_constant = ConstantInt::get( Type::getInt32Ty( context ), *(uint32_t*)c_ptr, false /* signed */ );	break;
				case 8:	new_constant = ConstantInt::get( Type::getInt64Ty( context ), *(uint64_t*)c_ptr, false /* signed */ );	break;
				default:
				{
					StringRef const_data ( (const char *) c_ptr, data_length );
					new_constant = ConstantArray::get( context, const_data, false /* dont add terminating null */ );
				}
			}
			
			if( !new_constant ) continue;
			
			new_constant->dump();
							
			//~ // get the type that is loaded
			const Type *Ty = load->getType();
			
			// do we need a cast ?
			if( load->getType() != new_constant->getType() )
			{
				new_constant = ConstantExpr::getBitCast( new_constant, Ty );
				new_constant->dump();
			}
			
			// zap the load and replace with constant address
			load->replaceAllUsesWith( new_constant );
			printf( "\nREPLACED :...\n" );
			load->dump();
			new_constant->dump();
			
			Changed = true;
		}	
		
		if( Changed )
			continue;	// re-optimize and do another pass of constant load elimination
		
		// if we can't do anything else, do an inlining pass
		if( inline_iterations > 0 )
		{
			inline_iterations --;
			
			PM_Inline.doInitialization();
			Changed |= PM_Inline.run( *specialized_func );
			PM_Inline.doFinalization();

			//~ for( int i=0; i<3; i++ )
			{
				PM.doInitialization();
				Changed |= PM.run( *specialized_func );
				PM.doFinalization();
			}
		}
		
		if( iterations>0 && !Changed ) 
			iterations--;
	} while( Changed || iterations>0 );
	
	return specialized_func;
}