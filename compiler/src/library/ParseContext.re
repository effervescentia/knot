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
  let symbols =
    module_
    |?< ModuleTable.(get_entry_data % Option.map(({symbols}) => symbols));

  let type_export =
    symbols
    |?< SymbolTable.(
          symbols' =>
            switch (id) {
            | Named(id') => symbols'.declared.types |> List.assoc_opt(id')
            | Main => symbols'.main
            }
        );

  let value_export =
    symbols
    |?< SymbolTable.(
          symbols' =>
            switch (id) {
            | Named(id') => symbols'.declared.values |> List.assoc_opt(id')
            | Main => symbols'.main
            }
        );

  switch (value_export, type_export) {
  | (None, None) =>
    let invalid_type = Type.Invalid(NotInferrable);
    ctx.symbols |> SymbolTable.import_type(alias, invalid_type);
    ctx.symbols |> SymbolTable.import_value(alias, invalid_type);

    Error(Type.ExternalNotFound(namespace, id));

  | _ =>
    type_export
    |> Option.iter(type_ =>
         ctx.symbols |> SymbolTable.import_type(alias, type_)
       );
    value_export
    |> Option.iter(type_ =>
         ctx.symbols |> SymbolTable.import_value(alias, type_)
       );

    Ok((type_export, value_export));
  };
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
