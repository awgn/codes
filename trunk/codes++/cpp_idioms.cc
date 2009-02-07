/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cctype> 
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>

#include <getopt.h>
#include <errno.h>

extern char *__progname;
const char usage[]=
      "%s [options]: class1 opt... -- class2 opt... -- classn opt...\n"
     "\n Class:\n"
      "   -c class            class  name\n"
      "   -s struct           struct name\n"
      "\n Inheritance:\n"                                            
      "   -p name -p name ... public    derivation\n"
      "   -r name -p name ... protected derivation\n"
      "   -v name -p name ... private   derivation\n"
      "\n Template:\n"
      "   -n elem -n elem ... template non-type\n"
      "   -t name ...         template parameter\n"
      "   -w name ...         template template parameter\n"
      "\n Idioms:\n"
      "   -A elem -A elem ... named-constructor idiom\n"
      "   -B elem -B elem ... named-parameter idiom\n"
      "   -C elem -C elem ... public overloaded non-virtuals call\n"
      "                       protect non-overloaded virtual idiom\n"
      "   -P                  proxy idiom\n"
      "   -L                  operator(): lvalue-rvalue idiom\n"
      "   -N                  non-derivable idiom\n"
      "   -O                  virtual-friend-function idiom (printOn)\n"            
      "   -3                  big-three guideline (non copyable)\n"
      "   -h                  print this help.\n"
      "\n Element: id,<type>,<default> examples: \n"
      "   'test' 'home,int' 'point,char *, NULL'\n\n";


// basic element
//

class Element {

    friend class Class;

    std::string __id;
    std::string __type;
    std::string __symbol;
    std::string __default;

    Element(const std::string &i, const std::string &t = "bool", const std::string &d = "true" ) :
        __id(i), __type(t), __symbol(t), __default(d) {

            std::string::iterator it = __symbol.begin();
            std::string::iterator it_end = __symbol.end();

            replace(it, it_end, ' ', '_');
            replace(it, it_end, '*', 'P');
            replace(it, it_end, '&', 'R');
            replace(it, it_end, ':', '$');

        }

    public:

    static Element opt(std::string option, const std::string &t, const std::string &d="") {
        std::string::size_type l = option.find(",");
        if (l != std::string::npos )
            option = option.substr(0,l); 

        return Element(option,t,d); 
    }

    static Element opt(std::string option) {

        std::string::size_type l = option.find(",");
        if ( l == std::string::npos )
            return Element(option);

        std::string type = option.substr(l+1);
        std::string::size_type r = type.find(",");
        if ( r == std::string::npos )
            return Element(option.substr(0,l), type );

        std::string def = type.substr(r+1);
        return Element(option.substr(0,l), type.substr(0,r), def);
    }

    const std::string &_id() const { return __id; }
    const std::string &_type() const { return __type; }
    const std::string &_symbol() const { return __symbol; }
    const std::string &_default() const { return __default; }
};


enum Section {
    _private_,
    _protected_,
    _public_
};


class Member {

    Member(const Member &);
    Member &operator=(const Member &);

    protected:
        std::string _type;
        std::string _id;

        Section     _locate;
        bool        _statica;
        bool        _const;

    public:
        Member(const std::string &t, const std::string &i) 
            : _type(t), _id(i), _locate(_private_), 
            _statica(false), _const(false) {}

        virtual ~Member() {}

        Member &statica()     { _statica = true; return *this; }
        Member &constante()   { _const = true; return *this; }

        Member &scope(const Section &s) { _locate = s; return *this; }
        Section where() const { return _locate; }

        virtual std::string dump() {
            std::string r;
            r.append("        ").append(_statica ? "static " : "");
            r.append(_const ? "const " : "" ).append(_type).append(" ").append(_id).append(";");  
            return r;
        }
};


class MemberFunction : public Member {

    bool _pure;
    bool _virtuale;
    bool _signatureOnly;

    std::string _args;
    std::string _body;
    public:

    MemberFunction& pure()          { _pure = true; _virtuale = true; return *this; }
    MemberFunction& virtuale()      { _virtuale = true; return *this; }
    MemberFunction& signatureOnly() { _signatureOnly = true; return *this; }

