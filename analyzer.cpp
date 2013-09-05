#include "analyzer.hpp"
#include "compunit.hpp"
#include "parsetree.hpp"
#include "debug.hpp"
#include "symtab.hpp"
#include "errors.hpp"

using namespace std;

Analyzer::Analyzer() {
    symTab = new SymbolTable();
	symTab_G = symTab; // XXX
}

Analyzer::Analyzer(string name) {
	filename = name;
    symTab = new SymbolTable();
	symTab_G = symTab; // XXX
}
 
Analyzer::~Analyzer() {
    delete symTab;
}

void Analyzer::addUse(Symbol * symbol, unsigned int line, Extree * treenode) {
	SymbolUse use;
	use.symbol = symbol;
	use.line = line;
	use.treenode = treenode;	
}

// Visit <id_list> node and return vector containing id names
vector<string>* Analyzer::visitIdList(ParseTreeNode *id_list, vector<string>* idents) {
    // Loop through id_list recursively
    while(id_list->hasChild()) {
            string ident = id_list->getFirstChild()->getNextSibling()->getToken()->getValue();
            idents->push_back(ident);
            id_list = id_list->getLastChild();
    }
    return idents;
}

void Analyzer::visitUseClause(ParseTreeNode *use_clause) {
    ParseTreeNode *pkg_id = use_clause->getNextSibling();
    ParseTreeNode *pkg_list = pkg_id->getNextSibling();
     // Collect all the packages to use 
    vector<string> pkgs;
    pkgs.push_back(pkg_id->getToken()->getValue());
    
    visitIdList(pkg_list, &pkgs);
    if(debugOpt.inDebug("astDeclare")) {
        cout<<"Using packages: "<<endl;
        for(vector<string>::const_iterator iter = pkgs.begin();
                iter != pkgs.end(); iter++) {
            cout<<*iter<<endl;
            symTab->UsePackage(*iter);
        }
    }

}

void Analyzer::visitDeclaration(ParseTreeNode *declaration) {
    // Determine the type of declaration
    ParseTreeNode *curNode = declaration->getFirstChild();

    if(!curNode->hasToken()) {

        char *pszDeclarType = curNode->getName();
        if(debugOpt.inDebug("astDeclare")) {
            cout<<"NonTerminal: "<<pszDeclarType<<endl; 
        }

        if(strcmp(pszDeclarType, "<pragma>") == 0) {
            // Handle pragma statement
            ASSERT(false, "Not implemented");
        } else if(strcmp(pszDeclarType, "<object_declar>") == 0) {
            visitObjectDeclaration(curNode, false, NULL);
        } else if(strcmp(pszDeclarType, "<type_declaration>") == 0) {
            visitTypeDeclaration(curNode);
        } else if(strcmp(pszDeclarType, "<subtype_declaration>") == 0) {
            visitSubTypeDeclaration(curNode);
        } else if(strcmp(pszDeclarType, "<subprogram_declaration>") == 0) {
            visitSubProgramSpecification(curNode->getFirstChild());
        } else {
        }
    } // if nonterminal
    else {
        if(debugOpt.inDebug("astDeclare")) {
            cout<<"Terminal: "<<curNode->getToken()->getType()<<endl; 
        }

        if(curNode->getToken()->getType() == Token::USE) {
            visitUseClause(curNode);
        } else if(curNode->getToken()->getType() == Token::IDENT) {
            // Handle exception declaration?
            //   ASSERT(true, "Not implemented");
        } else {
            ASSERT(true, "Error analyzing <declaration> node");
        }
    } // if terminal

}

void Analyzer::visitTypeDeclaration(ParseTreeNode *type_declar) {
    ParseTreeNode *curNode = type_declar->getFirstChild();
    Symbol *type = new Symbol(Symbol::TYPE);

    // Determine what type of type declaration is here
    if (curNode->hasToken()) {
        // type id is <type_definition> ;
        ParseTreeNode *id = curNode->getNextSibling();
        ParseTreeNode *type_def = id->getNextSibling()->getNextSibling()->getFirstChild();
        type->setName(id->getToken()->getValue());
        
        TypeDescriptor *typ_desc = visitTypeDefinition(type_def);
        type->setTypeDescriptor(typ_desc);
    } else {
        // <incomplete_type_decl>
        ParseTreeNode *id = curNode->getFirstChild()->getNextSibling();
        type->setName(id->getToken()->getValue());
    }

    symTab->AddSymbol(type);
}

vector<RangeNode*>* Analyzer::visitDiscreteRangeList(ParseTreeNode *cons_list) {
    // Rule:  <constrained_index_list> ( <discrete_range> <discrete_range_list> )
    vector<RangeNode*>* ranges = new vector<RangeNode*>();
    ParseTreeNode* cur_range = cons_list->getFirstChild()->getNextSibling();
    ranges->push_back(visitDiscreteRange(cur_range));
    
    // Go through the discrete_range_list
    cur_range = cur_range->getNextSibling();
    while(cur_range->hasChild()) {
        // <discrete_range_list> , <discrete_range> <discrete_range_list>
        //                        (empty)
        cur_range = cur_range->getFirstChild()->getNextSibling();
        ranges->push_back(visitDiscreteRange(cur_range));
        cur_range = cur_range->getNextSibling();
    }
    return ranges;
}

vector<string>* Analyzer::visitUnconstrainedRangeList(ParseTreeNode *uncons_list) {
    // Rule:  <unconstrained_index_list> ( <index_subtype_def> <index_subtype_def_list> )
    // Rule:  <index_subtype_def> id range lt gt
    vector<string>* ranges = new vector<string>();
    ParseTreeNode* cur_range = uncons_list->getFirstChild()->getNextSibling();
    ranges->push_back(cur_range->getFirstChild()->getToken()->getValue());
    
    // Go through the rest of the index_subtype_def_list
    cur_range = cur_range->getNextSibling();
    while(cur_range->hasChild()) {
        cur_range = cur_range->getFirstChild()->getNextSibling();
        ranges->push_back(cur_range->getFirstChild()->getToken()->getValue());
        cur_range = cur_range->getNextSibling();
    }
    return ranges;
}

