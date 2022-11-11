open Knot.Kore;
open AST;

let pp = Formatter.pp_variable;

include Framework.Statement({
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = (
    Common.untyped_t(string),
    Expression.expression_t('a),
  );

  let parse = Parser.variable;

  let format = pp;

  let to_xml = Debug.to_xml;
});
