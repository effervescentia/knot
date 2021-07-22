open Kore;
open Reference;

let to_scope = (types: list((string, Type.t))) => {
  let scope = Scope.create();

  types
  |> List.map(Tuple.map_fst2(AST.of_public % (x => Export.Named(x))))
  |> List.to_seq
  |> Hashtbl.add_seq(scope.types);

  scope;
};

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

let nil_prim = AST.nil |> as_nil |> AST.of_prim |> as_nil;

let bool_prim = AST.of_bool % as_bool % AST.of_prim % as_bool;

let int_prim =
  Int64.of_int % AST.of_int % AST.of_num % as_int % AST.of_prim % as_int;

let float_prim = AST.of_float % AST.of_num % as_float % AST.of_prim % as_float;

let string_prim = AST.of_string % as_string % AST.of_prim % as_string;

let jsx_node = AST.of_tag % as_lexeme % AST.of_node;

let jsx_tag = AST.of_tag % as_lexeme % AST.of_jsx;

let print_parse_err =
  fun
  | TypeError(err) =>
    err
    |> Type2.(Error.to_string(Raw.strong_to_string))
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
      err |> parse_err_to_string,
      namespace |> Namespace.to_string,
      cursor |> Debug.print_cursor,
    );

let print_errs =
  Print.many(~separator="\n\n", print_compile_err)
  % Print.fmt("found some errors during compilation:\n\n%s");
