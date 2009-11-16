#!/usr/bin/ruby1.9 -w
#
# thanks to http://d.hatena.ne.jp/pegacorn/20070412/1176309956

class Object
    def deep_clone
        return @deep_cloning_obj if (@deep_cloning||=nil)
        @deep_cloning_obj = clone
        @deep_cloning_obj.instance_variables.each do |var|
            val = @deep_cloning_obj.instance_variable_get(var)
            begin
                @deep_cloning = true
                val = val.deep_clone
            rescue TypeError
                next
            ensure
                @deep_cloning = false
            end
            @deep_cloning_obj.instance_variable_set(var, val)
        end
        deep_cloning_obj = @deep_cloning_obj
        @deep_cloning_obj = nil
        deep_cloning_obj
    end
end

if __FILE__ == $0

    require 'stringio'
    require 'test/unit'

    class Deep
        attr_accessor :val
        def initialize(*val)
            @val = val
        end
        def inspect 
            @val.join(",")
        end
    end

    class TC_clone < Test::Unit::TestCase

        def test_shallow_copy
            a = Deep.new(1,2)
            b = a.clone
            b.val << 3 
            assert( a.inspect == b.inspect, 'shallow copy error?!?')
        end

        def test_deep_clone
            a = Deep.new(1,2)
            b = a.deep_clone
            b.val << 3 
            assert( a.inspect == "1,2", 'deep_clone error!')
            assert( b.inspect == "1,2,3", 'deep_clone error!')
        end
    end
end
