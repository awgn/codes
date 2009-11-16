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