    MemberFunction(const std::string &t, const std::string &i, const std::string &a, const std::string &b = "{}") :
        Member(t,i), _pure(false), _virtuale(false), _signatureOnly(false), _args(a), _body(b) {}

    virtual std::string dump() {
        std::string r;
        r.append("        ");
        r.append( (_virtuale||_pure) ? "virtual " : (_statica ? "static " : ""));
        r.append(  _const ? "const " :"");
        r.append(Member::_type).append(Member::_type.empty() ? "" : " ");
        r.append(Member::_id).append("(").append(_args).append(")");
        r.append( _signatureOnly ? ";" : ( _pure ? "=0;" : std::string(" ") + _body));
        return r;
    }   

    using Member::where;
    using Member::scope;
    using Member::statica;
    using Member::constante;

};


// Base Class    ::::::::::::::::::::::::::::::::::::::::::::
//
struct BaseClass {

    std::list<Member *> scopeList;

    std::string __name;

    // idioms
    //

    BaseClass(const std::string &n="") : 
    scopeList(),
    __name(n) 
    {} 
    
    virtual ~BaseClass()
    {}

    void pushMember(Member *ptr) {
        scopeList.push_back(ptr);
    }

    std::string dumpScope(const Section &s) {
        std::string ret;
        std::list<Member *>::iterator it = scopeList.begin();        
        std::list<Member *>::iterator it_end = scopeList.end();        
        for(; it != it_end; ++it) {
            if ((*it)->where() != s )
                continue;

            ret.append((*it)->dump()).append("\n");
        }
        return ret;
    }

    bool hasSection(const Section &s) {
        std::list<Member *>::iterator it = scopeList.begin();      
        std::list<Member *>::iterator it_end = scopeList.end();          
        for (; it != it_end; ++it) {
            if ( (*it)->where() == s)
                return true;        
        }
        return false;
    }
};


// extra policy  ::::::::::::::::::::::::::::::::::::::::::::
//
struct ExtraPolicy: public virtual BaseClass {
    std::set<std::string> blocks;

    ExtraPolicy() :
    blocks()
    {}

    virtual ~ExtraPolicy() 
    {}

    void push(const std::string &s) {
        blocks.insert(s);
    }

    const std::string action() {
        std::string r;
        std::set<std::string>::iterator it = blocks.begin();
        std::set<std::string>::iterator it_end = blocks.end();

        for (; it != it_end; ++it) {
            r.append(*it).append("\n");
        }        
        return r;
    } 

};


// include policy  ::::::::::::::::::::::::::::::::::::::::::::
//
struct IncludePolicy: public virtual BaseClass {
    std::set<std::string> includes;

    IncludePolicy() :
    includes()
    {}

    virtual ~IncludePolicy()
    {}

    void push(const std::string &s) {
        includes.insert(s);
    }

    const std::string action() {
        std::string r;
        std::set<std::string>::iterator it = includes.begin();
        std::set<std::string>::iterator it_end = includes.end();

        for (; it != it_end; ++it) {
            r.append("#include <").append(*it).append(">\n");
        }        
        return r;
    } 

};


// template policy ::::::::::::::::::::::::::::::::::::::::::::
//
struct TemplatePolicy : public virtual BaseClass {

    std::list<Element>  list;        

    TemplatePolicy() :
    list()
    {}

    virtual ~TemplatePolicy()
    {}

    void push(const Element &e) {
        list.push_back(e);                
    }

    const std::string action()
    {
        std::string r;

        std::list<Element>::iterator it =   list.begin();
        std::list<Element>::iterator it_end = list.end();

        if (it == it_end)
            return r;

        r.append("template < ");

        for(;;) {
            r.append(it->_type()).append(" ").append(it->_id());
            if (!it->_default().empty())
                r.append(" = ").append(it->_default());

            if (++it == it_end)
                break;

            r.append(" , ");
        }
        r.append(" >\n");
        return r;
    }

};


// init policy :::::::::::::::::::::::::::::::::::::::::::: 
//
struct InitPolicy : public virtual BaseClass {

    std::list<Element>  list;  

    InitPolicy() :
    list()
    {}

    virtual ~InitPolicy()
    {}

    void push(const Element &e) {
        list.push_back(e);                
    }

