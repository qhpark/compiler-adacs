#ifndef _SYMTAB_HPP_
#define _SYMTAB_HPP_

#include <string>
#include <vector>
#include <set>

#include "parsetree.hpp"
#include "debug.hpp"

// QP: XXX FIXME!
// global symbol table - temporary

class SymbolTable;
extern SymbolTable *symTab_G;

using namespace std;
class TypeDescriptor;
class ExpressionNode;
class RangeNode;

class Symbol {
    friend std::ostream& operator<<( std::ostream &out, const Symbol &S);
    public:
        enum SymbolType { 
                        SYM_UNKNOWN, 
                        //PROC, FUNC,
                        VAR, CONST, 
                        //ARRAY , ENUM, RECORD, 
                        BLOCK, LITERAL, 
                        TYPE,
                        //SUBTYPE, 
                        PRAGMA, 
                        //ACCESS,

						// Universal String.
						UNI_INT, UNI_FLOAT, UNI_STRING, UNI_BOOLEAN
                        };

        enum DataType { DATA_UNKNOWN, BOOL, INT, FLOAT };
		Symbol();
		Symbol(SymbolType);
		~Symbol();

		void setName(std::string);
		std::string getName();

		void setSymbolType(SymbolType);
		SymbolType getSymbolType();

		void setDataType(DataType);
		DataType getDataType();

        // Type System Information for this symbol (if symbol is a type)
        void setTypeDescriptor(TypeDescriptor *);
        TypeDescriptor *getTypeDescriptor();
        
        // Pointer to Symbol that represents this symbol's type
        void setTypeDescriptorSym(Symbol *);
        Symbol *getTypeDescriptorSym();
       
		// Def.
		void setDef(unsigned int);
		unsigned int getDef();

		// Use.
		void addUse(unsigned int);
		std::vector<unsigned int> * getUses();

        // Symbol links
        Symbol *nextSameScope;      // Link symbols at same scope
        Symbol *nextSameName;       // Link to first symbol at higher scope with same name
        Symbol *nextOverload;       // Link to overloaded symbols at same scope

	private:
        
		SymbolType symType;
		DataType dataType;
		std::string name;
		unsigned int def;
		std::vector<unsigned int> uses;
        TypeDescriptor *typeDesc;
        Symbol* symTypeDesc;
};

class NameNode;
class SymbolTable {
    friend std::ostream& operator<<( std::ostream &out, const SymbolTable &T);
    public:
        typedef struct name_list_node {
            name_list_node* next;
            Symbol* symbol;
        };

    
		SymbolTable();
		~SymbolTable();

		// Get a pointer to a symbol by it name.
        Symbol* getSymbolByName(std::string name, bool usePkgs);
    
        // Add given symbol to the current scope
        void AddSymbol(Symbol *sym);

        // Create a new inner scope and make it current
        void OpenScope();
        
        // Close the current scope and make the parent scope current
        void CloseScope();
        
        // Add public definitions from given package to current scope
        void UsePackage(std::string pkg_name);

        // Remove public definitions from given package from current scope
        void RemovePackage(std::string pkg_name);
        
        string getNextAnonName(string name);


		// We need some helpers.

		// defined - return true if we can't redefined it again.
		// declared - declared once. We must now import all the symbols in the
		//            package.
		// Return the symbol of the package.
		Symbol * definePackage(std::string name, bool * defined, bool * declared);        

		// Check if we can assignment a := b given  the type of a and b.
		bool canAssign(Symbol * lhsType, Symbol * rhsType, bool usePkg);

		bool typeNameEquivalent(Symbol * type1, Symbol * type2);
		bool typeStructualEquivalent(Symbol * type1, Symbol * type2);

		vector<Symbol *> * getPossibleReturnType(NameNode * name, bool usePkg);
	private:

        void buildInitialEnvironment();

        name_list_node* getNameNode(std::string name, bool usePkgs);

        bool AddOverload(Symbol*, Symbol*);

		// List of all symbols.
        name_list_node* name_list;
        // Vector for maintaining scopes
        std::vector<Symbol*> rngScopeDisplay;
        
        unsigned int depth;
        unsigned int anon_ind;

};