ArrayDescriptor* Analyzer::visitArrayDefinition(ParseTreeNode *array_node) {
    ArrayDescriptor *array = 0;

    array_node = array_node->getFirstChild();
    char *pszArrayType = array_node->getName();
    ParseTreeNode* index_list = array_node->getFirstChild()->getNextSibling();
    ParseTreeNode* element_t = index_list->getNextSibling()->getNextSibling();

    if(strcmp(pszArrayType, "<unconstrained_array_def>") == 0) {
        // <unconstrained_array_def> -> array <unconstrained_index_list> of <element_type>
        array = new ArrayDescriptor(ArrayDescriptor::Unconstrained);
        vector<string> * ranges = visitUnconstrainedRangeList(index_list);
        array->setRange(ranges);
       
    } else {
        // <constrained_array_def> -> array <constrained_index_list> of <element_type>
        array = new ArrayDescriptor(ArrayDescriptor::Constrained);
        vector<RangeNode*> * ranges = visitDiscreteRangeList(index_list);
        array->setRange(ranges);
    }
    Symbol* type_sym = visitTypeOrSubtype(element_t->getFirstChild()->getFirstChild());
    ASSERT(type_sym != 0, "Invalid array element type.");
    array->setElementType(type_sym);
    return array; 
}

// Visit an enumeration type definition and return EnumDescriptor for it
EnumDescriptor* Analyzer::visitEnumDefinition(ParseTreeNode *enum_node) {
    // Rule: <enumeration_type_def> ( id <id_list> )

    EnumDescriptor *enum_d = new EnumDescriptor();

    ParseTreeNode* first_id = enum_node->getFirstChild()->getNextSibling();
    ParseTreeNode* id_list = first_id->getNextSibling();

    // Collect all the new identifiers
    vector<string>* ids = new vector<string>();
    ids->push_back(first_id->getToken()->getValue());
    visitIdList(id_list, ids);
    enum_d->setValues(ids);

    return enum_d;
}
void Analyzer::visitCompDeclar(ParseTreeNode *comp_decl_node, vector<Symbol*>* comps) {
    // Rules:  <component_declar_list> <component_declar> <component_declar_list>
    //                            |
    while(comp_decl_node->hasChild()) {
        comp_decl_node = comp_decl_node->getFirstChild();
        visitObjectDeclaration(comp_decl_node, true, comps); 
        comp_decl_node = comp_decl_node->getNextSibling();
    }
}

void Analyzer::visitVariantPart(ParseTreeNode *variant_node, vector<Variant*>* vars) {

}

RecordDescriptor* Analyzer::visitRecordDefinition(ParseTreeNode *record_node) {
    // Rules:   <record_type_definition> record <components_list>  end record
    //
    //          <components_list>  <component_declar_list> <variant_part_optional>
    //                            | null ;
    //
        
    ParseTreeNode* comps_list = record_node->getFirstChild()->getNextSibling();
    
    RecordDescriptor *record = new RecordDescriptor();
    if(comps_list->hasToken()) {
        // empty record
    } else {
        // TODO: Make sure that this isn't empty!! Grammar doesn't enfore this.
        ParseTreeNode* comp_decl_list = comps_list->getFirstChild();
        ParseTreeNode* var_part = comp_decl_list->getNextSibling();
        
        // Get all components
        if(comp_decl_list->hasChild()) {
            vector<Symbol*>* comps = new vector<Symbol*>();
            visitCompDeclar(comp_decl_list, comps);
            record->setComponents(comps);
        }
        
        // Get variant part
        if(var_part->hasChild()) {
            vector<Variant*>* vars = new vector<Variant*>();
            visitVariantPart(var_part, vars);
            record->setVariants(vars);
        }
    }
    return record;
}

TypeDescriptor *Analyzer::visitTypeDefinition(ParseTreeNode *type_def) {
    if(type_def->hasToken()) {
        // access type
        ParseTreeNode *subtype = type_def->getNextSibling();

        Symbol* subtypeSym = 0;
        if(subtype->hasToken()) {
            subtypeSym = symTab->getSymbolByName(subtype->getToken()->getValue(), false);
            cout<<subtypeSym->getName()<<endl;
            ASSERT(subtypeSym != 0, "access subtype is not defined. ");
        } else {
            TypeDescriptor* type_desc = visitSubTypeDefinition(subtype->getFirstChild());
            subtypeSym = new Symbol(Symbol::TYPE);
            subtypeSym->setTypeDescriptor(type_desc);
            subtypeSym->setName(symTab->getNextAnonName("anon"));
        }
        AccessDescriptor *access = new AccessDescriptor(subtypeSym);
        return access;
    } else {
        char *pszTypeDef = type_def->getName();
        if (strcmp(pszTypeDef, "<record_type_definition>") == 0) {
            return visitRecordDefinition(type_def); 
        } else if (strcmp(pszTypeDef, "<array_type_definition>") == 0) {
            return visitArrayDefinition(type_def);
        } else if (strcmp(pszTypeDef, "<enumeration_type_def>") == 0) {
            return visitEnumDefinition(type_def);
        } else {
            char buf[255];
            sprintf(buf, "unexpected type definition %s", pszTypeDef);
            ASSERT(false, buf);
        }
    }
}

TypeDescriptor* Analyzer::visitSubTypeDefinition(ParseTreeNode *subtype_def) {
    // <subtype_definition> id <range_constraint> 
    //                      id <index_constraint>

    subtype_def = subtype_def->getFirstChild();
    Symbol* parentTypeSym = symTab->getSymbolByName(subtype_def->getToken()->getValue(), false);
    if(parentTypeSym == 0) {
        ASSERT(false, "Parent type of subtype not defined");
    }
    SubTypeDescriptor* subtype = new SubTypeDescriptor(parentTypeSym);

    subtype_def = subtype_def->getNextSibling();
    char *pszConstraint = subtype_def->getName();

    if (strcmp(pszConstraint, "<range_constraint>") == 0) {
        // store the ExpressionNodes that represent the lower and upper bounds of the range
        ParseTreeNode* rangeNode = subtype_def->getFirstChild()->getNextSibling();
        rangeNode = rangeNode->getFirstChild();
        ExpressionNode* start = visitSimpleExp(rangeNode);
        rangeNode = rangeNode->getNextSibling()->getNextSibling();
        ExpressionNode* stop = visitSimpleExp(rangeNode);
        subtype->setRange(start, stop);
    } else {
        // <index constraint>
        ParseTreeNode* index_cons = subtype_def->getFirstChild();
        vector<RangeNode*>* ranges = visitDiscreteRangeList(index_cons);
        subtype->setRange(ranges);
    }

    return subtype;
}

