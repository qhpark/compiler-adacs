#include "symtab.hpp"
#include "debug.hpp"
#include <string>
#include <iostream>

#include "extree.hpp"

using namespace std;

// QP: XXX FIXME
// global symbol table, initialized by analyzer.
SymbolTable *symTab_G;

/********************************************************************
 * Symbol
 * -------------
 ********************************************************************/
Symbol::Symbol() : 
  symType (Symbol::SYM_UNKNOWN), 
  dataType(Symbol::DATA_UNKNOWN), name(""), typeDesc(0),
    nextSameScope(0), nextSameName(0), nextOverload(0), symTypeDesc(0) { 
    
}

Symbol::Symbol(SymbolType typ) : 
    symType(typ),
    dataType(Symbol::DATA_UNKNOWN), name(""), typeDesc(0),
    nextSameScope(0), nextSameName(0) , nextOverload(0), symTypeDesc(0){

}
    
Symbol::~Symbol() { }

void Symbol::setName(std::string aName) { name = aName; }
std::string Symbol::getName() { return name; }

void Symbol::setSymbolType(Symbol::SymbolType s) { symType = s; }
Symbol::SymbolType Symbol::getSymbolType() { return symType; }

void Symbol::setDataType(Symbol::DataType s) { dataType = s; }
Symbol::DataType Symbol::getDataType() { return dataType; }

void Symbol::setDef(unsigned int line) { def = line; }
unsigned int Symbol::getDef() { return def; }

void Symbol::addUse(unsigned int line) { uses.push_back(line); }
std::vector<unsigned int> * Symbol::getUses() { return &uses; }

void Symbol::setTypeDescriptor(TypeDescriptor *desc) { typeDesc = desc; }
TypeDescriptor* Symbol::getTypeDescriptor() {    return typeDesc; }

void Symbol::setTypeDescriptorSym(Symbol *sym) { symTypeDesc = sym; }
Symbol* Symbol::getTypeDescriptorSym() { return symTypeDesc; }

std::ostream& operator<<( std::ostream &out, const Symbol &s) {
	out << "Symbol: name=" << s.name;
    if(s.symTypeDesc != 0) {
        out<< " type-name: "<<s.symTypeDesc->name;
    }
    if(s.typeDesc != 0) {
        out<< *s.typeDesc;
    }
	return out;
}

/********************************************************************
 * Symbol Table.
 * -------------
 ********************************************************************/
SymbolTable::SymbolTable() : depth(0) { 
    // Initialize scope list and name list
    name_list = 0;
    rngScopeDisplay.push_back((Symbol*)0);

    buildInitialEnvironment();
}

SymbolTable::~SymbolTable() { }


string SymbolTable::getNextAnonName(string name) {
    char buf[5];
    anon_ind += 1;
    sprintf(buf, "%d", anon_ind);
    return name + buf;
}

// Search name_list to find innermost symbol that matches name
SymbolTable::name_list_node * SymbolTable::getNameNode(std::string name, bool usePkgs) {
    SymbolTable::name_list_node* curNode = name_list;
    while(curNode) {
        if (curNode->symbol->getName() == name) { return curNode; }
        curNode = curNode->next;
    }

    return 0;

}


Symbol * SymbolTable::getSymbolByName(std::string name, bool usePkgs) {
    
    name_list_node* curNode = getNameNode(name, usePkgs);
    if (curNode) {
        return curNode->symbol;
    } else {
        return 0;
    }
}

std::ostream& operator<<( std::ostream &out, const SymbolTable &S) {
    out << "Symbol Table "<<endl;
    out << "================================="<<endl;
    int numScopes = S.rngScopeDisplay.size();
    for(int i = 0; i < numScopes; i += 1) {
      out<<"At Scope: "<<i<<endl;
      Symbol* sym = S.rngScopeDisplay[i];
      while(sym) {
          out<<*sym<<endl;
          sym = sym->nextSameScope;
      } 
    }
    out << endl;
    return out;
}

// Check if subprog is a valid overload given prev_subprog (and any other overloads)
bool SymbolTable::AddOverload(Symbol* subprog, Symbol* prev_subprog) {
    return false;
}

