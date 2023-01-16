open Knot.Kore;

type t =
  | Nil
  | Boolean(KBoolean.Plugin.value_t)
  | Integer(KInteger.Plugin.value_t)
  | Float(KFloat.Plugin.value_t)
  | String(KString.Plugin.value_t);

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = unit;
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) = unit;
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = t;
  });

/* static */

let nil = Nil;
let of_boolean = x => Boolean(x);
let of_integer = x => Integer(x);
let of_float = x => Float(x);
let of_string = x => String(x);

/* methods */

let fold = (~nil, ~boolean, ~integer, ~float, ~string) =>
  fun
  | Nil => nil()
  | Boolean(x) => boolean(x)
  | Integer(x) => integer(x)
  | Float(x) => float(x)
  | String(x) => string(x);
