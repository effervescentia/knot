open Kore;
open Reference;

module CommonUtil = {
  let as_raw_node = (~cursor=Cursor.zero, x) => (x, cursor);
  let as_node = (~cursor=Cursor.zero, type_, x) => (x, type_, cursor);

  let to_scope = (types: list((string, Type.t))): DefinitionTable.t => {
    types
    |> List.map(Tuple.map_fst2(AST.of_public % (x => Export.Named(x))))
    |> List.to_seq
    |> DefinitionTable.from_seq;
  };

  let scope_to_closure = (scope: list((string, Type.Raw.t))) =>
    ClosureContext.create(
      ~scope=
        scope
        |> List.map(Tuple.map_fst2(AST.of_public))
        |> List.to_seq
        |> NestedHashtbl.from_seq,
    );
};

module RawUtil = {
  open Type.Raw;
  open AST.Raw;

  include CommonUtil;

  let as_invalid = (err, x) => as_node(Invalid(err), x);

  let as_abstract = (id, x) => as_node(Weak(ref(Ok(`Abstract(id)))), x);

  let nil_prim = nil |> as_raw_node |> of_prim |> as_raw_node;

  let bool_prim = of_bool % as_raw_node % of_prim % as_raw_node;

  let int_prim =
    Int64.of_int % of_int % of_num % as_raw_node % of_prim % as_raw_node;

  let float_prim = of_float % of_num % as_raw_node % of_prim % as_raw_node;

  let string_prim = of_string % as_raw_node % of_prim % as_raw_node;

  let jsx_node = of_tag % as_raw_node % of_node;

  let jsx_tag = of_tag % as_raw_node % of_jsx;

  let weak_unknown = Weak(ref(Ok(`Abstract(Type.Trait.Unknown))));
};

module ResultUtil = {
  open AST;

  include CommonUtil;

  let as_nil = x => as_node(Type.Valid(`Nil), x);
  let as_bool = x => as_node(Type.Valid(`Boolean), x);
  let as_int = x => as_node(Type.Valid(`Integer), x);
  let as_float = x => as_node(Type.Valid(`Float), x);
  let as_string = x => as_node(Type.Valid(`String), x);
  let as_element = x => as_node(Type.Valid(`Element), x);
  let as_function = (args, res, x) =>
    as_node(Type.Valid(`Function((args, res))), x);
  let as_unknown = x => as_node(Type.Valid(`Abstract(Unknown)), x);

  let as_abstract = (trait, x) => as_node(Type.Valid(`Abstract(trait)), x);

  let nil_prim = nil |> as_nil |> of_prim |> as_nil;

  let bool_prim = of_bool % as_bool % of_prim % as_bool;

  let int_prim = Int64.of_int % of_int % of_num % as_int % of_prim % as_int;

  let float_prim = of_float % of_num % as_float % of_prim % as_float;

  let string_prim = of_string % as_string % of_prim % as_string;
};

let print_parse_err =
  fun
  | TypeError(err) =>
    err
    |> Type.(Error.to_string(Raw.to_string))
    |> Print.fmt("TypeError<%s>")
  | ReservedKeyword(name) => name |> Print.fmt("ReservedKeyword<%s>");

let print_compile_err =
  fun
  | ImportCycle(cycles) =>
    cycles
    |> Print.many(~separator=" -> ", Functional.identity)
    |> Print.fmt("ImportCycle<%s>")
  | UnresolvedModule(name) => name |> Print.fmt("UnresolvedModule<%s>")
  | FileNotFound(path) => path |> Print.fmt("FileNotFound<%s>")
  | InvalidModule(namespace) =>
    namespace |> Namespace.to_string |> Print.fmt("InvalidModule<%s>")
  | ParseError(err, namespace, cursor) =>
    Print.fmt(
      "ParseError<%s, %s, %s>",
      parse_err_to_string(err),
      Namespace.to_string(namespace),
      Cursor.to_string(cursor),
    );

let print_errs =
  Print.many(~separator="\n\n", print_compile_err)
  % Print.fmt("found some errors during compilation:\n\n%s");