void Analyzer::visitSubTypeDeclaration(ParseTreeNode *subtype_declar) {
    // Rule: 
    // <subtype_declaration>    subtype id is <subtype_definition> ;
    //                          subtype id is id ;
    ParseTreeNode *id = subtype_declar->getFirstChild()->getNextSibling();
    ParseTreeNode *subtype_node = id->getNextSibling()->getNextSibling();
    Symbol *subtype = new Symbol(Symbol::TYPE);
    subtype->setName(id->getToken()->getValue());
    
    if(subtype_node->hasToken()) {
        // subtype with no constraints
        Symbol* baseTypeSym = symTab->getSymbolByName(subtype_node->getToken()->getValue(), false);
        ASSERT(baseTypeSym != 0, "base type not defined.");
        SubTypeDescriptor* type_desc = new SubTypeDescriptor(baseTypeSym);
        subtype->setTypeDescriptor(type_desc);
    } else {
        // subtype of some constraint
        TypeDescriptor* type_desc = visitSubTypeDefinition(subtype_node);
        subtype->setTypeDescriptor(type_desc);
    }
    symTab->AddSymbol(subtype);
}

void Analyzer::visitSubProgramSpecification(ParseTreeNode *subprog_spec) {

    ParseTreeNode *curNode = subprog_spec->getFirstChild();
    Symbol *subprog = new Symbol(Symbol::TYPE);
    TypeDescriptor* type_desc = 0;

    // TODO: Parse the rest of the declaration
    // Determine if subprogram is a procedure or a function
    Token::Type subprog_t = curNode->getToken()->getType();
    if(subprog_t == Token::PROCEDURE) {
        curNode = curNode->getNextSibling();
        subprog->setName(curNode->getToken()->getValue()); 

    } else if(subprog_t == Token::FUNCTION) {
        ParseTreeNode *designator = curNode->getNextSibling();
        // Check if operator overloading is being used
        if(designator->hasToken()) {
            // id
            subprog->setName(designator->getToken()->getValue());
        } else {
            // <operator_symbol> -> stringliteral
            subprog->setName(designator->getFirstChild()->getToken()->getValue());
        }

    } else {
        ASSERT(false, "Invalid subprogram specification.");
    }

    subprog->setTypeDescriptor(type_desc);
    symTab->AddSymbol(subprog);
}

void Analyzer::visitSubProgramBodyDeclaration(ParseTreeNode *body_declar) {
    ParseTreeNode *subprogram_decl = body_declar->getFirstChild();
    ParseTreeNode *body_declar_list = subprogram_decl->getNextSibling()->getNextSibling();

    visitSubProgramSpecification(subprogram_decl);
    visitBodyDeclareList(body_declar_list); 
}

// Visit an object declaration and add to symbol table at current scope
// Rules:   <object_declar> id <id_list> : <constant_optional> <type_or_subtype> <initialization_optional> ;
//          <component_declar> id <id_list> : <type_or_subtype> <initialization_optional> ;
// 
void Analyzer::visitObjectDeclaration(ParseTreeNode *object_declar, bool forRecord, vector<Symbol*>* syms) {
    ParseTreeNode *id = object_declar->getFirstChild();
    ParseTreeNode *id_list = id->getNextSibling();
    ParseTreeNode *constant_opt = 0;
    ParseTreeNode *type_or_subtype = 0;
    
    if(!forRecord) {
        constant_opt = id_list->getNextSibling()->getNextSibling();
        type_or_subtype = constant_opt->getNextSibling()->getFirstChild();
    } else {
        type_or_subtype = id_list->getNextSibling()->getNextSibling()->getFirstChild();
    }
    ParseTreeNode *init_op = type_or_subtype->getNextSibling();

    // Collect all the new identifiers
    vector<string> ids;
    ids.push_back(id->getToken()->getValue());

    visitIdList(id_list, &ids);

    // Is the identifier a constant or variable?
    Symbol::SymbolType id_symtype = Symbol::VAR;
    if(!forRecord && constant_opt->hasChild()) {
        id_symtype = Symbol::CONST;
    }

    Symbol* type_sym = visitTypeOrSubtype(type_or_subtype);

    if (!type_sym) {
        ASSERT(false, "Object type not defined.");
    }

    for(vector<string>::const_iterator iter = ids.begin();
            iter != ids.end(); iter++) {
        Symbol *sym = new Symbol(id_symtype);
        sym->setName(*iter);
        sym->setTypeDescriptorSym(type_sym);
        // don't add variable to symbol table for record definitions
        if(!forRecord) {
            symTab->AddSymbol(sym);
        } else {
            syms->push_back(sym);
        }
    }
}

// Note: type_or_subtype should point to the first child of a type_or_subtype node
Symbol* Analyzer::visitTypeOrSubtype(ParseTreeNode *type_or_subtype) {
    // What is the type of the identifiers
    Symbol* type_sym = 0; 
    char *pszIdType = type_or_subtype->getName();
    if ( strcmp(pszIdType, "<type>") == 0) {
        type_or_subtype = type_or_subtype->getFirstChild();
        if(type_or_subtype->hasToken()) {
            // Look up type from symbol table
            type_sym = symTab->getSymbolByName( type_or_subtype->getToken()->getValue(), false);
            if(!type_sym) {
                cerr<<type_or_subtype->getToken()->getValue()<<" is undefined.\n"; 
                ASSERT(false, "Undefined type");
            }
        } else {
            // Visit anonymous type definition
            TypeDescriptor *typ_desc = visitTypeDefinition(type_or_subtype->getFirstChild());
            type_sym = new Symbol();
            type_sym->setTypeDescriptor(typ_desc);
            type_sym->setName(symTab->getNextAnonName("anon"));
        }

    } else if (strcmp(pszIdType, "<subtype>") == 0) {
        type_or_subtype = type_or_subtype->getFirstChild();
        char *pszSubType = type_or_subtype->getName();
        if (strcmp(pszSubType, "<type_name>")) {
            type_or_subtype = type_or_subtype->getFirstChild();
            type_sym = symTab->getSymbolByName( type_or_subtype->getToken()->getValue(), false);
            if(!type_sym) {
                cerr<<type_or_subtype->getToken()->getValue()<<" is undefined.\n"; 
                ASSERT(false, "Undefined subtype");
            }
        } else {
            // Visit anonymous subtype definition
            TypeDescriptor *typ_desc = visitSubTypeDefinition(type_or_subtype->getFirstChild());
            type_sym = new Symbol();
            type_sym->setTypeDescriptor(typ_desc);
            type_sym->setName(symTab->getNextAnonName("anon"));

        }


        TypeDescriptor *typ_desc = visitTypeDefinition(type_or_subtype->getFirstChild());
    }

    return type_sym;
}

