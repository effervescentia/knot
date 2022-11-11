open Knot.Kore;
open AST;

let pp = Formatter.pp_effect;

include Framework.Statement({
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = Expression.expression_t('a);

  let parse = Parser.effect;

  let format = pp;

  let to_xml = (expr_to_xml, expr) =>
    Fmt.Node("Effect", [], [expr_to_xml(expr)]);
});
