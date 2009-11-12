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
        def class_method 
            puts "-> #{__method__}"
        end
    end

    module InstanceMethods
        def instance_method 
            puts "-> #{__method__}"
        end
    end

end

class Test 
    include XXX
end

if __FILE__ == $0
    Test.class_method
    Test.new.instance_method
end
