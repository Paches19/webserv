#!/usr/bin/perl

print("<html>")
print("<head>")
print("<h2>Environment with PERL</h2><br>")
print("</head>")
print("<body>")
# Loop through all environment variables and print them
foreach my $key (keys %ENV) {
    print "$key=$ENV{$key}<br>";
}
print("</body>")
print("</html>")