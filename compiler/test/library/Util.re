open Kore;
open Reference;

module CommonUtil = {
  let as_raw_node = (~range=Range.zero, x) => (x, range);
  let as_node = (~range=Range.zero, type_, x) => (x, type_, range);

  let to_scope = (types: list((string, Type.t))): DefinitionTable.t => {
    types
    |> List.map(Tuple.map_fst2(AST.of_public % (x => Export.Named(x))))
    |> List.to_seq
    |> DefinitionTable.of_seq;
  };
};

module RawUtil = {
  open Type.Raw;
  open AST.Raw;

  include CommonUtil;

  let as_invalid = (err, x) => as_node(Invalid(err), x);

  let as_weak = (scope_id, weak_id, x) =>
    as_node(Weak(scope_id, weak_id), x);

  let nil_prim = nil |> as_raw_node |> of_prim |> as_raw_node;

  let bool_prim = of_bool % as_raw_node % of_prim % as_raw_node;

  let int_prim =
    Int64.of_int % of_int % of_num % as_raw_node % of_prim % as_raw_node;

  let float_prim = of_float % of_num % as_raw_node % of_prim % as_raw_node;

  let string_prim = of_string % as_raw_node % of_prim % as_raw_node;

  let jsx_node = of_tag % as_raw_node % of_node;

  let jsx_tag = of_tag % as_raw_node % of_jsx;
};

module AnalyzedUtil = {
  open AST.Analyzed;
  open Type.Raw;

  include CommonUtil;

  let as_nil = x => as_node(Strong(`Nil), x);
  let as_bool = x => as_node(Strong(`Boolean), x);
  let as_int = x => as_node(Strong(`Integer), x);
  let as_float = x => as_node(Strong(`Float), x);
  let as_string = x => as_node(Strong(`String), x);
  let as_element = x => as_node(Strong(`Element), x);
  let as_function = (args, res, x) =>
    as_node(Strong(`Function((args, res))), x);

  let as_weak = (scope_id, weak_id, x) =>
    as_node(Weak(scope_id, weak_id), x);
  let as_generic = (scope_id, weak_id, x) =>
    as_node(Strong(`Generic((scope_id, weak_id))), x);

  let as_invalid = (err, x) => as_node(Invalid(err), x);

  let nil_prim = nil |> as_nil |> of_prim |> as_nil;

  let bool_prim = of_bool % as_bool % of_prim % as_bool;

  let int_prim = Int64.of_int % of_int % of_num % as_int % of_prim % as_int;

  let float_prim = of_float % of_num % as_float % of_prim % as_float;

  let string_prim = of_string % as_string % of_prim % as_string;

  let jsx_node = of_tag % as_element % of_node;

  let jsx_tag = of_tag % as_element % of_jsx;
};

module ResultUtil = {
  open AST;
  open Type;

  include CommonUtil;

  let as_nil = x => as_node(Valid(`Nil), x);
  let as_bool = x => as_node(Valid(`Boolean), x);
  let as_int = x => as_node(Valid(`Integer), x);
  let as_float = x => as_node(Valid(`Float), x);
  let as_string = x => as_node(Valid(`String), x);
  let as_element = x => as_node(Valid(`Element), x);
  let as_function = (args, res, x) =>
    as_node(Valid(`Function((args, res))), x);

  let as_generic = (scope_id, weak_id, x) =>
    as_node(Valid(`Generic((scope_id, weak_id))), x);

  let as_invalid = (err, x) => as_node(Invalid(err), x);

  let nil_prim = nil |> as_nil |> of_prim |> as_nil;

  let bool_prim = of_bool % as_bool % of_prim % as_bool;

  let int_prim = Int64.of_int % of_int % of_num % as_int % of_prim % as_int;

  let float_prim = of_float % of_num % as_float % of_prim % as_float;

  let string_prim = of_string % as_string % of_prim % as_string;

  let jsx_node = of_tag % as_element % of_node;

  let jsx_tag = of_tag % as_element % of_jsx;
};