    const std::string action()
    {
        std::string r;

        std::list<Element>::iterator it = list.begin();
        std::list<Element>::iterator it_end = list.end();

        if ( it == it_end )
            return r;

        r.append(" : ");
        for(;;) {

            r.append(it->_id()).append("(").append(it->_default()).append(")");
            ++it;

            if (it == it_end)
                break;
            r.append(" , ");
        }
        return r;
    }

};          


// derivation policy :::::::::::::::::::::::::::::::::::::::::::: 
//

struct DerivationPolicy : public virtual BaseClass {

    std::list<Element>  list;        

    DerivationPolicy() :
    list()
    {}

    virtual ~DerivationPolicy()
    {}

    void push(const Element &e) {
        list.push_back(e);                
    }

    const std::string action()
    {
        std::string r;

        std::list<Element>::iterator it = list.begin();
        std::list<Element>::iterator it_end = list.end();

        if ( it == it_end )
            return r;

        r.append(" : ");

        for(;;) {
            r.append(it->_type()).append(" ").append(it->_id());
            ++it;

            if (it == it_end)
                break;
            r.append(" , ");
        }
        return r;
    }

};


// NamedConstructorIdiom policy :::::::::::::::::::::::::::::::::::::::::::: 
//

struct NamedConstructorIdiom : public virtual BaseClass {

    void push(const Element &e) {

        // public:
        //

        MemberFunction *p = new MemberFunction(
                std::string(__name),
                e._id(),
                "", 
                std::string("{ return ").append(__name).append("(); }"));

        p->statica().scope(_public_);
        pushMember(p); 
    }

};


// NamedParameterIdiom policy :::::::::::::::::::::::::::::::::::::::::::: 
//

struct NamedParameterIdiom : public virtual BaseClass {

    void push(const Element &e) {

        // private:
        //

        std::string r(e._id());
        r.append("_");
        Member * p = new Member(e._type(), r ); 
        p->scope(_private_);
        pushMember(p);          // bool id_ ; 

        // public:
        //

        MemberFunction * q = new MemberFunction(
                                std::string(__name)+"& ", 
                                e._id(), 
                                "", 
                                "{ " + e._id() + "_= " + e._default() + "; return *this; }");
        q->scope(_public_);
        pushMember(q);
                
    }

};


// PonvPnovIdiom policy :::::::::::::::::::::::::::::::::::::::::::: 
//

struct PonvPnovIdiom : public virtual BaseClass {

    void push(const Element &e) {

        // protected:
        //

        std::string r(e._id());
        r.append("_");

        MemberFunction * p = new MemberFunction(
                "void",
                std::string(e._id()).append("_").append(e._symbol()), 
                std::string(e._type()).append(" x")); 

        p->scope(_protected_); 
        p->pure();
        pushMember(p);          

        // public:
        //

        MemberFunction * q = new MemberFunction(
                                "void",
                                e._id(), 
                                std::string(e._type())+" x", 
                                "{ " + e._id() + "_" + e._symbol() + "(x); }");
        q->scope(_public_);
        pushMember(q);
 

    }
 
};

// ::::::::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::::: CLASS ::::::::::::::::::::::::: 
// ::::::::::::::::::::::::::::::::::::::::::::::::::

class Class : 
    public IncludePolicy, public ExtraPolicy, public InitPolicy, 
    public TemplatePolicy, public DerivationPolicy, public NamedConstructorIdiom, 
    public NamedParameterIdiom, public PonvPnovIdiom {

    private:

        std::ostream *out;
        bool struc;

        static std::string __NAME;
        static std::list<Class *> theClasses; 

        void guardName(const std::string &n) {
            __NAME.append("__").append(n);
            std::transform(__NAME.begin(), __NAME.end(), __NAME.begin(), static_cast<int(*)(int)>(toupper));
        }

        Class(Class &);
        Class &operator=(const Class &);

    public:

        explicit Class (const std::string &n, bool s = false ) 
        : 
        BaseClass(n),
        out(NULL),
        struc(s) 
        {} 

        static std::string openGuard() {
            std::string ret;
            __NAME.append("__HH"); 
            ret.append("#ifndef ").append(__NAME).append("\n#define ").append(__NAME).append("\n");
            return ret;
        }

        static std::string closeGuard() {
            std::string ret;
            ret.append("#endif /* ").append(__NAME).append(" */\n"); 
            return ret;
        }

        static void pushClass(Class *c, bool guard = true) {
              theClasses.push_back(c);
              if(guard) {
                 __NAME.append("__").append(c->__name);
                 std::transform(__NAME.begin(), __NAME.end(), __NAME.begin(), static_cast<int(*)(int)> (toupper)); 
              }
        }

        static void dumpClasses() {

            std::list<Class *>::iterator it = theClasses.begin();
            std::list<Class *>::iterator it_end = theClasses.end();
        
            if (theClasses.empty())
                return;
        
            std::cout << openGuard();
        
            for(; it != it_end ; ++it)
                std::cout << *(*it);
        
            std::cout << closeGuard();
        }


        /* virtual */ ~Class () { }

        friend std::ostream& operator<< (std::ostream& o, Class & b);  

};