// Process declarations list by adding declarations to the symbol table
void Analyzer::visitSpecDeclareList(ParseTreeNode *spec_decl_list) {
    while(spec_decl_list && spec_decl_list->hasChild()) {
        ParseTreeNode* spec_declar = spec_decl_list->getFirstChild()->getFirstChild();
        char *pszDecl_type = spec_declar->getName(); 

        if (strcmp(pszDecl_type, "<declaration>") == 0) {
            visitDeclaration(spec_declar);
        } else if (strcmp(pszDecl_type, "<private_type_declar>") == 0) {
            // <private_type_declar> -> type id is private
            Symbol *privateSym = new Symbol();
            privateSym->setName(spec_declar->getFirstChild()->getNextSibling()->getToken()->getValue());
            symTab->AddSymbol(privateSym);
        } else {
            ASSERT(true, "Error analyzing <spec_decl_list> node");
        }

        spec_decl_list = spec_decl_list->getLastChild();
    }

}

// Visit the private_part of a package declarations
// These names should not be exposed to external users 
void Analyzer::visitPrivatePart(ParseTreeNode *private_part) {
    if ( private_part->hasChild() ) {
        // Visit the first private_item
        ParseTreeNode *private_item = private_part->getFirstChild()->getNextSibling();
        visitPrivateItem(private_item);

        // Visit any remaining private_items
        ParseTreeNode *private_item_list = private_item->getNextSibling();

        while ( private_item_list->hasChild() ) {
            visitPrivateItem(private_item_list->getFirstChild());
            private_item_list = private_item_list->getLastChild();
        }


    }
}

// Visit a single private_item and add entries into symbol table
// A private type can be either a type or a subtype
void Analyzer::visitPrivateItem(ParseTreeNode *private_item) {
    // Rules:
    // <private_item>   subtype id is <subtype_definition> ;
    //                  type id is <type_definition> ;
    //                  subtype id is id;
     
    ParseTreeNode *curNode = private_item->getFirstChild();
    Token::Type token_t = curNode->getToken()->getType();
    Symbol *privateSym = new Symbol();

    // All rules have id as second child
    curNode = curNode->getNextSibling();
    privateSym->setName(curNode->getToken()->getValue());

    TypeDescriptor* type_desc = 0;
    curNode = curNode->getNextSibling()->getNextSibling();
    if ( token_t == Token::TYPE ) {
        type_desc = visitTypeDefinition(curNode);
    } else if ( token_t == Token::SUBTYPE ) {
        if(curNode->hasToken()) {
        } else {
            type_desc = visitSubTypeDefinition(curNode);
        }
    } else {
        ASSERT (false, "Invalid private item");
    }
    if(type_desc) {
        type_desc->setPrivate(true);
        privateSym->setTypeDescriptor(type_desc);
    }
    symTab->AddSymbol(privateSym);

}

// Visit a package declaration and update the symbol table
void Analyzer::visitPackageDeclaration(ParseTreeNode *node) {
    //Rule: <package_spec_or_body> -> id is <spec_declar_list> <private_part> end <id_optional>

    ParseTreeNode * id_node = node->getFirstChild();
    ParseTreeNode * spec_declar_list = id_node->getNextSibling()->getNextSibling();
    ParseTreeNode * private_part = spec_declar_list->getNextSibling();
    ParseTreeNode * id_optional = private_part->getNextSibling()->getNextSibling(); 

    string pkg_id = id_node->getToken()->getValue();
    // TODO: Make sure optional id matches pkg_id

    symTab->OpenScope();
    visitSpecDeclareList(spec_declar_list);

    visitPrivatePart(private_part);
}

// Visit a body declaration list, this requires a new scope.
void Analyzer::visitBodyDeclareList(ParseTreeNode *body_decl_list) {
    // Open subprogram scope
    symTab->OpenScope();
    while (body_decl_list && body_decl_list->hasChild()) {
        ParseTreeNode* body_declar = body_decl_list->getFirstChild()->getFirstChild();
        char *bd_decl_name = body_declar->getName(); 

        if (!strcmp(bd_decl_name, "<declaration>")) {
            visitDeclaration(body_declar);
        } else if (!strcmp(bd_decl_name, "<subprogram_body_decl>")) {
            visitSubProgramBodyDeclaration(body_declar);
        } else {
            ASSERT(true, "Error analyzing <body_decl_list> node");
        }

        body_decl_list = body_decl_list->getLastChild();
    }
    // Close subprogram scope
    symTab->CloseScope();
}

