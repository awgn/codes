#!/usr/bin/ruby1.9 -w

compiler = '/usr/bin/g++'

cflags = %w{-Wall}

if __FILE__ == $0

    ARGV.each do |arg|

    puts "* Testing ", arg

    File.open("include_test_1.cc", "w") do |f1| 
    f1.puts "#include<#{arg}>"
    f1.puts "int main(int,char *[]) {}"
    end

    File.open("include_test_2.cc", "w") do |f2|
    f2.puts "#include<#{arg}>"
    end

    exe = "it_#{arg}.out"
    if (File.exist?(exe))
        next
    end

    `#{compiler} -I . #{cflags.join(' ')} -o #{exe} include_test_1.cc include_test_2.cc` 
    puts $?

    File.unlink("include_test_1.cc")    
    File.unlink("include_test_2.cc")    

    if ($? != 0)
        raise "compiler error!"
    end
    end
end
