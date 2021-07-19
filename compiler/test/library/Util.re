open Kore;
open AST.Raw.Util;
open Reference;

let to_scope = (types: list((string, Type.t))) => {
  let scope = Scope.create();

  types
  |> List.map(Tuple.map_fst2(to_public % (x => Export.Named(x))))
  |> List.to_seq
  |> Hashtbl.add_seq(scope.types);

  scope;
};

/* typed utils */

let as_lexeme = (~cursor=Cursor.zero, x) => (x, cursor);
let as_typed_lexeme = (~cursor=Cursor.zero, type_, x) => (x, type_, cursor);

let as_nil = x => as_typed_lexeme(Type.K_Strong(K_Nil), x);
let as_bool = x => as_typed_lexeme(Type.K_Strong(K_Boolean), x);
let as_int = x => as_typed_lexeme(Type.K_Strong(K_Integer), x);
let as_float = x => as_typed_lexeme(Type.K_Strong(K_Float), x);
let as_string = x => as_typed_lexeme(Type.K_Strong(K_String), x);
let as_element = x => as_typed_lexeme(Type.K_Strong(K_Element), x);
let as_invalid = (err, x) => as_typed_lexeme(Type.K_Invalid(err), x);
let as_weak = (id, x) => as_typed_lexeme(Type.K_Weak(id), x);

/* raw utils */

let raw_nil = nil |> as_nil;
let raw_bool = to_bool % as_bool;
let raw_int = Int64.of_int % to_int % as_int;
let raw_float = to_float % as_float;
let raw_string = to_string % as_string;
let raw_public = to_public % as_lexeme;

/* primitive utils */

let nil_prim = raw_nil |> to_prim |> as_nil;
let bool_prim = raw_bool % to_prim % as_bool;
let int_prim = Int64.of_int % to_int % to_num % as_int % to_prim % as_int;
let float_prim = to_float % to_num % as_float % to_prim % as_float;
let string_prim = raw_string % to_prim % as_string;

/* type factories */

let to_neg_int = to_neg_op % as_int;
let to_neg_float = to_neg_op % as_float;
let to_not_bool = to_not_op % as_bool;
let to_public_id = f => raw_public % to_id % f;
let to_public_export = raw_public % AST.Final.Util.to_named_export;
let to_public_main_export = raw_public % AST.Final.Util.to_main_export;
let to_public_main_import = raw_public % AST.Final.Util.to_main_import;

let jsx_node = to_tag % as_lexeme % to_node;
let jsx_tag = to_tag % as_lexeme % to_jsx;

let print_parse_err =
  fun
  | TypeError(err) => err |> Type.err_to_string |> Print.fmt("TypeError<%s>")
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
      err |> _parse_err_to_string,
      namespace |> Namespace.to_string,
      cursor |> Cursor.to_string,
    );

let print_errs =
  Print.many(~separator="\n\n", print_compile_err)
  % Print.fmt("found some errors during compilation:\n\n%s");
