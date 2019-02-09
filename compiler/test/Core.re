include OUnit2;
include Knot.Token;
include Knot.AST;
include Assert;

module LazyStream = Opal.LazyStream;
module FileStream = Knot.FileStream;
module TokenStream = KnotLex.TokenStream;

module Lexer = KnotLex.Lexer;
module Parser = KnotParse.Parser;