//=============================================================================
// Type System
//=============================================================================

// Base Type Descriptor
class TypeDescriptor {

    friend std::ostream& operator<<( std::ostream &out, const TypeDescriptor &S);

    public:
        enum Type { 
            UNKNOWN, PROC, FUNC, 
            ARRAY , ENUM, RECORD, STANDARD,
            SUBTYPE, ACCESS, TYPE
        };

        TypeDescriptor(Type type_) : type(UNKNOWN) {}
        virtual ~TypeDescriptor();
        virtual Type getType() const { return type; }
        virtual void print(ostream& out) const;

        // Is this type private
        bool isPrivate() const { return fPrivate; }
        void setPrivate(bool fPriv) { fPrivate = fPriv; }
        
    private:
        Type type;
        bool fPrivate;
};

// Type Descriptor for new types
class NewTypeDescriptor : public TypeDescriptor {

    public:
        NewTypeDescriptor() : TypeDescriptor(TYPE), parentType(0) {}
        virtual ~NewTypeDescriptor();
		Symbol * parentType;
};

// Type Descriptor for SubTypes
class SubTypeDescriptor : public TypeDescriptor {

    public:
        enum ConstraintType {
            None,
            Range,
            Index
        };

        SubTypeDescriptor(Symbol* parentSym) : 
            rngIndex(0), constraint_t(None),
            rngLower(0), rngUpper(0),
            baseTypeSym(parentSym), 
            TypeDescriptor(SUBTYPE) {}
        virtual ~SubTypeDescriptor();
        
        void print(ostream& out) const; 

        ConstraintType getConstraintType() const { return constraint_t; }
        
        Symbol* getBaseType() const { return baseTypeSym; }
       
        // Range Constraint
        void setRange(ExpressionNode* lower, ExpressionNode* upper);
        ExpressionNode* getLower() const { return rngLower; }
        ExpressionNode* getUpper() const { return rngUpper; } 

        // Index Constraint
        void setRange(vector<RangeNode*> *rngs) { constraint_t = Index; rngIndex = rngs; }
        vector<RangeNode*> * getRange() const { return rngIndex; }
    private:
        Symbol* baseTypeSym;
        ConstraintType constraint_t;
        // Range Constraint
        ExpressionNode* rngLower;
        ExpressionNode* rngUpper;
        // Index Constraint
        vector<RangeNode*> *rngIndex;

};

// Type Descriptor for Enumeration types
class EnumDescriptor : public TypeDescriptor {

    public:
        EnumDescriptor() : id_list(0), TypeDescriptor(ENUM) {}
        virtual ~EnumDescriptor();
        void setValues(vector<string>* ids);
        vector<string>* getValues() const { return id_list; }
        void print(ostream& out) const;
    private:
        vector<string>* id_list;
};

// Type Descriptor for Access variables
class AccessDescriptor : public TypeDescriptor {

    public:
        AccessDescriptor(Symbol* sub_t) : 
            subtypeSym(sub_t), TypeDescriptor(ACCESS) 
        {}
        virtual ~AccessDescriptor();
        void print(ostream& out) const;
        Symbol* getSubtype() const { return subtypeSym; }
    private:
        Symbol* subtypeSym;
};

// Type Descriptor for Records
class Variant {
    public:
        typedef struct when_list_node {
            ExpressionNode* whenNode;
            when_list_node* next;
        };
        Variant() : tag(0), when_list(0) {} 
    private:
        Symbol* tagTypeSym;
        string tag;
        when_list_node* when_list;
        

};


class RecordDescriptor : public TypeDescriptor {

    public:
        RecordDescriptor() :
           components(0), variants(0), TypeDescriptor(RECORD) {}
        virtual ~RecordDescriptor();
        void print(ostream& out) const;

        // Components
        vector<Symbol*>* getComponents() const { return components; }
        void setComponents(vector<Symbol*>* comps) { components = comps; }

        // Variants
        vector<Variant*>* getVariants() const { return variants; }
        void setVariants(vector<Variant*>* vars) { variants = vars; }
        
    private:
        // components of a record, names are not added to symbol table
        vector<Symbol*>* components;
        // variant part of a record
        vector<Variant*>* variants;
};