std::string Class::__NAME;
std::list<Class *> Class::theClasses;


//////////////////////////////////////////////////////////
//
//  Dump Class
//
//////////////////////////////////////////////////////////

std::ostream & operator<<(std::ostream& out, Class & ent) {

    ent.out = &out;

    // includes
    //
    out << ent.IncludePolicy::action();

    // extra policy
    //
    out << ent.ExtraPolicy::action();

    // template
    //
    out << ent.TemplatePolicy::action();

    // class//struct
    //
    out << ( ent.struc ? "struct" : "class" ) << " " << ent.__name;

    // derivation policy...
    //
    out << ent.DerivationPolicy::action();

    out << " {\n";

    // private: 
    //
    if ( ent.hasSection(_private_) ) {
        out << "\n    private:\n";
        out << ent.dumpScope(_private_);
    }

    // protected: 
    //
    if ( ent.hasSection(_protected_) ) {
        out << "\n    protected:\n";
        out << ent.dumpScope(_protected_);

    }

    // public: 
    //
    if ( ent.hasSection(_public_) ) {
        out << "\n    public:\n";
        out << ent.dumpScope(_public_);
    }

    out << "\n};\n";   

    return out;
}



Class *notnull(Class *e) {
    if ( e == NULL )
         throw std::runtime_error("unknown class/structure name: -c/-s option must come first!");
    return e;
};


