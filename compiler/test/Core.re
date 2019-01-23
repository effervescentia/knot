include OUnit2;
include Knot.Token;
include KnotParse.AST;
include Assert;

module LazyStream = Opal.LazyStream;
module FileStream = Knot.FileStream;
module TokenStream = KnotLex.TokenStream;
module ContextualTokenStream = KnotLex.ContextualTokenStream;

module Lexer = KnotLex.Lexer;
module Parser = KnotParse.Parser;
