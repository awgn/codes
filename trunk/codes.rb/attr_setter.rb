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

#
# Attribute Setter

module AttrSetter
    def self.included(base)
        base.extend(ClassMethods)
        base.send(:include, InstaceMethod)
    end

    module ClassMethods
        def attr_setter(*met)
            met.each do |m|
                code = %Q{
                def set_#{m}(value)
                    @#{m} = value    
                end
                def is_#{m}?
                    @#{m}
                end
                }
                class_eval(code)
            end
        end
    end

    module InstaceMethod
        def method_missing(method, *)
            m = method.to_s.split('_')
            if (m[0]=='non')
                self.send("set_#{m[1]}", false)
                return self
            end
            if (m[0]=='set')
                raise NoMethodError, "undefined method `#{m[1]}' for #{self}"
            end
            self.send("set_#{method}", true)
            self
        end
    end
end


if __FILE__ == $0
    
   require 'test/unit'

   class Klass        
       include AttrSetter
       attr_setter :property

       def initialize
           @property = false
       end
   end

   class TC_test < Test::Unit::TestCase
        def test_false
            x = Klass.new
            x.non_property
            assert_equal(x.is_property?, false)
        end        
        def test_true
            x = Klass.new
            x.property
            assert_equal(x.is_property?, true)
        end
        def test_false_true
            x = Klass.new
            x.non_property.property
            assert_equal(x.is_property?, true)
        end
        def test_true_false
            x = Klass.new
            x.property.non_property
            assert_equal(x.is_property?, false)
        end

        def test_missing_property_1
            x = Klass.new
            assert_raise(NoMethodError) { x.missing_property }
        end        
        def test_missing_property_2
            x = Klass.new
            assert_raise(NoMethodError) { x.non_missing_property }
        end
        def test_missing_property_3
            x = Klass.new
            assert_raise(NoMethodError) { x.is_missing_property? }
        end
   end 
end

