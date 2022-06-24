open Infix;
open Reference;

module Symbols = SymbolTable.Symbols;

type t = {
  /* unique identifier for this namespace */
  namespace: Namespace.t,
  /* error reporting callback */
  report: Error.compile_err => unit,
  /* the active module table for the compiler instance */
  modules: ModuleTable.t,
  /* the symbols in context for this module */
  symbols: SymbolTable.t,
};

/* static */

let create =
    (
      ~modules=ModuleTable.create(0),
      ~symbols=SymbolTable.create(),
      ~report=Error.throw,
      namespace: Namespace.t,
    ) => {
  namespace,
  report,
  modules,
  symbols,
};

let create_module = (parent: t): t =>
  create(
    ~modules=parent.modules,
    ~symbols={
      ...SymbolTable.create(),
      imported:
        Symbols.concat(parent.symbols.imported, parent.symbols.declared),
    },
    ~report=parent.report,
    parent.namespace,
  );
