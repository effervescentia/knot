include OUnit2;
include Knot.Token;
include KnotParse.AST;
include Assert;

module LazyStream = Opal.LazyStream;
module FileStream = Knot.FileStream;
module ContextualStream = Knot.ContextualStream;
module TokenStream = KnotLex.TokenStream;

module Lexer = KnotLex.Lexer;
module Parser = KnotParse.Parser;