void Analyzer::visitStmtList(ParseTreeNode * node,  vector<StatementNode*> * stmt_list) {

    // Handles.
    // <statement_list> <statement> <statement_list>
    //                |

    if (!node->hasChild()) {
        return;
    } else {
        ParseTreeNode * stmt = node->getFirstChild()->getFirstChild(); 
        char * stmt_type = stmt->getName();

        if(strcmp(stmt_type, "<pragma>") == 0) {
            // Not very intesting right now.

        } else if(strcmp(stmt_type, "<assign_statement>") == 0) {
            stmt_list->push_back(visitAssignStmt(stmt));

        } else if(strcmp(stmt_type, "<null_statement>") == 0) {
            stmt_list->push_back(visitNullStmt());

        } else if(strcmp(stmt_type, "<call_statement>") == 0) {
            stmt_list->push_back(visitCallStmt(stmt));

        } else if(strcmp(stmt_type, "<block>") == 0) {
            stmt_list->push_back(visitBlockStmt(stmt));

        } else if(strcmp(stmt_type, "<loop_statement>") == 0) {
            stmt_list->push_back(visitLoopStmt(stmt));

        } else if(strcmp(stmt_type, "<if_statement>") == 0) {

        } else if(strcmp(stmt_type, "<exit_statement>") == 0) {
            // alan:FIXME;
            ExitStmtNode * rtstmt = new ExitStmtNode();
            stmt_list->push_back(rtstmt);

		} else if(strcmp(stmt_type, "<if_statement>") == 0) {
			stmt_list->push_back(visitIfStmt(stmt));

        } else if(strcmp(stmt_type, "<return_statement>") == 0) {
            ReturnStmtNode * rtstmt = new ReturnStmtNode();
            rtstmt->exp = visitExpression(stmt->getFirstChild()->getNextSibling());
            stmt_list->push_back(rtstmt);

        } else if(strcmp(stmt_type, "<raise_statement>") == 0) {

		} else if(strcmp(stmt_type, "<case_statement>") == 0) {
			stmt_list->push_back(visitCaseStmt(stmt));
		}

        visitStmtList(node->getLastChild(), stmt_list);
    }

    //TODO:FIXME
    // MUST DO TYPE CHECKING HERE.
}

StatementNode * Analyzer::visitNullStmt() {
    return new NullStmtNode();
}

void Analyzer::visitSuffixList(ParseTreeNode *node, vector<NamePacker *> &npack_list) {
    //  <name_suffix_list>  <name_suffix_list> <name_suffix>
    //              |
    //  <name_suffix> ( <expression> <expression_list> )
    //              | ' id
    //              | . id
    //              | . <operator_symbol>

    if (!node->hasChild()) 
        return;

    ParseTreeNode * suffix_list = node->getFirstChild();
    ParseTreeNode * name_suffix = suffix_list->getNextSibling();

    visitSuffixList(suffix_list, npack_list);

    NamePacker *npack = new NamePacker;
    npack->exprs = NULL;
    // handle name_siffix
    ParseTreeNode * dlim = name_suffix->getFirstChild();
    int tk_type = dlim->getToken()->getType();
    // check for the types
    if (tk_type == Token::DOT) {
        // DOT 
        ParseTreeNode * node = dlim->getNextSibling();

        if (node->hasToken())
            npack->id = node->getToken()->getValue();
        else 
            // operator
            npack->id = node->getFirstChild()->getToken()->getValue();
        npack->atype = NamePacker::DOT;


    } else if (tk_type == Token::SINGLE_QUOTE) {
        // TICK
        ParseTreeNode * id = dlim->getNextSibling();
        npack->atype = NamePacker::TICK;
        npack->id = id->getToken()->getValue();
    } else if (tk_type == Token::LEFT_BRACE) {
        // expression
        ParseTreeNode * expr = dlim->getNextSibling();
        ParseTreeNode * expr_list = expr->getNextSibling();
        // get exp and evaluate the type and store to vector,
        npack->exprs = new vector<set<Symbol *> *>;
        npack->exprs->push_back(
                visitExpression(expr)->evalPossibleTypes());
        if (expr_list->hasChild()) 
            visitExpressionList(expr_list, npack->exprs) ;
        npack->atype = NamePacker::EXPR;
        npack->id = "N/A";
    }

    npack_list.push_back(npack);

}

NameNode * Analyzer::visitName(ParseTreeNode *node) {
    // <name> id <name_suffix_list> <all_optional> 
    ParseTreeNode * id_name = node->getFirstChild();
    ParseTreeNode * suffix_list = id_name->getNextSibling();
    ParseTreeNode * all_opt = suffix_list->getNextSibling();
    NameNode * namenode = new NameNode(id_name->getToken()->getValue()); 
    NamePacker *npack = new NamePacker;

    namenode->id = id_name->getToken()->getValue();	
    npack->atype = NamePacker::HEAD;
    npack->id = namenode->id;
    npack->exprs = NULL; 
    namenode->namelist.push_back(npack);

    if (suffix_list->hasChild())
        visitSuffixList(suffix_list, namenode->namelist);

    if (all_opt->hasChild()) 
        namenode->all_access = true;

    // type check using vector.
    // must determine whether it's an array, proc, func.....
    // assign the resolved symbol : Symbol *x = resolve(vector*);
    return namenode;
}

StatementNode * Analyzer::visitCallStmt(ParseTreeNode * node) {
	ParseTreeNode * name = node->getFirstChild();
	CallStmtNode * callstmnt = new CallStmtNode();
	callstmnt->name = visitName(name);
	return callstmnt;
}

/**
 * Assignment.
 */
StatementNode * Analyzer::visitAssignStmt(ParseTreeNode * node) {
	ParseTreeNode * name = node->getFirstChild();
	ParseTreeNode * expression = name->getNextSibling()->getNextSibling();


	AssignStmtNode * assignment = new AssignStmtNode();
	assignment->exp = visitExpression(expression);

	// Type checking: TODO:FIXME
	set<Symbol*> *posTypes = assignment->exp->evalPossibleTypes();
	
	return assignment;
}

StatementNode * Analyzer::visitIfStmt(ParseTreeNode * if_stmt) {

	ParseTreeNode * condition = if_stmt->getFirstChild()->getNextSibling();
	IfStmtNode * if_stmt_node = new IfStmtNode();

	// First one must be if.
	if_stmt_node->type = IfStmtNode::IF;
	if_stmt_node->cond = visitExpression(condition);


	// Take care of the statements.
    symTab->OpenScope();
	ParseTreeNode * stmts = condition->getNextSibling()->getNextSibling();
	visitStmtList(stmts, &(if_stmt_node->stmt_list));
    symTab->CloseScope();

	if_stmt_node->elsifStmt = 
		  visitElsifStmtList(stmts->getNextSibling());
	
	if_stmt_node->elseStmt = 
		  visitElseStmt(stmts->getNextSibling()->getNextSibling());

	return if_stmt_node;
}


