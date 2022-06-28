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

/* methods */

/**
 report a compile error
 */
let report = (err: Error.parse_err, range: Range.t, ctx: t) =>
  ctx.report(ParseError(err, ctx.namespace, range));

/**
 find the type of an export from a different module and import it into the symbol table
 */
let import = (namespace: Namespace.t, id: Export.t, alias: string, ctx: t) => {
  let module_ = ctx.modules |> ModuleTable.find(namespace);
  let export =
    module_
    |?< ModuleTable.(get_entry_data % Option.map(({exports}) => exports))
    |?< (exports => Hashtbl.find_opt(exports, id));

  ctx.symbols
  |> SymbolTable.import(alias, export |?: Type.Invalid(NotInferrable));

  export |> Option.to_result(~none=Type.ExternalNotFound(namespace, id));
};

/**
 convert to a scope for use within a function or other closure
 */
let to_scope = (range: Range.t, ctx: t): Scope.t => {
  let types =
    ctx.symbols.imported.values
    @ ctx.symbols.declared.values
    |> List.to_seq
    |> Hashtbl.of_seq;

  {...Scope.create(ctx.namespace, ctx.report, range), types};
};
