include Knot.Kore;
include Test.Infix;
include ModuleAliases;

module Parser = Parse.Parser;
module InputStream = File.InputStream;

module A = AST.Result;
module AE = AST.Expression;
module AM = AST.Module;
module AR = AST.Raw;
module T = AST.Type;
module TE = AST.TypeExpression;
module U = Util.ResultUtil;