// Add symbol to symbol table at the current scope
void SymbolTable::AddSymbol(Symbol* sym) {
    ASSERT (sym != (Symbol*)0, "Symbol can not be null.");

    // Get pointer to first symbol in scope
    Symbol* headSym = rngScopeDisplay[depth];

    // Make sure no symbol with same name declared in this scope
    Symbol* curSym = headSym;
    while(curSym) {
        if(curSym->getName() == sym->getName()) {
            Symbol::SymbolType sym_t = curSym->getSymbolType();
            TypeDescriptor* type_desc = curSym->getTypeDescriptor();
            // Allow overloads of functions and procedures
            if(type_desc == 0 ||
               (type_desc->getType() == TypeDescriptor::PROC|| type_desc->getType() == TypeDescriptor::FUNC)) {
                    ASSERT(false, "Name clash");
            } else {
                // Check for overloading
                if(!AddOverload(curSym, sym)) {
                    ASSERT(false, "A subprogram with the same signature already exists.");
                }
            }
        }
        curSym = curSym->nextSameScope;
    }

    // Look for same name at a previous scope and if we are overriding
    // it we need to update name_list
    name_list_node* prevName = getNameNode(sym->getName(), false);
    
    if(prevName) {
        // Update name_list with new symbol for existing name
        sym->nextSameName = prevName->symbol;
        prevName->symbol = sym;
    } else {
        // Add to name_list
        name_list_node* nn = new name_list_node();
        nn->symbol = sym;
        nn->next = name_list;
        name_list = nn;
    }
    
    // Add to beginning of current scope
    if(headSym) {
        sym->nextSameScope = headSym;
    }
    rngScopeDisplay[depth] = sym;

	if (debugOpt.inDebug("dumpstonly")) {
        cout<<*this;
    }
}

// Create a new scope
void SymbolTable::OpenScope() {
    depth += 1;
    rngScopeDisplay.push_back((Symbol*)0);
}

// Close the current scope
void SymbolTable::CloseScope() {
    ASSERT(depth > 0, "At the top-most scope.");

    Symbol *curSym = rngScopeDisplay.back();
    
    while(curSym) {
        Symbol* nextSym = curSym->nextSameScope;
        if(curSym->nextSameName) {
            // Update the name_list with symbol in a higher scope
            name_list_node* nextsym = getNameNode(curSym->getName(), false);
            if(nextsym) {
                nextsym->symbol = curSym->nextSameName;
            } else {
                ASSERT(false, "Symbol Table is inconsistent.");
            }
        } else {

            name_list_node* nextsym = getNameNode(curSym->getName(), false);
            // delete by copying over the next node's information

            if(!nextsym) {
                ASSERT(false, "Symbol Table is inconsistent.");
            }
            name_list_node* nn = nextsym->next;
            if(nn) {
                nextsym->next = nn->next;
                nextsym->symbol = nn->symbol;
                delete nn;
            }
        }
        curSym->nextSameScope = (Symbol*)0;
        
        // Deallocate the symbol
        delete curSym;
        
        curSym = nextSym;
    }

    rngScopeDisplay.pop_back();
    depth -= 1;
}

// Add public identifiers from package to current scope
void SymbolTable::UsePackage(std::string pkg_name) {
//TODO: Load names from package and link in with the current level
}

// Remove public identifiers from package from current scope
void SymbolTable::RemovePackage(std::string pkg_name) {
// TODO: Remove the link to the given package at the current scope
}

