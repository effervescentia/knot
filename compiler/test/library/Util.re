open Kore;
open Reference;

module A = AST;
module AR = AST.Raw;
module T = Type;
module TR = Type.Raw;

module CommonUtil = {
  let as_raw_node = (~range=Range.zero, x) => Node.Raw.create(x, range);
  let as_node = (~range=Range.zero, type_, x) => (x, type_, range);

  let to_scope = (types: list((string, T.t))): DeclarationTable.t => {
    types
    |> List.map(Tuple.map_fst2(A.of_public % (x => Export.Named(x))))
    |> List.to_seq
    |> DeclarationTable.of_seq;
  };
};

module RawUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_unknown = x => as_node(TR.(`Unknown), x);
  let as_nil = x => as_node(TR.(`Nil), x);
  let as_bool = x => as_node(TR.(`Boolean), x);
  let as_int = x => as_node(TR.(`Integer), x);
  let as_float = x => as_node(TR.(`Float), x);
  let as_string = x => as_node(TR.(`String), x);
  let as_element = x => as_node(TR.(`Element), x);
  let as_function = (args, res, x) =>
    as_node(TR.(`Function((args, res))), x);

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

  let as_invalid = (inv, x) => as_node(T.Invalid(inv), x);
  let as_nil = x => as_node(T.Valid(`Nil), x);
  let as_bool = x => as_node(T.Valid(`Boolean), x);
  let as_int = x => as_node(T.Valid(`Integer), x);
  let as_float = x => as_node(T.Valid(`Float), x);
  let as_string = x => as_node(T.Valid(`String), x);
  let as_element = x => as_node(T.Valid(`Element), x);
  let as_function = (args, res, x) =>
    as_node(T.Valid(`Function((args, res))), x);

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
