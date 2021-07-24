open Kore;
open Reference;

/* let to_scope = (types: list((string, Type2.Raw.t))) => {
     let scope = Scope.create();

     types
     |> List.map(Tuple.map_fst2(AST.of_public % (x => Export.Named(x))))
     |> List.to_seq
     |> Hashtbl.add_seq(scope.types);

     scope;
   }; */

let as_lexeme = (~cursor=Cursor.zero, x) => (x, cursor);
let as_typed_lexeme = (~cursor=Cursor.zero, type_, x) => (x, type_, cursor);

let as_nil = x => as_typed_lexeme(Type2.Raw.Strong(`Nil), x);

let as_bool = x => as_typed_lexeme(Type2.Raw.Strong(`Boolean), x);

let as_int = x => as_typed_lexeme(Type2.Raw.Strong(`Integer), x);

let as_float = x => as_typed_lexeme(Type2.Raw.Strong(`Float), x);

let as_string = x => as_typed_lexeme(Type2.Raw.Strong(`String), x);

let as_element = x => as_typed_lexeme(Type2.Raw.Strong(`Element), x);

let as_invalid = (err, x) => as_typed_lexeme(Type2.Raw.Invalid(err), x);

let as_weak = (id, x) => as_typed_lexeme(Type2.Raw.Weak(id), x);

let nil_prim = AST.Raw.nil |> as_nil |> AST.Raw.of_prim |> as_nil;

let bool_prim = AST.Raw.of_bool % as_bool % AST.Raw.of_prim % as_bool;

let int_prim =
  Int64.of_int
  % AST.Raw.of_int
  % AST.Raw.of_num
  % as_int
  % AST.Raw.of_prim
  % as_int;

let float_prim =
  AST.Raw.of_float % AST.Raw.of_num % as_float % AST.Raw.of_prim % as_float;

let string_prim = AST.Raw.of_string % as_string % AST.Raw.of_prim % as_string;

let jsx_node = AST.Raw.of_tag % as_lexeme % AST.Raw.of_node;

let jsx_tag = AST.Raw.of_tag % as_lexeme % AST.Raw.of_jsx;

let print_parse_err =
  fun
  | TypeError(err) =>
    err
    |> Type2.(Error.to_string(Raw.to_string))
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