// Add the initial symbols
void SymbolTable::buildInitialEnvironment() {
   // Universal standard types 
   Symbol* uni_integer = new Symbol(Symbol::UNI_INT);
   uni_integer->setName("__integer");
   this->AddSymbol(uni_integer);

   Symbol* uni_float = new Symbol(Symbol::UNI_FLOAT);
   uni_float->setName("__float");
   this->AddSymbol(uni_float);

   Symbol* uni_string = new Symbol(Symbol::UNI_STRING);
   uni_string->setName("__string");
   this->AddSymbol(uni_string);

   Symbol* uni_boolean = new Symbol(Symbol::UNI_BOOLEAN);
   uni_boolean->setName("__boolean");
   this->AddSymbol(uni_boolean);
  
   // Standard types
   Symbol* integer_t = new Symbol(Symbol::TYPE);
   integer_t->setName("integer");
   NewTypeDescriptor * integer_t_d = new NewTypeDescriptor();
   integer_t_d->parentType = uni_integer;
   integer_t->setTypeDescriptor(integer_t_d);
   this->AddSymbol(integer_t);
   
   Symbol* float_t = new Symbol(Symbol::TYPE);
   float_t->setName("float");
   NewTypeDescriptor * float_t_d = new NewTypeDescriptor();
   float_t_d->parentType = uni_float;
   float_t->setTypeDescriptor(float_t_d);
   this->AddSymbol(float_t);
   
   Symbol* string_t = new Symbol(Symbol::TYPE);
   string_t->setName("string");
   NewTypeDescriptor * string_t_d = new NewTypeDescriptor();
   string_t_d->parentType = uni_string;
   string_t->setTypeDescriptor(string_t_d);
   this->AddSymbol(string_t);
   
   Symbol* boolean_t = new Symbol(Symbol::TYPE);
   boolean_t->setName("boolean");
   NewTypeDescriptor * boolean_t_d = new NewTypeDescriptor();
   boolean_t_d->parentType = uni_boolean;
   boolean_t->setTypeDescriptor(boolean_t_d);
   this->AddSymbol(boolean_t);
  
	// Standard Unary operation.

	// Unary + int
	Symbol* op_add_int = new Symbol(Symbol::TYPE);
	op_add_int->setName("+");
	FuncDescriptor * op_add_int_d = new FuncDescriptor();	
	op_add_int->setTypeDescriptor(op_add_int_d);
	FormalParameter * op_add_int_p = new FormalParameter();
	op_add_int_p->type = uni_integer; 
	op_add_int_p->name = 0; // These are not formal names. 
	op_add_int_p->in = true;
	op_add_int_p->out = false; 
	op_add_int_d->formalParams.push_back(op_add_int_p);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_add_int);

	// Unary + float
	Symbol* op_add_float = new Symbol(Symbol::TYPE);
	op_add_float->setName("+");
	FuncDescriptor * op_add_float_d = new FuncDescriptor();	
	op_add_float->setTypeDescriptor(op_add_float_d);
	FormalParameter * op_add_float_p = new FormalParameter();
	op_add_float_p->type = uni_float; 
	op_add_float_p->name = 0; // These are not formal names. 
	op_add_float_p->in = true;
	op_add_float_p->out = false; 
	op_add_float_d->formalParams.push_back(op_add_float_p);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_add_float);

	// Unary - int
	Symbol* op_sub_int = new Symbol(Symbol::TYPE);
	op_sub_int->setName("-");
	FuncDescriptor * op_sub_int_d = new FuncDescriptor();	
	op_sub_int->setTypeDescriptor(op_sub_int_d);
	FormalParameter * op_sub_int_p = new FormalParameter();
	op_sub_int_p->type = uni_integer; 
	op_sub_int_p->name = 0; // These are not formal names. 
	op_sub_int_p->in = true;
	op_sub_int_p->out = false; 
	op_sub_int_d->formalParams.push_back(op_sub_int_p);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_sub_int);

	// Unary - float
	Symbol* op_sub_float = new Symbol(Symbol::TYPE);
	op_sub_float->setName("-");
	FuncDescriptor * op_sub_float_d = new FuncDescriptor();	
	op_sub_float->setTypeDescriptor(op_sub_float_d);
	FormalParameter * op_sub_float_p = new FormalParameter();
	op_sub_float_p->type = uni_float; 
	op_sub_float_p->name = 0; // These are not formal names. 
	op_sub_float_p->in = true;
	op_sub_float_p->out = false; 
	op_sub_float_d->formalParams.push_back(op_sub_float_p);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_sub_float);

	// Unary NOT boolean
	Symbol* op_not_bool = new Symbol(Symbol::TYPE);
	op_not_bool->setName("not");
	FuncDescriptor * op_not_bool_d = new FuncDescriptor();	
	op_not_bool->setTypeDescriptor(op_not_bool_d);
	FormalParameter * op_not_bool_p = new FormalParameter();
	op_not_bool_p->type = uni_boolean; 
	op_not_bool_p->name = 0; // These are not formal names. 
	op_not_bool_p->in = true;
	op_not_bool_p->out = false; 
	op_not_bool_d->formalParams.push_back(op_not_bool_p);
	op_add_int_d->returnType = boolean_t;
	this->AddSymbol(op_not_bool);

	// Unary ABS int
	Symbol* op_abs_int = new Symbol(Symbol::TYPE);
	op_abs_int->setName("abs");
	FuncDescriptor * op_abs_int_d = new FuncDescriptor();	
	op_abs_int->setTypeDescriptor(op_abs_int_d);
	FormalParameter * op_abs_int_p = new FormalParameter();
	op_abs_int_p->type = uni_integer; 
	op_abs_int_p->name = 0; // These are not formal names. 
	op_abs_int_p->in = true;
	op_abs_int_p->out = false; 
	op_abs_int_d->formalParams.push_back(op_abs_int_p);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_abs_int);

	// Unary abs float
	Symbol* op_abs_float = new Symbol(Symbol::TYPE);
	op_abs_float->setName("abs");
	FuncDescriptor * op_abs_float_d = new FuncDescriptor();	
	op_abs_float->setTypeDescriptor(op_abs_float_d);
	FormalParameter * op_abs_float_p = new FormalParameter();
	op_abs_float_p->type = uni_float; 
	op_abs_float_p->name = 0; // These are not formal names. 
	op_abs_float_p->in = true;
	op_abs_float_p->out = false; 
	op_abs_float_d->formalParams.push_back(op_abs_float_p);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_abs_float);


	// Standard Binary operation.

	// Unary + int int
	Symbol* op_add_int_int = new Symbol(Symbol::TYPE);
	op_add_int_int->setName("+");
	FuncDescriptor * op_add_int_int_d = new FuncDescriptor();	
	op_add_int_int->setTypeDescriptor(op_add_int_int_d);
	FormalParameter * op_add_int_int_p1 = new FormalParameter();
	op_add_int_int_p1->type = uni_integer; 
	op_add_int_int_p1->name = 0; // These are not formal names. 
	op_add_int_int_p1->in = true;
	op_add_int_int_p1->out = false; 
	op_add_int_int_d->formalParams.push_back(op_add_int_int_p1);
	FormalParameter * op_add_int_int_p2 = new FormalParameter();
	op_add_int_int_p2->type = uni_integer; 
	op_add_int_int_p2->name = 0; // These are not formal names. 
	op_add_int_int_p2->in = true;
	op_add_int_int_p2->out = false; 
	op_add_int_int_d->formalParams.push_back(op_add_int_int_p2);
	op_add_int_d->returnType = integer_t;
	//this->AddSymbol(op_add_int_int);

	// Unary + float float
	Symbol* op_add_float_float = new Symbol(Symbol::TYPE);
	op_add_float_float->setName("+");
	FuncDescriptor * op_add_float_float_d = new FuncDescriptor();	
	op_add_float_float->setTypeDescriptor(op_add_float_float_d);
	FormalParameter * op_add_float_float_p1 = new FormalParameter();
	op_add_float_float_p1->type = uni_float; 
	op_add_float_float_p1->name = 0; // These are not formal names. 
	op_add_float_float_p1->in = true;
	op_add_float_float_p1->out = false; 
	op_add_float_float_d->formalParams.push_back(op_add_float_float_p1);
	FormalParameter * op_add_float_float_p2 = new FormalParameter();
	op_add_float_float_p2->type = uni_float; 
	op_add_float_float_p2->name = 0; // These are not formal names. 
	op_add_float_float_p2->in = true;
	op_add_float_float_p2->out = false; 
	op_add_float_float_d->formalParams.push_back(op_add_float_float_p2);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_add_float_float);

	// Unary - int int
	Symbol* op_sub_int_int = new Symbol(Symbol::TYPE);
	op_sub_int_int->setName("-");
	FuncDescriptor * op_sub_int_int_d = new FuncDescriptor();	
	op_sub_int_int->setTypeDescriptor(op_sub_int_int_d);
	FormalParameter * op_sub_int_int_p1 = new FormalParameter();
	op_sub_int_int_p1->type = uni_integer; 
	op_sub_int_int_p1->name = 0; // These are not formal names. 
	op_sub_int_int_p1->in = true;
	op_sub_int_int_p1->out = false; 
	op_sub_int_int_d->formalParams.push_back(op_sub_int_int_p1);
	FormalParameter * op_sub_int_int_p2 = new FormalParameter();
	op_sub_int_int_p2->type = uni_integer; 
	op_sub_int_int_p2->name = 0; // These are not formal names. 
	op_sub_int_int_p2->in = true;
	op_sub_int_int_p2->out = false; 
	op_sub_int_int_d->formalParams.push_back(op_sub_int_int_p2);
	op_add_int_d->returnType = integer_t;
	//this->AddSymbol(op_sub_int_int);

	// Unary - float float
	Symbol* op_sub_float_float = new Symbol(Symbol::TYPE);
	op_sub_float_float->setName("-");
	FuncDescriptor * op_sub_float_float_d = new FuncDescriptor();	
	op_sub_float_float->setTypeDescriptor(op_sub_float_float_d);
	FormalParameter * op_sub_float_float_p1 = new FormalParameter();
	op_sub_float_float_p1->type = uni_float; 
	op_sub_float_float_p1->name = 0; // These are not formal names. 
	op_sub_float_float_p1->in = true;
	op_sub_float_float_p1->out = false; 
	op_sub_float_float_d->formalParams.push_back(op_sub_float_float_p1);
	FormalParameter * op_sub_float_float_p2 = new FormalParameter();
	op_sub_float_float_p2->type = uni_float; 
	op_sub_float_float_p2->name = 0; // These are not formal names. 
	op_sub_float_float_p2->in = true;
	op_sub_float_float_p2->out = false; 
	op_sub_float_float_d->formalParams.push_back(op_sub_float_float_p2);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_sub_float_float);

	// Unary & int int
	Symbol* op_amp_int_int = new Symbol(Symbol::TYPE);
	op_amp_int_int->setName("&");
	FuncDescriptor * op_amp_int_int_d = new FuncDescriptor();	
	op_amp_int_int->setTypeDescriptor(op_amp_int_int_d);
	FormalParameter * op_amp_int_int_p1 = new FormalParameter();
	op_amp_int_int_p1->type = uni_integer; 
	op_amp_int_int_p1->name = 0; // These are not formal names. 
	op_amp_int_int_p1->in = true;
	op_amp_int_int_p1->out = false; 
	op_amp_int_int_d->formalParams.push_back(op_amp_int_int_p1);
	FormalParameter * op_amp_int_int_p2 = new FormalParameter();
	op_amp_int_int_p2->type = uni_integer; 
	op_amp_int_int_p2->name = 0; // These are not formal names. 
	op_amp_int_int_p2->in = true;
	op_amp_int_int_p2->out = false; 
	op_amp_int_int_d->formalParams.push_back(op_amp_int_int_p2);
	this->AddSymbol(op_amp_int_int);

	// Unary * int int
	Symbol* op_mult_int_int = new Symbol(Symbol::TYPE);
	op_mult_int_int->setName("*");
	FuncDescriptor * op_mult_int_int_d = new FuncDescriptor();	
	op_mult_int_int->setTypeDescriptor(op_mult_int_int_d);
	FormalParameter * op_mult_int_int_p1 = new FormalParameter();
	op_mult_int_int_p1->type = uni_integer; 
	op_mult_int_int_p1->name = 0; // These are not formal names. 
	op_mult_int_int_p1->in = true;
	op_mult_int_int_p1->out = false; 
	op_mult_int_int_d->formalParams.push_back(op_mult_int_int_p1);
	FormalParameter * op_mult_int_int_p2 = new FormalParameter();
	op_mult_int_int_p2->type = uni_integer; 
	op_mult_int_int_p2->name = 0; // These are not formal names. 
	op_mult_int_int_p2->in = true;
	op_mult_int_int_p2->out = false; 
	op_mult_int_int_d->formalParams.push_back(op_mult_int_int_p2);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_mult_int_int);

	// Unary * float float
	Symbol* op_mult_float_float = new Symbol(Symbol::TYPE);
	op_mult_float_float->setName("*");
	FuncDescriptor * op_mult_float_float_d = new FuncDescriptor();	
	op_mult_float_float->setTypeDescriptor(op_mult_float_float_d);
	FormalParameter * op_mult_float_float_p1 = new FormalParameter();
	op_mult_float_float_p1->type = uni_float; 
	op_mult_float_float_p1->name = 0; // These are not formal names. 
	op_mult_float_float_p1->in = true;
	op_mult_float_float_p1->out = false; 
	op_mult_float_float_d->formalParams.push_back(op_mult_float_float_p1);
	FormalParameter * op_mult_float_float_p2 = new FormalParameter();
	op_mult_float_float_p2->type = uni_float; 
	op_mult_float_float_p2->name = 0; // These are not formal names. 
	op_mult_float_float_p2->in = true;
	op_mult_float_float_p2->out = false; 
	op_mult_float_float_d->formalParams.push_back(op_mult_float_float_p2);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_mult_float_float);

	// Unary / int int
	Symbol* op_div_int_int = new Symbol(Symbol::TYPE);
	op_div_int_int->setName("/");
	FuncDescriptor * op_div_int_int_d = new FuncDescriptor();	
	op_div_int_int->setTypeDescriptor(op_div_int_int_d);
	FormalParameter * op_div_int_int_p1 = new FormalParameter();
	op_div_int_int_p1->type = uni_integer; 
	op_div_int_int_p1->name = 0; // These are not formal names. 
	op_div_int_int_p1->in = true;
	op_div_int_int_p1->out = false; 
	op_div_int_int_d->formalParams.push_back(op_div_int_int_p1);
	FormalParameter * op_div_int_int_p2 = new FormalParameter();
	op_div_int_int_p2->type = uni_integer; 
	op_div_int_int_p2->name = 0; // These are not formal names. 
	op_div_int_int_p2->in = true;
	op_div_int_int_p2->out = false; 
	op_div_int_int_d->formalParams.push_back(op_div_int_int_p2);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_div_int_int);

	// Unary / float float
	Symbol* op_div_float_float = new Symbol(Symbol::TYPE);
	op_div_float_float->setName("/");
	FuncDescriptor * op_div_float_float_d = new FuncDescriptor();	
	op_div_float_float->setTypeDescriptor(op_div_float_float_d);
	FormalParameter * op_div_float_float_p1 = new FormalParameter();
	op_div_float_float_p1->type = uni_float; 
	op_div_float_float_p1->name = 0; // These are not formal names. 
	op_div_float_float_p1->in = true;
	op_div_float_float_p1->out = false; 
	op_div_float_float_d->formalParams.push_back(op_div_float_float_p1);
	FormalParameter * op_div_float_float_p2 = new FormalParameter();
	op_div_float_float_p2->type = uni_float; 
	op_div_float_float_p2->name = 0; // These are not formal names. 
	op_div_float_float_p2->in = true;
	op_div_float_float_p2->out = false; 
	op_div_float_float_d->formalParams.push_back(op_div_float_float_p2);
	op_add_int_d->returnType = float_t;
	//this->AddSymbol(op_div_float_float);

	// Unary % int int
	Symbol* op_mod_int_int = new Symbol(Symbol::TYPE);
	op_mod_int_int->setName("%");
	FuncDescriptor * op_mod_int_int_d = new FuncDescriptor();	
	op_mod_int_int->setTypeDescriptor(op_mod_int_int_d);
	FormalParameter * op_mod_int_int_p1 = new FormalParameter();
	op_mod_int_int_p1->type = uni_integer; 
	op_mod_int_int_p1->name = 0; // These are not formal names. 
	op_mod_int_int_p1->in = true;
	op_mod_int_int_p1->out = false; 
	op_mod_int_int_d->formalParams.push_back(op_mod_int_int_p1);
	FormalParameter * op_mod_int_int_p2 = new FormalParameter();
	op_mod_int_int_p2->type = uni_integer; 
	op_mod_int_int_p2->name = 0; // These are not formal names. 
	op_mod_int_int_p2->in = true;
	op_mod_int_int_p2->out = false; 
	op_mod_int_int_d->formalParams.push_back(op_mod_int_int_p2);
	op_add_int_d->returnType = integer_t;
	this->AddSymbol(op_mod_int_int);

}