IfStmtNode * Analyzer::visitElsifStmtList(ParseTreeNode * elsif_stmt_list) {


	if (elsif_stmt_list->hasChild()) {
	
		ParseTreeNode * condition = 
		 elsif_stmt_list->getFirstChild()->getFirstChild()->getNextSibling();

		IfStmtNode * elsif_stmt_node = new IfStmtNode();

		// First one must be elsif.
		elsif_stmt_node->type = IfStmtNode::ELSIF;
		elsif_stmt_node->cond = visitExpression(condition);

    	symTab->OpenScope();
		ParseTreeNode * stmts = condition->getNextSibling()->getNextSibling();
		visitStmtList(stmts, &(elsif_stmt_node->stmt_list));
    	symTab->CloseScope();

		// Elsif
		elsif_stmt_node->elsifStmt = 
		  visitElsifStmtList(elsif_stmt_list->getLastChild());

		return elsif_stmt_node;

	} else {
		return 0;
	}

}

IfStmtNode * Analyzer::visitElseStmt(ParseTreeNode * else_stmt) {

	if (else_stmt->hasChild()) {
	
		// ELSE
		IfStmtNode * else_stmt_node = new IfStmtNode();
		else_stmt_node->type = IfStmtNode::ELSE;

    	symTab->OpenScope();
		ParseTreeNode * stmts = else_stmt->getLastChild();
		visitStmtList(stmts, &(else_stmt_node->stmt_list));
    	symTab->CloseScope();

		return else_stmt_node;

	} else {
		return 0;
	}

}

StatementNode * Analyzer::visitCaseStmt(ParseTreeNode * case_stmt) {

	CaseStmtNode * caseNode = new CaseStmtNode();
	ParseTreeNode * target = case_stmt->getFirstChild()->getNextSibling();
	caseNode->target = visitExpression(target);
	ParseTreeNode * whenList = target->getNextSibling()->getNextSibling();
	visitCaseWhenList(whenList, caseNode);
	return caseNode;
}

void Analyzer::visitCaseWhenList(ParseTreeNode * case_list, CaseStmtNode * parent) {

	// <case_when_list> when <choice> <choice_list> => <statement_list> <case_when_list>
	//                | <others_optional>

	CaseWhenNode * whenNode = new CaseWhenNode();
	whenNode->parent = parent;
	if (case_list->getNumChild() > 1) {
		ParseTreeNode * firstChoice = case_list->getFirstChild()->getNextSibling();
		visitChoice(firstChoice, whenNode);
		ParseTreeNode * otherChoices = firstChoice->getNextSibling();
		visitChoiceList(otherChoices, whenNode);
		ParseTreeNode * stmts = otherChoices->getNextSibling();
		visitStmtList(stmts, &(whenNode->stmt_list));
		
	} else {

		// <others_optional> when others => <statement_list>
		//                 |
		ParseTreeNode * others = case_list->getFirstChild();
		if (others->hasChild()) {
			ParseTreeNode * stmts = others->getLastChild();
			visitStmtList(stmts, &(whenNode->stmt_list));
		}
	}
}

void Analyzer::visitChoice(ParseTreeNode * choice, CaseWhenNode * parent) {

	if (choice->getNumChild() == 1) {

		// Non-ranged conditions.
		ExpressionNode * cond = visitExpression(choice->getFirstChild());
		parent->conds.push_back(cond);

	} else {

		// Ranged.
		ExpressionRangeNode * range = new ExpressionRangeNode();
		ExpressionNode * lower = visitExpression(choice->getFirstChild());
		ExpressionNode * upper = visitExpression(choice->getLastChild());
		range->lower = lower;
		range->upper = upper;
		parent->ranges.push_back(range);
	
	}

}

void Analyzer::visitChoiceList(ParseTreeNode * choice_list, CaseWhenNode * parent) {

	// <choice_list> BAR <choice> <choice_list>
	//             |

	if (choice_list->hasChild()) {
		ParseTreeNode * choice = choice_list->getFirstChild()->getNextSibling();
		visitChoice(choice, parent);
		ParseTreeNode * choiceList = choice->getNextSibling();
		visitChoiceList(choiceList, parent);
	}
}

StatementNode * Analyzer::visitLoopStmt(ParseTreeNode * node) {

	LoopStmtNode * loop = new LoopStmtNode();

    symTab->OpenScope();
	ParseTreeNode * id_opt = node->getFirstChild();
	ParseTreeNode * iter_opt = id_opt->getNextSibling();


	// For or while loop.
	if (iter_opt->hasChild()) {
		ParseTreeNode * for_or_while = iter_opt->getFirstChild();


		// While Loop:
		if (for_or_while->getToken()->getType() == Token::WHILE) {
			ParseTreeNode * exp = for_or_while->getNextSibling();
			loop->whileCond= visitExpression(exp);
			loop->type = LoopStmtNode::WHILE;

			// TODO:FIXME Must do type checking here.

		// For loop.
		} else {

			ParseTreeNode * id = for_or_while->getNextSibling();
			ParseTreeNode * reverse = id->getNextSibling()->getNextSibling();
			ParseTreeNode * range = reverse->getNextSibling();

			// TODO:FIXME put in symtab here.

			
			
		}
	}

	// Do the iteration here.
	ParseTreeNode * stmts = iter_opt->getNextSibling()->getFirstChild()->getNextSibling();
	visitStmtList(stmts, &(loop->stmt_list));

    symTab->CloseScope();
	return loop;
}


RangeNode * Analyzer::visitDiscreteRange(ParseTreeNode * discrete_range) {

	//<discrete_range> <range>
	//               | subtype <subtype>

	if (discrete_range->getNumChild() == 1) {
		ParseTreeNode * range = discrete_range->getFirstChild();
		ParseTreeNode * lower = range->getFirstChild();
		ParseTreeNode * upper = range->getLastChild();

		ExpressionRangeNode * rangeNode = new ExpressionRangeNode();
		rangeNode->lower = visitSimpleExp(lower);
		rangeNode->upper = visitSimpleExp(upper);

		return rangeNode;

	} else {

		// TODO:FIXME do subtype ranges. or not.
		return 0;
	}
}

StatementNode * Analyzer::visitBlockStmt(ParseTreeNode * node) {
	BlockStmtNode * block = new BlockStmtNode();

	ParseTreeNode * pt_block_id = node->getFirstChild();
	ParseTreeNode * pt_decl_part_optional = pt_block_id->getNextSibling();
	ParseTreeNode * pt_stmt_list = pt_decl_part_optional->getNextSibling()->getNextSibling();

	if (pt_decl_part_optional->hasChild()) {
		
	}
}

