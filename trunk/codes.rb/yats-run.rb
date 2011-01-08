#!/usr/bin/ruby1.9.1 

if __FILE__ == $0
    executable = Dir.glob("test*").sort 
    puts "YATS: running #{executable.size} executable tests..."
    n = 0
    executable.each { |test|
        if(system("./#{test}"))
                n += 1        
        end
    } 
    puts "#{n}/#{executable.size} test passed."   
end