Symbol * SymbolTable::definePackage(std::string name, bool * defined, bool * declared) {
	Symbol * symbol = getSymbolByName(name, false);
	// Iterate throught all the overloaded symbols to see any of them are packages????
	*defined = false;
	*declared = false;
}
       
bool SymbolTable::canAssign(Symbol * lhsType, Symbol * rhsType, bool usePkg) {
	Symbol::SymbolType rhsSymType = rhsType->getSymbolType();

	if (rhsSymType == Symbol::UNI_INT || rhsSymType == Symbol::UNI_FLOAT ||
	    rhsSymType == Symbol::UNI_STRING || rhsSymType == Symbol::UNI_BOOLEAN) {
		return typeStructualEquivalent(lhsType, rhsType);
	}

	return typeNameEquivalent(lhsType, rhsType);	
}

bool SymbolTable::typeNameEquivalent(Symbol * type1, Symbol * type2) {
	return type1 == type2;
}

bool SymbolTable::typeStructualEquivalent(Symbol * type1, Symbol * type2) {

	if (typeNameEquivalent(type1, type2))
		return true;

	TypeDescriptor * type1_d = type1->getTypeDescriptor();
	TypeDescriptor * type2_d = type2->getTypeDescriptor();

	NewTypeDescriptor * type1_newd = dynamic_cast<NewTypeDescriptor *>(type1_d);
	NewTypeDescriptor * type2_newd = dynamic_cast<NewTypeDescriptor *>(type2_d);

	if (type1_newd) {
		return typeStructualEquivalent(type1_newd->parentType, type2);
	}

	if (type2_newd) {
		return typeStructualEquivalent(type2_newd->parentType, type1);
	}
	
	return false;
}