// Used by name only:XXX QP
void Analyzer::visitExpressionList(ParseTreeNode * exprl,
		vector<set<Symbol *> * > *exprs) {
	// <expression_list> , <expression> <expression_list>
	if (!exprl->hasChild())
		return;
	// visit epxression
	ParseTreeNode * expr = exprl->getFirstChild()->getNextSibling();
	ParseTreeNode * expr_list = expr->getNextSibling();

	exprs->push_back(visitExpression(expr)->evalPossibleTypes());
	if (expr_list->hasChild()) 
		visitExpressionList(expr_list, exprs) ;
}

/*
 * Most general of all expressions.
 */
ExpressionNode * Analyzer::visitExpression(ParseTreeNode * exp) {
	if (exp->getNumChild() == 1) {
		return visitRelation(exp->getFirstChild());
	} else {
		ParseTreeNode * subExp = exp->getFirstChild();
		ParseTreeNode * op1 = subExp->getNextSibling();
		ParseTreeNode * op2 = op1->getNextSibling();

		BinaryExpNode * expression = new BinaryExpNode();
        if(op2->hasToken()) {
			if (op2->getToken()->getType() == Token::THEN) {
				expression->op = BinaryExpNode::AND_THEN;

			} else if (op2->getToken()->getType() == Token::ELSE) {
				expression->op = BinaryExpNode::OR_ELSE;
			}
		} else {
			if (op1->getToken()->getType() == Token::AND) {
				expression->op = BinaryExpNode::AND;

			} else if (op1->getToken()->getType() == Token::OR) {
				expression->op = BinaryExpNode::OR;
			}
		}
		expression->leftOperand = visitExpression(subExp);
		expression->leftOperand->parentExpression = expression;
		expression->rightOperand = visitRelation(exp->getLastChild());
		expression->rightOperand->parentExpression = expression;
		return expression;
	}
}

ExpressionNode * Analyzer::visitRelation(ParseTreeNode * pt_rel) {
	/*
		<relation> <relation> =  <simple_expression>
		         | <relation> /= <simple_expression>
		         | <relation> lt <simple_expression>
		         | <relation> <= <simple_expression>
		         | <relation> gt <simple_expression>
		         | <relation> >= <simple_expression>
		         |               <simple_expression>
	*/
	if (pt_rel->getNumChild() == 1) {
		return visitSimpleExp(pt_rel->getFirstChild());

	} else {
		ParseTreeNode * subRel = pt_rel->getFirstChild();
		ParseTreeNode * op = subRel->getNextSibling();
		ParseTreeNode * simpExp = op->getNextSibling();
		BinaryExpNode * relation = new BinaryExpNode();
		if (op->getToken()->getType() == Token::EQ) {
			relation->op = BinaryExpNode::EQ;
		} else if (op->getToken()->getType() == Token::SLASH_EQ) {
			relation->op = BinaryExpNode::SLASH_EQ;
		} else if (op->getToken()->getType() == Token::LT) {
			relation->op = BinaryExpNode::LT;
		} else if (op->getToken()->getType() == Token::LTE) {
			relation->op = BinaryExpNode::LTE;
		} else if (op->getToken()->getType() == Token::GT) {
			relation->op = BinaryExpNode::GT;
		} else if (op->getToken()->getType() == Token::GTE) {
			relation->op = BinaryExpNode::GTE;
		}

		relation->leftOperand = visitRelation(subRel);
		relation->leftOperand->parentExpression = relation;
		relation->rightOperand = visitSimpleExp(pt_rel->getLastChild());
		relation->rightOperand->parentExpression = relation;

		return relation;
	}
}

ExpressionNode * Analyzer::visitSimpleExp(ParseTreeNode * exp) {

	/*
		<simple_expression> <simple_expression> + <term>
		                  | <simple_expression> - <term>
		                  | <simple_expression> & <term>
		                  |                       <term>
	*/
	if (exp->getNumChild() == 1) {
		return visitTerm(exp->getFirstChild());
	} else {
		ParseTreeNode * subExp = exp->getFirstChild();
		ParseTreeNode * op = subExp->getNextSibling();
		ParseTreeNode * term = op->getNextSibling();
		BinaryExpNode * simpleExpression = new BinaryExpNode();

		if (op->getToken()->getType() == Token::ADD) {
			simpleExpression->op = BinaryExpNode::ADD;
		} else if (op->getToken()->getType() == Token::SUB) {
			simpleExpression->op = BinaryExpNode::SUB;
		} else if (op->getToken()->getType() == Token::AMP) {
			simpleExpression->op = BinaryExpNode::AMP;
		}

		simpleExpression->leftOperand = visitSimpleExp(subExp);
		simpleExpression->leftOperand->parentExpression = simpleExpression;
		simpleExpression->rightOperand = visitTerm(exp->getLastChild());
		simpleExpression->rightOperand->parentExpression = simpleExpression;
		return simpleExpression;
	}
}

ExpressionNode * Analyzer::visitTerm(ParseTreeNode * term) {

	/*
		<term> <term> *   <factor>
		     | <term> /   <factor>
		     | <term> mod <factor>
		     |            <factor>
	*/
	if (term->getNumChild() == 1) {
		return visitFactor(term->getFirstChild());
	} else {
		ParseTreeNode * subTerm = term->getFirstChild();
		ParseTreeNode * op = subTerm->getNextSibling();
		ParseTreeNode * factor = op->getNextSibling();
		BinaryExpNode * termNode = new BinaryExpNode();

		if (op->getToken()->getType() == Token::MULT) {
			termNode->op = BinaryExpNode::MULT;
		} else if (op->getToken()->getType() == Token::DIV) {
			termNode->op = BinaryExpNode::DIV;
		} else if (op->getToken()->getType() == Token::MOD) {
			termNode->op = BinaryExpNode::MOD;
		}

		termNode->leftOperand = visitTerm(subTerm);
		termNode->leftOperand->parentExpression = termNode;
		termNode->rightOperand = visitFactor(term->getLastChild());
		termNode->rightOperand->parentExpression = termNode;
		return termNode;
	}
}

