open Knot.Kore;
open AST;

include Framework.Statement({
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = (
    Common.untyped_t(string),
    Expression.expression_t('a),
  );

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
