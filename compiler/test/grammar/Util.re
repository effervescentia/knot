open Kore;

let to_block = (~type_=?, ~cursor=Cursor.zero) =>
  Block.create(cursor, ~type_?);

let as_nil = x => to_block(~type_=Type.K_Nil, x);

let as_bool = x => to_block(~type_=Type.K_Boolean, x);

let as_int = x => to_block(~type_=Type.K_Integer, x);

let as_float = x => to_block(~type_=Type.K_Float, x);

let as_string = x => to_block(~type_=Type.K_String, x);

let as_invalid = x => to_block(~type_=Type.K_Invalid, x);

let (<+>) = (l, r) => l % to_block % r;

let nil_prim = AST.nil |> as_nil |> AST.of_prim;

let bool_prim = AST.of_bool % as_bool % AST.of_prim;

let int_prim =
  Int64.of_int % AST.of_int % as_int % AST.of_num % as_int % AST.of_prim;

let float_prim = AST.of_float % as_float % AST.of_num % as_float % AST.of_prim;

let string_prim = AST.of_string % as_string % AST.of_prim;

let inv_id = x => to_block(~type_=Type.K_Invalid, x) |> AST.of_id;

let jsx_node = AST.of_tag % AST.of_node;

let jsx = AST.of_tag % AST.of_jsx;

let some = x => Some(x);