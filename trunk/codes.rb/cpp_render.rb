#!/usr/bin/ruby1.9 -w 
#
# $Id$ 
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <bonelli@antifork.org> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
# ----------------------------------------------------------------------------
#

require 'stringio'
require 'deep_clone'
require 'boolean_trait'

#
###################################### LazyReader base class

class LazyReader

    def self.lazy_reader(*met, arg)
        met.each do |m|
        if ( m.class != Symbol )
            next
        end
        code = %Q{
            def #{m} 
                meta_reader(__method__,#{arg})
            end
        }
        class_eval(code) 
        end
    end    
    
    private

    def meta_reader(tag,arg)    
        code = 
        %Q{
            if @#{tag}.class == Proc
                raise \"Proc given for '#{tag}' while class name is unspecified\" if('#{arg}'.empty?) 
                @#{tag}.call('#{arg}')
            else
                @#{tag}
            end
        }
        eval(code)
    end

end


#
###################################### Indenter 

class Indenter
    attr_accessor :level
    attr_reader :out

    def initialize(out, deep=4)
        @out   = out
        @deep  = deep 
        @level = 0
    end 

    def method_missing(method, *args)
        raise "'#{method.to_s}' method non supported" unless [:write, :putc, :puts, :print, :printf ].include?(method)
        out = StringIO.new
        out.send(method, *args)
        out.string.each_line { |line| @out.puts "#{" "*(@level*@deep)}#{line}" } 
    end    
end

#
###################################### typedefs

class Typedef
    def initialize(old,new)
        @old = old
        @new = new
    end

    def to_s
        %Q{typedef #{@old} #{@new};
}
    end
end

def typedef(type,new_type)
    Typedef.new(type,new_type)
end

#
###################################### comment 

class Comment
    def initialize(text)
        @comment = [] 
        text.each_line { |l| @comment << l.strip }
        @comment << ""
    end

    def to_s
        "\n#{ @comment.collect { |l| "// #{l}" }.join("\n") }\n"
    end
end

def comment(text)
    Comment.new(text)
end

#
###################################### CV Qualifier and Attributes

module QualifiersAndAttributes

    include BooleanTrait 

    boolean_trait :const, :volatile, :static, :virtual

    def initialize
        @const    = false
        @volatile = false

        @static   = false
        @virtual  = false
        @pure     = false
    end

    def pure
        @pure = true 
        @virtual = true
    end

    def non_pure
        @pure = false
    end

    def is_pure?
        @pure == true
    end
end

#
###################################### Class Member

class Member
    include QualifiersAndAttributes

    attr_reader :type, :id, :value

    def initialize(type, id, value=nil)
       super() 

       @type  = type
       @id    = id
       @value = value
    end

    def to_s
        %Q{\
#{(@static ?   "static "   : "")}#{(@const  ?   "const "    : "")}#{(@volatile ? "volatile " : "")}\
#{@type} #{@id}#{@value ? " = #{@value}" : ""};}
    end
end

#
###################################### Class MemberFunctionBase

class MemberFunctionBase < LazyReader

    include QualifiersAndAttributes

    attr_reader :class_name
    lazy_reader :type, :id, :args, :definition, :class_name

    def initialize(type,id,args,definition)

       super() # initialize Qualifiers.. Module
 
       @type        = type
       @id          = id
       @args        = args
       @definition  = definition 

       @class_name = nil

    end

    def method_of(name)
        @class_name = name
    end

end

#
###################################### Class MemberFunction

class MemberFunction < MemberFunctionBase

    def initialize(type, id, args, definition = nil)
        super
        @body = []
        if (definition)
            definition.each_line { |l| @body << l.strip }
        end
    end

    def to_s
        raise "'#{id}' cannot be declared both virtual and static" if (@static && @virtual)
        raise "'#{id}' pure-specifier on function-definition" if (@pure && @definition) 

        %Q{\
#{@virtual ? "virtual " : (@static ? "static " : "" ) }#{type}
#{id}(#{args})#{@const ? " const" : ""}#{@volatile ? " volatile" : ""}#{!definition ? (@pure ? "=0;" : ";" ) : 
@body.empty? ? "\n{}" : %Q{
{
#{@body.collect{ |l| "    #{l}" }.join("\n")}
}} } 
}
    end
end

# 
# factory: costructors, destructor, operator=, methods...

def cpp_method(type,id,args,definition=nil)
    MemberFunction.new(type,id,args,definition)
end

def ctor(definition = nil)
    MemberFunction.new("", lambda { |c| c }, "", definition)
end

def dtor(definition = nil)
    MemberFunction.new("", lambda { |c| "~#{c}" }, "", definition)
end
    
def copy_ctor(definition = nil)
    MemberFunction.new("", lambda { |c| c }, lambda { |c| "const #{c} &"}, definition)
end

def operator_eq(definition = nil)
    MemberFunction.new(lambda { |c| "#{c} &"}, 'operator=', lambda { |c| "const #{c} &"}, definition)
end

#
###################################### Template Module

module T 

    class Param 
        attr_reader :id

        def initialize(t=nil ,i=nil, v=nil)
            @type, @id, @value = t, i, v
        end

        def to_s
            "#{@type ? @type : "typename" }#{@id ? " #{@id}" : ""}#{@value ? " = #{@value}" : ""}"
        end
    end

    #
    # helper factory 

    def T.typename(i=nil, v=nil)
        Param.new("typename", i, v)
    end

    def T.nontype(t='bool', i=nil, v=nil)
        Param.new(t, i, v)
    end

    def T.template(t=nil, i=nil, v=nil)
        Param.new("#{t} class", i, v)
    end

    #
    # template class

    class Template

        def initialize
            @template = []
            @spec = {} 
        end

        def <<(e)
            raise "template parameter '#{e.id}' already in use" if ( @template.any? {|x| e.id == x.id } )
            @template << e
            self
        end

        def specialize(id,value)
            raise "template paramenter '#{id}' not found" unless (@template.find { |o| o.id == id } )
            @spec[id] = value
        end

        def template
            "template <#{ @template.join(", ") }>"
        end

        def to_s
            "template <#{ @template.select { |e| !@spec.has_key?(e.id)  } .join(", ") }>"
        end

        def spec 
            "<#{ @template.collect { |e| @spec.has_key?(e.id) ? @spec[e.id] : e.id  } .join(", ") }>"
        end

        def specialized?
            !@spec.empty?
        end
    end
end

#
###################################### CppClass...

class CppClass

    attr_reader :name, :members

    def initialize(name)
        @name = name
        @members = { :public => [], :protected => [], :private =>[] }
    end
    
    def <<(members)
        members.each_pair do |k,v| 
            if(v.class == Array) 
                v.each { |e| @members[k] << qualify(e) }
            else
                @members[k] << qualify(v)
            end
        end 
    end

    def qualify(e)
        if (e.respond_to?(:method_of))
            e.method_of(@name)
        end
        e
    end

    def template
        @template ||= T::Template.new
    end

    def to_s()
        %Q{\
#{ (@template ||= nil) ? "#{@template}\n" : "" }class #{@name}#{ @template ? (@template.specialized? ? @template.spec : "") : "" }
{
#{  
    str = StringIO.new 
    out = Indenter.new(str)
    out.level = 1 
    @members.each_pair { |k,v| out.puts "#{k}:\n\n#{v.collect { |l| "#{l}\n" }.join }" unless v.empty? }
    out.out.string
}};}
    end

end

#
# factory:

def cpp_class(name)
    CppClass.new(name)
end

#
###################################### CppModules

class CppModule 

    def initialize(name, out = STDOUT)
        @namespaces = []
        @elements   = []
        @headers    = []
        @guard = name.upcase
        @out = out
    end

    def include(*headers)
        headers.each { |h| @headers << h }
    end

    def namespace(*ns)
        ns.each { |n| @namespaces << n }
    end

    def <<(e)
        #raise "<< CppClass argumet expected" unless klass.kind_of?(CppClass)
        @elements << e.deep_clone 
    end

    def prolog
        @out.puts "#ifndef _" + @guard + "_H_" 
        @out.puts "#define _" + @guard + "_H_"
        @out.puts 
    end

    def body
        out = Indenter.new(@out)
        @headers.each { |h| out.puts "#include <#{h}>" }
        puts if @headers.size > 0

        @namespaces.each { |n| print "namespace #{n} { " }
        (puts; puts) if @namespaces.size > 0

            out.level += 1 if @namespaces.size > 0 
            @elements.each { |c| out.puts c }
        
        @namespaces.reverse.each { |n| print "} // #{n}\n" }

    end

    def epilog
        @out.puts
        @out.puts "#endif /* _" + @guard + "_H_ */"
    end

    alias close epilog

    def self.open(name, out = STDOUT)
        m = CppModule.new(name,out)
        if block_given?
            m.prolog
            yield m
            m.body
            m.epilog
        end
    end

end

def cpp_module(x, out = STDOUT, &b)
    CppModule.open(x,out,&b)
end 

################################################################

if __FILE__ == $0 

    begin

        cpp_module('test',STDOUT) do |m| 

            m.include 'stdio', 'cstdlib' 
            m.namespace 'ns'  

            c = cpp_class('test')
            x = cpp_method('int', 'hello', ('...'), "// hello\n// world").const.volatile.static

            y = x.dup
            y.virtual.non_static

            c << { 
                private: 
                    [ comment("this is a simple class"), 
                    x,
                    y, 
                    dtor("") 
                    ], 
                public: 
                    [ ctor(), 
                    copy_ctor(), 
                    operator_eq("return *this;") 
                    ] 
            }

            sub = cpp_class('subclass')
            sub << { public: x.dup }

            c << { public: sub }
            c << { public: typedef('int', 'value_type') }

            c.template << T.typename('Q') << T.typename('T')
            m << c

            c.template.specialize('Q','int')
            m << c

            m << comment("define a new type... \n")
            m << typedef('test<int,int>', 'my_type')

        end

    rescue => err
        puts "error: #{err}!"
    end
end