int 
main(int argc, char *argv[]) {

    int i;

    for (;argc>1;  optind = 0  ) {

        Class *x = NULL;
        MemberFunction *ctor = NULL;
        MemberFunction *dtor = NULL;

        while ((i = getopt(argc, argv, "+p:r:v:n:t:w:c:s:A:B:C:LP3ONh")) != EOF) {

            switch(i) {
            case 'c': {
                 x = new Class(optarg);
                 ctor = new MemberFunction("",optarg,"");
                 ctor->scope(_public_);
                 x->pushMember(ctor);

                 dtor = new MemberFunction("",std::string("~").append(optarg),"");
                 dtor->scope(_public_);
                 x->pushMember(dtor);
                 break;
             }
             case 's': {
                   x = new Class(optarg, true /* is structure? */ );
                   ctor = new MemberFunction("",optarg,"");
                   ctor->scope(_public_);
                   x->pushMember(ctor);

                   dtor = new MemberFunction("",std::string("~").append(optarg),"");
                   dtor->scope(_public_);
                   x->pushMember(dtor);
                   break;
            }

            case 'n': notnull(x)->TemplatePolicy::push(Element::opt(optarg)); 
                      break;     
            case 't': notnull(x)->TemplatePolicy::push(Element::opt(optarg,"typename")); 
                      break;     
            case 'w': notnull(x)->TemplatePolicy::push(Element::opt(optarg,"template <typename> class")); 
                      break; 

            case 'p': notnull(x)->DerivationPolicy::push(Element::opt(optarg,"public"));    
                      break;
            case 'r': notnull(x)->DerivationPolicy::push(Element::opt(optarg,"protected")); 
                      break;
            case 'v': notnull(x)->DerivationPolicy::push(Element::opt(optarg,"private"));   
                      break;

            // Idioms
            //

            case 'A': { 
                 notnull(x)->NamedConstructorIdiom::push(Element::opt(optarg)); 
                 ctor->scope(_private_);
                 break;
            }

            case 'B': {
                  notnull(x)->NamedParameterIdiom::push(Element::opt(optarg));   
                  break;
            }

            case 'C': {
                 notnull(x)->PonvPnovIdiom::push(Element::opt(optarg)); 
                 break;
            }

            case 'N': { 

                 notnull(x)->DerivationPolicy::push(Element::opt(std::string("ForceLeaf<")+notnull(x)->__name+">","private virtual"));

                 Class *y = new Class("Type2type", true);
                 y->TemplatePolicy::push(Element::opt("T","class"));
                 Member *typed = new Member("typedef T","type");
                 typed->scope(_public_);
                 y->pushMember(typed);
                 Class::pushClass(y, false);

                 y = new Class("ForceLeaf");
                 y->TemplatePolicy::push(Element::opt("T", "typename"));
                 Member *friends = new Member("friend class", "Type2type<T>::type");
                 friends->scope(_private_);
                 y->pushMember(friends);
                 MemberFunction *ctor = new MemberFunction("","ForceLeaf","");
                 ctor->scope(_private_);
                 y->pushMember(ctor);          
                 Class::pushClass(y, false /* do not append to guardian */);

                 break; 
            }

            case 'P': {

                Class  *y = new Class("Proxy");
                Member *m = new Member("T &", "value_");          
                y->TemplatePolicy::push(Element::opt("T","typename"));
                m->scope(_private_);
                y->pushMember(m);

                // copy constructor
                MemberFunction *f = new MemberFunction("", "Proxy","const T &r",": value_(r) {}");
                f->scope(_public_);
                y->pushMember(f);

                // operator=

                f = new MemberFunction("Proxy &","operator=","const T &r", "{ value_ = r; return *this; }");
                f->scope(_public_);
                y->pushMember(f);

                // operator<<

                f = new MemberFunction("std::ostream&", 
                                       "operator<<", 
                                       "std::ostream& o, const Proxy& p", 
                                       "{ o << p.value_; return o; }");
                f->scope(_public_);
                y->pushMember(f);

                Class::pushClass(y, false /* do not append to guardian */);
                break;
            }     

            case 'L': {

                Class  *y = new Class("ElementRef");
                y->TemplatePolicy::push(Element::opt("T","typename"));

                MemberFunction *f = new MemberFunction("operator", "T","");
                f->scope(_public_);
                y->pushMember(f);

                f = new MemberFunction("ElementRef &","operator=","const T &r");
                f->scope(_public_);
                y->pushMember(f);

                Class::pushClass(y, false /* do not append to guardian */);
                break;
            }          

            case 'O': {  // printOn idiom... 

                notnull(x)->IncludePolicy::push("iostream"); 
                MemberFunction *f = new MemberFunction(
                                        "void", "printOn",
                                        "std::ostream & o", "{ o << __PRETTY_FUNCTION__ << std::endl; }");
                f->virtuale();
                f->scope(_protected_);
                notnull(x)->pushMember(f);

                f = new MemberFunction( "friend std::ostream &", 
                                        "operator<<",
                                        std::string("std::ostream& o, const ") + notnull(x)->__name + "& b", 
                                        "{ b.printOn(o); return o; }" );
                f->scope(_public_);
                notnull(x)->pushMember(f);

                break;
            }

            case '3': { 

                 MemberFunction *copyctor = new MemberFunction( "",
                                                                notnull(x)->__name,
                                                                std::string("const ")+ notnull(x)->__name + " &");
                 copyctor->signatureOnly().scope(_private_);
                 notnull(x)->pushMember(copyctor);

                 MemberFunction *opequal = new MemberFunction(
                                                notnull(x)->__name+"&", 
                                                "operator=", 
                                                std::string("const ") + notnull(x)->__name + "&" );

                 opequal->signatureOnly().scope(_private_);
                 notnull(x)->pushMember(opequal);
                 break;
            }

            case 'h': fprintf(stderr,usage,__progname); exit(0);
            case '?': 
            default : 
                      throw std::runtime_error("unknown option");

            }

        }

        argc -= (optind-1);
        argv += (optind-1);

        Class::pushClass(x);

    }
    
    Class::dumpClasses();

    return 0;
}
