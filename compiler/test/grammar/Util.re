open Kore;

let wrap_block = (~cursor=Cursor.zero) => Block.create(cursor);

let nil_prim = AST.nil |> AST.of_prim;

let bool_prim = AST.of_bool % AST.of_prim;

let int_prim =
  Int64.of_int % AST.of_int % wrap_block % AST.of_num % AST.of_prim;

let float_prim = AST.of_float % wrap_block % AST.of_num % AST.of_prim;

let string_prim = AST.of_string % AST.of_prim;

let jsx_node = AST.of_tag % AST.of_node;

let jsx = AST.of_tag % AST.of_jsx;

let some = x => Some(x);