// Type Descriptor for the Standard types
class StandardDescriptor : public TypeDescriptor {

    public:
        StandardDescriptor() : TypeDescriptor(STANDARD) {}
        virtual ~StandardDescriptor();
        enum StandardType {
            Boolean,
            Integer,
            Float,
            String
        };
    
        
};

// Type Descriptor for Arrays
class ArrayDescriptor : public TypeDescriptor {

    public:
        enum ArrayType {
            Constrained, 
            Unconstrained
        };

        ArrayDescriptor(ArrayType at) 
            : arrayType(at), TypeDescriptor(ARRAY), 
            elemTypeSym(0), consIndexList(0), unconsIndexList(0)
        { }
        virtual ~ArrayDescriptor();
        void print(ostream& out) const ;
        ArrayType getArrayType() const { return arrayType; }
        
        // Array element type
        Symbol* getElementType() const { return elemTypeSym; }
        void setElementType(Symbol* sym) { elemTypeSym = sym; } 

        // Constrained range list
        void setRange(vector<RangeNode*>* ranges) { consIndexList = ranges; }
        vector<RangeNode*>* getConsRange() const { return consIndexList; }
         
        // Unconstrained range list
        void setRange(vector<string>* ranges) { unconsIndexList = ranges; }
        vector<string>* getUnconsRange() const { return unconsIndexList; }
        
    private:
        // symbol representing element type (could be anonymous)
        Symbol *elemTypeSym;
        // type of array: constrained or unconstrained
        ArrayType arrayType;
        // vector of discrete ranges for constrained arrays
        vector<RangeNode*>* consIndexList;
        // unconstrained arrays
        vector<string>* unconsIndexList;
};


// The declaration of the formal parameter.
class FormalParameter{
	public:
        FormalParameter() : name(0), type(0), in(0), out(0) {}
        ~FormalParameter() {}
		Symbol * name;
		Symbol * type;
		bool in;
		bool out;
};

// Type Descriptor for SubPrograms
class ProcDescriptor : public TypeDescriptor {

    public:
		vector<FormalParameter *> formalParams;

        ProcDescriptor() : TypeDescriptor(PROC) {}
        virtual ~ProcDescriptor();
		unsigned int getNumParams() { return formalParams.size(); }
};

class FuncDescriptor : public TypeDescriptor {

    public:
		vector<FormalParameter *> formalParams;
		Symbol * returnType;

        FuncDescriptor() : TypeDescriptor(FUNC), returnType(0) {}    
        virtual ~FuncDescriptor();
		unsigned int getNumParams() { return formalParams.size(); }
};


//* This is for printing out stats.
class Extree;
struct SymbolUse {
	Symbol * symbol;
	unsigned int line;
	Extree * treenode;
};

// temporary NamePacker

//typedef set<Symbol *> symbolset_t;
#define indent(depth) \
	for (int ii = 0; ii < depth; ii++) { \
		cout << " "; \
	} 
struct NamePacker {
	enum AccessType { HEAD, DOT, TICK, EXPR };
	string id;
	AccessType atype;	
	vector<set<Symbol *> *> *exprs;
	//void quickDump(unsigned int depth) const ;
#if 1
	void quickDump(unsigned int depth) const {
		indent(depth); 
		cout << "[Id: "<< id << ", AccessType: " ;
		switch(atype) {
			case HEAD:
				cout << "HEAD]" << endl;;
				break;
			case DOT:
				cout << "DOT]" << endl;
				break;
			case TICK:
				cout << "TICK]" << endl;
				break;
			case EXPR:
				ASSERT(exprs != NULL, "bug...");
				cout << "EXPR <Possible Types>]\n";
				for(vector<set<Symbol *> *>::const_iterator i = exprs->begin();
						i != exprs->end(); i++) {
						indent(depth + 1); 
					for(set<Symbol *>::const_iterator  expr = (*i)->begin();
							expr != (*i)->end(); expr++)  {
						cout << (**expr) <<"  " ;
						//cout << *((*expr)->getTypeDescriptor()) <<" : " ;
					}
					cout << endl;
				}
				break;
		}
	}
#endif
	
};
#endif
