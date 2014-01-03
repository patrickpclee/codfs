#!/bin/perl
# Valentin Heinitz, 2010-03-20
# Script for making C++ class which converts enum to string.
# Version: 0.03
# This script is Public Domain. Copy, modify and redistribute it as you want.
# The software is provided "as is", without warranty of any kind.
#
# Call it from Source-code root-directory with command-line:
# perl enum2string.pl [INPUT FILE] > [ToStringClassFileName.h]
use strict;
use warnings;
#Found enum types will be added to this list
my @etypes=();
#Found includes will be added to this list
my @includes=();

# Pattern for searching enum types and corresponding include-file. 
my $pat="^[^\t]+\t([^\t]+)\t.*enum:([a-zA-Z_][a-zA-Z0-9_]*)";
my $file = "tags";

system "ctags -R $ARGV[0]";

open T, ">tmp" or die "couldn't open tmp\n";
open F, $file or die "couldn't open $file\n";

#Replacing MS-Windows paths by UNIX paths (for includes)
while (<F>) {
 $_ =~ s/\\/\//;
 print T "$_";
}
close F;
close T;

$file="tmp";

open F, $file or die "couldn't open $file\n";
while (<F>) {
  if (my ($m) = m/$pat/){
    push @etypes, $2;
    push @includes, $1;
    #print "$_\n";
  }
}
close F;

#Class name to be generated. Change as you like, or set from argument 
my $clName = 'EnumToString';

#Remove duplicates from enum type list
my %hlp1 = ();
my @uniqenums = grep { ! $hlp1{$_} ++ } @etypes;

#Remove duplicates from include list
my %hlp2 = ();
my @uniqinc = grep { ! $hlp2{$_} ++ } @includes;



print "#ifndef _H_G__$clName\n";
print "#define _H_G__$clName\n\n";
print "#include <iostream>\nusing std::cout;\n";

foreach my $inc (@uniqinc)
{
  print "#include \"$inc\"\n";
}

print "\n\n";

print "class $clName {\n";
print "  $clName(); // utility class, no instances\n\n";
print "public:\n";


foreach my $k (@uniqenums)
{
  $pat = "^([a-zA-Z0-9_]*)\t.*e\tenum:$k";
  print "  static const char * toString( $k en ) {\n";
  print "    switch( en ) {\n";
  open F, $file or die "couldn't open $file\n";
  while (<F>) {
    if (my ($m) = m/$pat/){
      print "      case $1: return \"$1\";\n";
    }
  }
  close F;
  print "    }\n";
  print "    return \"???\";\n";
  print "  }\n\n";

}

#Test
#print "static void testEnum2String() {\n";
#foreach my $k (@uniqenums)
#{
#  $pat = "^([a-zA-Z0-9_]*)\t.*e\tenum:$k";
#  print "  std::cout << \"Enums of $k:\\n\";\n";
#  open F, $file or die "couldn't open $file\n";
#  while (<F>) {
#    if (my ($m) = m/$pat/){
#      print "  std::cout <<\"   \"<<  toString( $1 ) <<\" -> \" << $1 <<\"\\n\";\n";
#    }
#  }
#  close F;
#}
#print "  }\n\n";

print "};\n\n";
print "#endif\n";

unlink ("tags");
unlink ("tmp");
