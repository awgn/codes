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


module XXX
    
    def self.included(base)
        puts "!module included in #{base}!"
        base.extend(ClassMethods)
        base.send(:include, InstanceMethods)
    end

    module ClassMethods
        def class_method(out) 
            out.print "#{__method__}"
        end
    end

    module InstanceMethods
        def instance_method(out) 
            out.print "#{__method__}"
        end
    end

end

if __FILE__ == $0

    require 'stringio'
    require 'test/unit'

    class TestMixin
        include XXX
    end

    class TC_test < Test::Unit::TestCase 
        include XXX

        def test_class_method
            out = StringIO.new
            TestMixin.class_method(out)
            assert(out.string == "class_method",'instance method error');
        end

        def test_instance_method
            out = StringIO.new
            TestMixin.new.instance_method(out)
            assert(out.string == "instance_method",'instance method error');
        end
    end

end
