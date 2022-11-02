open Kore;

module ParseContext = AST.ParseContext;
module SymbolTable = AST.SymbolTable;
module Symbols = AST.SymbolTable.Symbols;
module Type = AST.Type;

let __symbols =
  Symbols.{
    types: [("foo", Valid(`Boolean)), ("bar", Valid(`String))],
    values: [("bar", Valid(`String))],
  };

let suite =
  "Library.ParseContext"
  >::: [
    "create_module() - import context from parents"
    >: (
      () => {
        let ctx = ParseContext.create(Stdlib);

        ctx.symbols |> SymbolTable.declare_type("foo", Type.Valid(`Boolean));

        let module_1 = ParseContext.create_module(ctx);

        module_1.symbols
        |> SymbolTable.declare_type("bar", Type.Valid(`String));
        module_1.symbols
        |> SymbolTable.declare_value("bar", Type.Valid(`String));

        let module_1_1 = ParseContext.create_module(module_1);

        Assert.symbols(__symbols, module_1_1.symbols.imported);
      }
    ),
  ];
