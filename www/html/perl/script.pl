use strict;
use CGI ':standard';
use Encode 'decode_utf8';
binmode STDOUT, ":utf8";
  
my $name = decode_utf8(param('name'));
my $gender = decode_utf8(param('gender'));
my $profession = decode_utf8(param('profession'));
my @sports = decode_utf8(param('sport'));
  
my $list;
  
if (@sports) 
{
    $list = join ', ', @sports;
} 
else 
{
    $list = 'Null';
}
  
print header,
start_html(-title=>$name, -encoding => "UTF-8"),
h1("Hello, $name"),
h3 p('You have Submitted the following Data:'),
h4 table(Tr(td('Name:'),
h4 td($name)),
h4 Tr(td('Gender:'),
h4 td($gender)),
h4 Tr(td('Profession:'),
h4 td($profession)),
h4 Tr(td('Sports:'),
h4 td($list))),
end_html;