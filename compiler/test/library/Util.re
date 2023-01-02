open Kore;

module A = AST.Result;
module AR = AST.Raw;
module T = AST.Type;

module CommonUtil = {
  let as_untyped = (~range=Range.zero, x) => Node.raw(x, range);
  let as_typed = (~range=Range.zero, type_, x) =>
    Node.typed(x, type_, range);
};

module RawUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_node = x => as_typed((), x);
  let as_unknown = x => as_typed(`Unknown, x);
  let as_nil = x => as_typed(`Nil, x);
  let as_bool = x => as_typed(`Boolean, x);
  let as_int = x => as_typed(`Integer, x);
  let as_float = x => as_typed(`Float, x);
  let as_string = x => as_typed(`String, x);
  let as_element = x => as_typed(`Element, x);
  let as_style = x => as_typed(`Style, x);
  let as_struct = (props, x) => as_typed(`Object(props), x);
  let as_function = (args, res, x) => as_typed(`Function((args, res)), x);
  let as_decorator = (args, target, x) =>
    as_typed(`Decorator((args, target)), x);

  /* primitive factories */

  let nil_prim = AR.nil |> AR.of_prim |> as_node;
  let bool_prim = AR.of_bool % AR.of_prim % as_node;
  let int_prim = Int64.of_int % AR.of_int % AR.of_prim % as_node;
  let float_prim = AR.of_float % AR.of_prim % as_node;
  let string_prim = AR.of_string % AR.of_prim % as_node;

  /* jsx factories */

  let ksx_node = x => x |> AR.of_element_tag |> AR.of_node;
  let ksx_tag = x => x |> AR.of_element_tag |> AR.of_ksx;
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
  let as_style = x => as_typed(T.Valid(`Style), x);
  let as_enum = (variants, x) =>
    as_typed(T.Valid(`Enumerated(variants)), x);
  let as_struct = (props, x) => as_typed(T.Valid(`Object(props)), x);
  let as_function = (args, res, x) =>
    as_typed(T.Valid(`Function((args, res))), x);
  let as_view = (props, res, x) =>
    as_typed(T.Valid(`View((props, res))), x);
  let as_decorator = (args, target, x) =>
    as_typed(T.Valid(`Decorator((args, target))), x);

  /* primitive factories */

  let nil_prim = A.nil |> A.of_prim |> as_nil;
  let bool_prim = A.of_bool % A.of_prim % as_bool;
  let int_prim = Int64.of_int % A.of_int % A.of_prim % as_int;
  let float_prim = A.of_float % A.of_prim % as_float;
  let string_prim = A.of_string % A.of_prim % as_string;

  /* jsx factories */

  let ksx_node = x => x |> A.of_element_tag |> A.of_node;
  let ksx_tag = x => x |> A.of_element_tag |> A.of_ksx;
};