vector<Symbol *> * SymbolTable::getPossibleReturnType(NameNode * name, bool usePkg) {

	return 0;
}
/******************************************************************************
 * Type Descriptors
 * 
 *****************************************************************************/
std::ostream& operator<<( std::ostream &out, const TypeDescriptor &t) {
    out<< " (";
    if(t.fPrivate) {
        out<< " is-private";
    }
    t.print(out);
    out<< ")";

    return out;
}


void TypeDescriptor::print(ostream& out) const {
}

TypeDescriptor::~TypeDescriptor() {}
NewTypeDescriptor::~NewTypeDescriptor() {}
SubTypeDescriptor::~SubTypeDescriptor() {}

void SubTypeDescriptor::print(ostream& out) const {
    out<<"subtype "<<baseTypeSym->getName();
    if(constraint_t == Range) {
        out << " range ";
        if(rngLower != 0 && rngUpper != 0) {
            out << " L: "<< "U: ";
        }
    } else if (constraint_t == Index) {
        out << " index ";
    }
}

void SubTypeDescriptor::setRange(ExpressionNode* lower, ExpressionNode* upper) {
    constraint_t = Range;
    rngLower = lower;
    rngUpper = upper;
}

ArrayDescriptor::~ArrayDescriptor() {}

void ArrayDescriptor::print(ostream& out) const {
    if(arrayType == Constrained) {
        out<<" constrained ";
        if(consIndexList != 0) {
            vector<RangeNode*>::const_iterator iter = consIndexList->begin();
            for( ; iter != consIndexList->end(); iter += 1) {
                out<<*iter<<" ";
            }
        }
    } else {
        out<<" unconstrained";
    }
    if(elemTypeSym) {
        out<<"of "<<elemTypeSym->getName();
    }
}

