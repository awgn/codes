#!/usr/bin/ruby1.9

if ARGV.size < 1 || ARGV[0] == "-h" || ARGV[0] == "--help" 
        puts "#{__FILE__} TARGET LINK_NAME"
        puts "#{__FILE__} TARGET ."
        puts "#{__FILE__} LINK_NAME (symlink)"
        exit 1
end

begin
        target = ARGV[0]
        link   = ""

        if ARGV.size == 1 
                if !File.exist?(ARGV[0]) || !File.symlink?(ARGV[0])
                        raise "#{ARGV[0]} not a symlink!"
                end
                target = File.readlink ARGV[0]
                link = ARGV[0]
                File.unlink(link)
        elsif ARGV.size == 2
                link = ARGV[1] == "." ? target.split("/").last : ARGV[1]
        end

        # puts "target: #{target}"
        # puts "link: #{link}"

        if !File.exist? target
                raise "#{link} TARGET not found!"
        end

        if File.exist? link
                raise "#{link} LINK_NAME already exists!"
        end

        File.open(link, "w")  { |f| f.puts "#include \"#{target}\"" } 

rescue Exception => e 
        puts "#{__FILE__}: #{e}"
        exit 1
end
