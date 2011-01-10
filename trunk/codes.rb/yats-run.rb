#!/usr/bin/ruby1.9.1 

if __FILE__ == $0
    executable = Dir.glob("test*").delete_if { |e| !File.executable?(e) } 
    puts "YATS: running #{executable.size} executable tests..."
    n = 0
    executable.sort.each { |test|
        if(system("./#{test}"))
                n += 1        
        end
    } 
    puts "#{n}/#{executable.size} tests passed."   
end
