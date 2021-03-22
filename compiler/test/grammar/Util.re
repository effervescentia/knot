open Kore;

let as_lexeme = (~cursor=Cursor.zero, x) => (x, cursor);
let as_typed_lexeme = (~cursor=Cursor.zero, type_, x) => (x, type_, cursor);

let as_nil = x => as_typed_lexeme(Type.K_Nil, x);

let as_bool = x => as_typed_lexeme(Type.K_Boolean, x);

let as_int = x => as_typed_lexeme(Type.K_Integer, x);

let as_float = x => as_typed_lexeme(Type.K_Float, x);

let as_string = x => as_typed_lexeme(Type.K_String, x);

let as_element = x => as_typed_lexeme(Type.K_Element, x);

let as_unknown = x => as_typed_lexeme(Type.K_Unknown, x);

let as_invalid = x => as_typed_lexeme(Type.K_Invalid, x);

let nil_prim = AST.nil |> as_nil |> AST.of_prim |> as_nil;

let bool_prim = AST.of_bool % as_bool % AST.of_prim % as_bool;

let int_prim =
  Int64.of_int % AST.of_int % AST.of_num % as_int % AST.of_prim % as_int;

let float_prim = AST.of_float % AST.of_num % as_float % AST.of_prim % as_float;

let string_prim = AST.of_string % as_string % AST.of_prim % as_string;

let jsx_node = AST.of_tag % as_lexeme % AST.of_node;

let jsx_tag = AST.of_tag % as_lexeme % AST.of_jsx;

let some = x => Some(x);