ExpressionNode * Analyzer::visitFactor(ParseTreeNode * factor) {

	/*
		<factor> not <primary>
		       | abs <primary>
		       |  +  <primary>
		       |  -  <primary>
		       |     <primary>
		       | <factor> ** <primary>
	*/
	if (factor->getNumChild() == 1) {
		return visitPrimary(factor->getFirstChild());

	} else if (factor->getNumChild() == 3) {
		BinaryExpNode * powerNode = new BinaryExpNode();
		powerNode->leftOperand = visitFactor(factor->getFirstChild());
		powerNode->op = BinaryExpNode::EXP;
		powerNode->rightOperand = visitPrimary(factor->getLastChild());
		return powerNode;

	} else {
		ParseTreeNode * op = factor->getFirstChild();
		UnaryExpNode * primary = new UnaryExpNode();
		if (op->getToken()->getType() == Token::ADD) {
			primary->op = UnaryExpNode::ADD;
		} else if (op->getToken()->getType() == Token::SUB) {
			primary->op = UnaryExpNode::SUB;
		} else if (op->getToken()->getType() == Token::NOT) {
			primary->op = UnaryExpNode::NOT;
		} else if (op->getToken()->getType() == Token::ABS) {
			primary->op = UnaryExpNode::ABS;
		}
		primary->rightOperand = visitPrimary(factor->getLastChild());

		return primary;
	}
}

ExpressionNode * Analyzer::visitPrimary(ParseTreeNode * exp) {
	/*
		<primary> <literal>
		        | <name>
		        | <aggregate>
		        | ( <expression> )
	*/


	char * type = (exp->getFirstChild())->getName();

	// ( ) priority.
	if (exp->getNumChild() == 3) {
		return visitExpression(exp->getFirstChild()->getNextSibling());
	}

	if(strcmp(type, "<literal>") == 0) {
		ParseTreeNode * pt_literal = exp->getFirstChild()->getFirstChild();
		Token * tk_literal = pt_literal->getToken();
		UnaryExpNode * unaryExp = new UnaryExpNode();
		unaryExp->op = UnaryExpNode::LITERAL;

		// Deal with the literals.
		if (tk_literal->getType() == Token::INT_LIT) {
			unaryExp->literal = tk_literal;

		} else if (tk_literal->getType() == Token::FLOAT_LIT) {
			unaryExp->literal = tk_literal;

		} else if (tk_literal->getType() == Token::STRING_LIT) {
			unaryExp->literal = tk_literal;
		}

		return unaryExp;

	} else if(strcmp(type, "<name>") == 0) {
		ParseTreeNode * pt_name = exp->getFirstChild();
		UnaryExpNode * unaryExp = new UnaryExpNode();
		unaryExp->op = UnaryExpNode::NAME;
		unaryExp->name = visitName(pt_name);
	}

	ExpressionNode * tmp = new ExpressionNode();
	return tmp;
}

/*
void Analyzer::visitIdOptional(ParseTreeNode *stmt_list) {
}
*/

// visit body. 
PackageBodyNode * Analyzer::visitPackageBody(ParseTreeNode *node) {
	//  body id is <body_declar_list> begin <statement_list> <exception_part_optional> end <id_optional>
    
	// no need to check for valid keyword, otherwise it's an error.
    ParseTreeNode * id_node = node->getFirstChild()->getNextSibling();
    ParseTreeNode * body_declar_list = id_node->getNextSibling()->getNextSibling();
    ParseTreeNode * stmt_list = body_declar_list->getNextSibling()->getNextSibling();
    ParseTreeNode * expt_optional = stmt_list->getNextSibling();
    ParseTreeNode * id_optional = expt_optional->getNextSibling()->getNextSibling(); 
   
    string pkg_id = id_node->getToken()->getValue();
	if (id_optional->hasChild() &&
			pkg_id != id_optional->getFirstChild()->getToken()->getValue()) {
		/*
		unsigned int line = id_optional->getFirstChild()->getToken()->getLine();
		unsigned int col = id_optional->getFirstChild()->getToken()->getCol();
		*/
		PRINT_ERRORS(filename, id_optional->getFirstChild()->getToken(),
				IdMismatch);
	}

	PackageBodyNode * body = new PackageBodyNode(); 
	body->name= id_node->getToken()->getValue();

	bool defined;

	symTab->definePackage(body->name, &defined, &(body->declared));

	if (defined) {
		cerr << "Redefinition of package body " << body->name << "." << endl;
	}

	visitBodyDeclareList(body_declar_list);
    symTab->OpenScope();
	visitStmtList(stmt_list, &(body->stmt_list));
    symTab->CloseScope();

	return body;
}


// Visit a <compilation_unit> node and return a new PackageBodyNode if a package_body is a child
// otherwise return 0 (a package_spec is a child)
PackageBodyNode* Analyzer::visitCompilationUnit(ParseTreeNode * node){
    
    // Rules: <compilation_unit> -> <package_declar> ->  package <package_spec_or_body> ;
    ParseTreeNode *pack_spec_or_body = node->getFirstChild()->getFirstChild()->getNextSibling();
	ParseTreeNode * first_child = pack_spec_or_body->getFirstChild();
	if (first_child->getToken()->getType() == Token::BODY)  {
		// Handle Body.
		return visitPackageBody(pack_spec_or_body);
	} else {
		// Handle Spec.
        visitPackageDeclaration(pack_spec_or_body);
	}

    return 0;
}

// Process the children of this node and add any new 
// packages to the list of packages
void Analyzer::visitCompilationUnits(ParseTreeNode * node, vector<PackageBodyNode*> &pkg_list) {

    // Rules: <compilations> -> <pragma_list> <compilation_unit> <compilations>
    
	// Check for def and use HERE!
    // Handle empty <compilations>
	if (!node->hasChild()) {
		return;
	}
    
    // If node is not empty then it must have these
    // three children
	ParseTreeNode * pragma = node->getFirstChild();
	ParseTreeNode * cu = pragma->getNextSibling();
	ParseTreeNode * cu_list = cu->getNextSibling();

	// TODO:  Pragma.

	PackageBodyNode * pkg = visitCompilationUnit(cu);

    // Only add the node if it was not a package declaration
	if (pkg) {
	    pkg_list.push_back(pkg);
    }

    // Recursively add the rest of the compilation units
    visitCompilationUnits(cu_list, pkg_list);

}



void Analyzer::printSymbolTable() const {
    cout<<*symTab;
}
