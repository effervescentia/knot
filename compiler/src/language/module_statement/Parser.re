open Knot.Kore;
open Parse.Kore;
open AST;

let parse = (ctx: ParseContext.t) =>
  choice([KImport.Plugin.parse(ctx), KDeclaration.Plugin.parse(ctx)]);
