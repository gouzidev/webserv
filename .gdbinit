# Set up layout and printing options
layout src
set print elements 0
set print repeats 10000
focus next

# Define a custom command to run with your config
define runwebserv
    run cgi.conf
end

# Print helpful message
echo \n=== GDB configured for webserv ===\n
echo Type 'runwebserv' to start with cgi.conf\n\n
