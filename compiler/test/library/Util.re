open Kore;
open Reference;

module A = AST;
module AR = AST.Raw;
module T = Type;
module TR = Type.Raw;

module CommonUtil = {
  let as_untyped = (~range=Range.zero, x) => Node.untyped(x, range);
  let as_typed = (~range=Range.zero, type_, x) =>
    Node.typed(x, type_, range);
};

module RawUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_unknown = x => as_typed(TR.(`Unknown), x);
  let as_nil = x => as_typed(TR.(`Nil), x);
  let as_bool = x => as_typed(TR.(`Boolean), x);
  let as_int = x => as_typed(TR.(`Integer), x);
  let as_float = x => as_typed(TR.(`Float), x);
  let as_string = x => as_typed(TR.(`String), x);
  let as_element = x => as_typed(TR.(`Element), x);
  let as_struct = (props, x) => as_typed(TR.(`Struct(props)), x);
  let as_function = (args, res, x) =>
    as_typed(TR.(`Function((args, res))), x);
  let as_decorator = (args, target, x) =>
    as_typed(TR.(`Decorator((args, target))), x);

  /* primitive factories */

  let nil_prim = AR.nil |> AR.of_prim |> as_nil;
  let bool_prim = AR.of_bool % AR.of_prim % as_bool;
  let int_prim = Int64.of_int % AR.of_int % AR.of_num % AR.of_prim % as_int;
  let float_prim = AR.of_float % AR.of_num % AR.of_prim % as_float;
  let string_prim = AR.of_string % AR.of_prim % as_string;

  /* jsx factories */

  let jsx_node = AR.of_tag % AR.of_node;
  let jsx_tag = AR.of_tag % AR.of_jsx;
};

module ResultUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_invalid = (inv, x) => as_typed(T.Invalid(inv), x);
  let as_nil = x => as_typed(T.Valid(`Nil), x);
  let as_bool = x => as_typed(T.Valid(`Boolean), x);
  let as_int = x => as_typed(T.Valid(`Integer), x);
  let as_float = x => as_typed(T.Valid(`Float), x);
  let as_string = x => as_typed(T.Valid(`String), x);
  let as_element = x => as_typed(T.Valid(`Element), x);
  let as_enum = (variants, x) =>
    as_typed(T.Valid(`Enumerated(variants)), x);
  let as_struct = (props, x) => as_typed(T.Valid(`Struct(props)), x);
  let as_function = (args, res, x) =>
    as_typed(T.Valid(`Function((args, res))), x);
  let as_view = (props, res, x) =>
    as_typed(T.Valid(`View((props, res))), x);
  let as_style = (props, ids, classes, x) =>
    as_typed(T.Valid(`Style((props, ids, classes))), x);
  let as_decorator = (args, target, x) =>
    as_typed(T.Valid(`Decorator((args, target))), x);

  /* primitive factories */

  let nil_prim = A.nil |> A.of_prim |> as_nil;
  let bool_prim = A.of_bool % A.of_prim % as_bool;
  let int_prim = Int64.of_int % A.of_int % A.of_num % A.of_prim % as_int;
  let float_prim = A.of_float % A.of_num % A.of_prim % as_float;
  let string_prim = A.of_string % A.of_prim % as_string;

  /* jsx factories */

  let jsx_node = A.of_tag % A.of_node;
  let jsx_tag = A.of_tag % A.of_jsx;
};
