open Knot.Kore;
open Parse.Onyx;

let parse =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.expression_parser_t =>
  Parser.ksx(ctx, parsers)
  >|= Node.add_type(Type.Raw.(`Element))
  >|= Node.map(AST.Raw.of_jsx);

let pp = Formatter.pp_ksx;

let dump = pp;