EnumDescriptor::~EnumDescriptor() {}
void EnumDescriptor::setValues(vector<string>* ids) {
    id_list = ids;
}

void EnumDescriptor::print(ostream& out) const {
    if(id_list != 0) {
        vector<string>::const_iterator iter = id_list->begin();
        out<<"[";
        for( ; iter != id_list->end(); iter += 1) {
            out<<*iter<<" ";
        }
        out<<"]";
    } else {
        out<<"id_list is null";
    }
}

RecordDescriptor::~RecordDescriptor() {}

void RecordDescriptor::print(ostream& out) const {
    out << " record ";
    if(components != 0) {
        vector<Symbol*>::const_iterator iter = components->begin();
        out << "[";
        for( ; iter != components->end(); iter += 1) {
            out << (*iter)->getName() << ",";
        }
        out << "]";
    }
    if(variants != 0) {
        vector<Variant*>::const_iterator iter = variants->begin();
        out << "{";
        for( ; iter != variants->end(); iter += 1) {
            //out << (*iter)->getName() << ",";
        }
        out << "}";

    }
}

AccessDescriptor::~AccessDescriptor() {}

void AccessDescriptor::print(ostream& out) const {
    out << " pointer to ";
    out << subtypeSym->getName();
}

ProcDescriptor::~ProcDescriptor() {}
FuncDescriptor::~FuncDescriptor() {}
StandardDescriptor::~StandardDescriptor() {}
