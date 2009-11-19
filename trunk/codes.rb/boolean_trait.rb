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
# Boolean Trait

module BooleanTrait

    def self.included(base)
        base.extend(ClassMethods)
    end

    module ClassMethods
        def boolean_trait(*met)
            met.each do |m|
                code = %Q{
                def non_#{m}
                    @#{m} = false
                    self    
                end
                def #{m}
                    @#{m} =  true
                    self    
                end
                def is_#{m}?
                    @#{m}
                end
                }
                class_eval(code)
            end
        end
    end
end


if __FILE__ == $0
    
   require 'test/unit'

   class Klass        
       include BooleanTrait
       boolean_trait :property

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

       # def test_missing_1
       #     x = Klass.new
       #     assert_raise(NoMethodError) { x.missing }
       # end        
       # def test_missing_2
       #     x = Klass.new
       #     assert_raise(NoMethodError) { x.non_missing }
       # end
       # def test_missing_3
       #     x = Klass.new
       #     assert_raise(NoMethodError) { x.is_missing? }
       # end
   end 

end

