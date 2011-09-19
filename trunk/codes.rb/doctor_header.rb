#!/usr/bin/ruby1.9.1

$compiler = '/usr/bin/g++'
$cflags   = %w{-I. -I.. -Wall -std=c++0x}
$version  = "0.5"

#
# generic header test
#

class HeaderTest

    def initialize(name, header)
        @test_name = name
        @exe =  "HDT_" + name.gsub(/[ ]/, '_')
        @header = header
        puts "executable: #{@exe}"
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
        `#{cmd}`
        $?
    end

    def compile(*files)
        if compile?(true, *files) != 0
            raise RuntimeError, "Compiler error!"
        end
    end
 
    def subheaders(*files)
        cmd = "#{$compiler} #{$cflags.join(' ')} -o #{@exe} #{files.join(' ')} -H 2>&1" 
        io = IO.popen(cmd, "r")
        list = []
        io.each do |line| 
            if line.include? ".."
                list << line.gsub(/.*\//, '').chomp
            end
        end
        list.sort.uniq
    end

    def count_includes()
        n = 0
        File.open(@header, "r" ).each_line do |line|
            n += 1 if line.chomp.index("#include") == 0
        end
        n
    end

    def test_header_name(n)
        "HDT_#{n}_#{@header}"
    end

    def create_test_header(n, skip)
        i = 0
        File.open(test_header_name(n), "w") do |h|
            File.open(@header, "r" ).each_line do |line|
                if (line.chomp.index("#include") == 0)
                    i += 1
                    if i != n && !skip.include?(i)
                        h.puts line
                    else
                        #puts "test: # #{n} line:#{i} skip include -> #{line}"
                        if n == i
                            @test_include = line
                        end
                    end
                else
                    h.puts line
                end
            end
        end
        @test_include
    end

    def create_test(test, orig_test, new_header)
        File.open(test, "w") do |t|
            File.open(orig_test, "r" ).each_line do |line|
                    t.puts line.gsub(/#{@header}/, new_header)
            end
        end
    end

    def remove_test_header(n)
        File.unlink(test_header_name(n))
    end                               

    def remove(*files)
        files.each do |f| 
            if File.exist?(f)  
                File.unlink(f) 
            end
        end       
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

        create 'DH_translation_unit.cpp', 
                "#include<#{@header}>
                 int main() 
                 { return 0; }"

        compile 'DH_translation_unit.cpp'

        remove 'DH_translation_unit.cpp', @exe
   end
end

class MultipleInclusion < HeaderTest

    def initialize(header)
        super('Multiple inclusion', header)
    end

    def run_test()
        
        create 'DH_translation_unit.cpp', 

                "#include<#{@header}>
                 #include<#{@header}>
                 int main() { return 0; }"

        compile 'DH_translation_unit.cpp'       

        remove 'DH_translation_unit.cpp', @exe      
    end
end


class MultipleTranslationUnit < HeaderTest

    def initialize(header)
        super('Multiple translation unit', header)
    end

    def run_test()

        create 'DH_translation_unit_1.cpp', 

                "#include<#{@header}>
                 int main() 
                 { return 0; }"

        create 'DH_translation_unit_2.cpp', 
                "#include<#{@header}>"

        compile 'DH_translation_unit_1.cpp', 'DH_translation_unit_2.cpp'      
        
        remove 'DH_translation_unit_1.cpp', 'DH_translation_unit_2.cpp', @exe      
    end
end


class UnnecessaryInclude < HeaderTest

    def initialize(header,test = nil)
        super("Unnecessary includes", header)
        @test = test  
    end

    def run_test()
      
        skip = []
        max_incl = count_includes

        begin
            (1..max_incl).each do |n|

                #puts "skip headers list: #{skip.join(' ')}" 
                                       
                if skip.include? n
                    next
                end
                
                test_include = create_test_header n, skip 

                if @test == nil
                    create 'DH_translation_unit.cpp', 

                        "#include<#{test_header_name n}>
                         int main() 
                         { return 0; }"
                else  
                    create_test 'DH_translation_unit.cpp', @test, test_header_name(n)  
                end

                ret = compile? false, 'DH_translation_unit.cpp'
                sub = []
                if ret == 0
                    sub = subheaders 'DH_translation_unit.cpp'
                end

                remove_test_header n
                
                remove 'DH_translation_unit.cpp', @exe

                test_include = test_include.gsub(/.*</,'').gsub(/>.*/,'')
                                
                print "#{@header}: <#{test_include.chomp}> "

                if ret == 0 and !sub.include?(test_include.chomp)
                   print "-> // possibly unnecessary\n"
                   skip << n
                   raise LocalJumpError
                else
                   print "\n";
                end
            end
        rescue LocalJumpError
            retry
        end
    end
end


if __FILE__ == $0

    puts "Doctor Header #{$version}!"

    $use_test = false
    $test_file = nil

    begin

    ARGV.each do |arg|
   
        if arg == "--test"
            $use_test = true
            next
        end

        if arg == "--help" or arg == "-h"
            puts "doctor_header [--test test.cpp] header.hpp..."
            next
        end

        if $use_test
           $test_file = arg
           $use_test = false
           next
        end

        test_list = []
        
        test_list << SimpleInclude.new(arg)
        test_list << MultipleInclusion.new(arg)
        test_list << MultipleTranslationUnit.new(arg)
    
        if $test_file and File.exist? $test_file
            #run a custom test if available...
            test_list << UnnecessaryInclude.new(arg,$test_file)
        else
            test_list << UnnecessaryInclude.new(arg)
        end

        puts "Running #{test_list.count} tests..."

        test_list.each { |test| test.run }

        $test_file = nil
    end
    rescue Exception => msg 
        STDERR.puts msg
    end
end
