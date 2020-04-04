// <ioStmt> | <simpleStmt> | <declareStmt> | <conditionalStmt> | <iterativeStmt>

bool checkIDinScope(/*the 'lexeme' of identifier*/){
	/*check the presence of ID 'lexeme' from current scope (*st)
	all the way up to functionTable*/
}

void Build_ST_Semantic_Analyser(/*root of AST*/){
	/*NOTE: All errors, as they occur must be pushed in the error stack [change its definition as discussed to handle semantic error and relevant changes in the foundNewError(..) function]*/
	/*traverses the AST in top-to-down, left-to-right order,
	and execute the respective case statement for the current AST
	nodes occuring during traversal*/

	switch(gSymbol gs /*this is one of the enum values of the nodes that are a part of AST*/){
	/*one case for each possible 'gs'
	some will not have anything to do but need to call this function recursively so that we can
	backtrack as needed*/

	/* some desired implementations for <statements> are mentioned below*/

	case 'assignmentStmt':
		checkIDinScope(/*LHS ID [even if it is an array (only 'lexeme' is needed)]*/);
		/*update 'isAssigned' if LHS ID is the output parameter of the function.*/
		/*bound checking of LHS ID if it is an array. [May be dynamic]*/
		/*RHS (lvalueIDstmt or lvalueArrStmt) will involve an expression, check for checkIDinScope(RHS IDs) recursively*/
		Build_ST_Semantic_Analyser(/*child or sibling*/);
		break;

	case 'moduleReuseStmt':
		checkIDinScope(/*for all involved variables recursively*/);
		/*called function must be declared or defined not both*/
		/*when complete symbol table has been built, check if any declaration was redundant using 'funcStatus' (change its definition if required)*/
		/*input list and output list of the called function must match (w.r.t. type and sequence) with the variables in the passed parameter list and LHS list [generated from <optional>] respectively.*/
		/*check that it is not a recursive call*/
		Build_ST_Semantic_Analyser(/*child or sibling*/);
		break;

	case 'declareStmt':
		/*fill symbol table*/
		Build_ST_Semantic_Analyser(/*child or sibling*/);
		break;

	case 'conditionalStmt':
		checkIDinScope(/*swich variable*/);
		if(/*switch variable is INTEGER*/){
			/*<default> must be present*/
		}
		else if(/*switch variable is BOOLEAN*/){
		/*case must be there for both TRUE and FALSE,
		<default> must not be there.*/
		}
		else{
			/*any other data type for switch variabe means error*/
		}
		checkIDinScope(/*recursively for all the variables involved in statements inside the case(s)*/);
		Build_ST_Semantic_Analyser(/*child or sibling*/);
		break;

	case 'iterativeStmt':
		switch(/*type of iterative statement*/){
			case 'FOR':
				checkIDinScope(/*loop variable*/);
				type(/*loop variable*/) == 'INTEGER';
				checkIDinScope(/*recursively check for variables in nested scopes*/);

			case 'WHILE':
				checkIDinScope(/*variables involved in the loop condition expression*/);
				type(/*loop condition*/) == 'BOOLEAN';
				checkIDinScope(/*recursively check for variables in nested scopes*/);
		}
		Build_ST_Semantic_Analyser(/*child or sibling*/);
		break;
	}
}


/*Additional semantic checks:
	1. when the scope of the function ends, check if all its output parameters have been assigned.
	2. declaration checks:
		check that two functions can't have same name
		check that two variables can't have same name
	.
	.
	.
	.
	*/
