open Kore;

let nil_prim = AST.nil |> AST.of_prim;

let bool_prim = AST.of_bool % AST.of_prim;

let int_prim = Int64.of_int % AST.of_int % AST.of_num % AST.of_prim;

let float_prim = AST.of_float % AST.of_num % AST.of_prim;
