#!/usr/bin/ruby1.9.1

$compiler = '/usr/bin/g++'
$cflags   = %w{-I . -Wall -std=c++0x}
$verbose  = false
$version  = "0.1"

#
# generic header test
#

class HeaderTest

    def initialize(name, header)
        @test_name = name
        @exe =  "HT_" + name.gsub(/[ ]/, '_')
        @header = header
    end

    def run() 
        # check whether the test was already compiled

        if (File.exist?(@exe))
            return
        end

        print "#{@header}: -> TEST|#{@test_name}\n"
        run_test

    end

    def create(file, *lines)
        File.open(file, "w") do |f|
            lines.each { |l| f.puts l } 
        end
    end

    def compile?(output, *files)
        
        cmd = output ?  "#{$compiler} #{$cflags.join(' ')} -o #{@exe} #{files.join(' ')}" :
                        "#{$compiler} #{$cflags.join(' ')} -o #{@exe} #{files.join(' ')} > /dev/null 2>&1"

        puts "COMMAND: #{cmd}" unless !$verbose
        `#{cmd}`

        files.each { |f| File.unlink(f) }
        File.unlink(@exe) if $? == 0
        $?
    end

    def compile(*files)
        if compile?(true, *files) != 0
            raise RuntimeError, "Compier error!"
        end
    end

    def count_includes()
        n = 0
        File.open(@header, "r" ).each_line do |line|
            n += 1 if line.index("#include")
        end
        n
    end

    def test_header_name(n)
        "HT_#{n}_#{@header}"
    end

    def create_test_header(n, skip)
        i = 0
        File.open(test_header_name(n), "w") do |h|

            File.open(@header, "r" ).each_line do |line|
                if (line.index("#include"))
                    i += 1
                    if i != n && !skip.include?(i)
                        h.puts line
                    else
                        @test_include = line
                    end

                else
                    h.puts line
                end
            end

        end
        @test_include
    end

    def remove_test_header(n)
        File.unlink(test_header_name n)
    end
end

#
# header test..
#

class SimpleInclude < HeaderTest

    def initialize(header)
        super('Simple include', header)
    end

    def run_test()

        create 'translation_unit.cpp', 
                "#include<#{@header}>
                 int main() 
                 { return 0; }"

        compile 'translation_unit.cpp'
   end
end

class MultipleInclusion < HeaderTest

    def initialize(header)
        super('Multiple inclusion', header)
    end

    def run_test()
        
        create 'translation_unit.cpp', 

                "#include<#{@header}>
                 #include<#{@header}>
                 int main() { return 0; }"

        compile 'translation_unit.cpp'       
    end
end


class MultipleTranslationUnit < HeaderTest

    def initialize(header)
        super('Multiple translation unit', header)
    end

    def run_test()

        create 'translation_unit_1.cpp', 

                "#include<#{@header}>
                 int main() 
                 { return 0; }"

        create 'translation_unit_2.cpp', 
                "#include<#{@header}>"

        compile 'translation_unit_1.cpp', 'translation_unit_2.cpp'      
    end
end


class PointlessInclude < HeaderTest

    def initialize(header)
        super('Pointless includes', header)
    end

    def run_test()
      
        skip = []
        max_incl = count_includes

        begin
            (1.. max_incl).each do |n|

                if skip.include? n
                    next
                end
                
                ti = create_test_header n, skip 

                create 'translation_unit.cpp', 

                        "#include<#{test_header_name n}>
                         int main() 
                         { return 0; }"

                ret = compile? false, 'translation_unit.cpp'
                
                remove_test_header n
                
                if ret == 0
                   print "#{@header}: ->      #{ti.chomp} /* pointless */\n"
                   skip << n
                   raise LocalJumpError
                end
            end
        rescue LocalJumpError
            retry
        end
    end
end


if __FILE__ == $0

    puts "Doctor Header #{$version}!"

    begin
    ARGV.each do |arg|
    
        test_list = []
        # test_list << SimpleInclude.new(arg)
        test_list << MultipleInclusion.new(arg)
        test_list << MultipleTranslationUnit.new(arg)
        test_list << PointlessInclude.new(arg)

        test_list.each { |test| test.run }
    end
    rescue Exception => msg 
        STDERR.puts msg
    end
end
