open Infix;

type types_t = list((string, Type.t));

module Symbols = {
  type t = {
    mutable types: types_t,
    mutable values: types_t,
  };

  /* static */

  let create = (): t => {types: [], values: []};

  /* methods */

  let concat = (lhs: t, rhs: t): t => {
    types: lhs.types @ rhs.types,
    values: lhs.values @ rhs.values,
  };

  let clone = (symbols: t): t => {
    types: symbols.types,
    values: symbols.values,
  };

  /* pretty printing */

  let pp: Fmt.t(t) =
    (ppf, {types, values}) =>
      Fmt.(
        struct_(
          string,
          record(string, Type.pp),
          ppf,
          ("Symbols", [("types", types), ("values", values)]),
        )
      );
};

type t = {
  imported: Symbols.t,
  declared: Symbols.t,
  mutable main: option(Type.t),
};

/* static */

let create = (): t => {
  imported: Symbols.create(),
  declared: Symbols.create(),
  main: None,
};

/* methods */

let import = (id: string, type_: Type.t, table: t) =>
  table.imported.values = table.imported.values @ [(id, type_)];

let declare_type = (id: string, type_: Type.t, table: t) =>
  table.declared.types = table.declared.types @ [(id, type_)];

let declare_value = (~main=false, id: string, type_: Type.t, table: t) => {
  table.declared.values = table.declared.values @ [(id, type_)];

  if (main) {
    table.main = Some(type_);
  };
};

let resolve_type = (~no_imports=false, id: string, table: t): option(Type.t) =>
  table.declared.types
  @ (no_imports ? table.imported.types : [])
  |> List.assoc_opt(id);

let resolve_value = (~no_imports=false, id: string, table: t): option(Type.t) =>
  table.declared.values
  @ (no_imports ? table.imported.values : [])
  |> List.assoc_opt(id);

let to_module_type = (table: t): Type.t =>
  Type.(
    Valid(
      `Module(
        (
          table.declared.types
          |> List.map(Tuple.map_snd2(t => Container.Type(t)))
        )
        @ (
          table.declared.values
          |> List.map(Tuple.map_snd2(t => Container.Value(t)))
        ),
      ),
    )
  );

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, {imported, declared}) =>
    Fmt.(
      struct_(
        string,
        Symbols.pp,
        ppf,
        ("SymbolTable", [("imported", imported), ("declared", declared)]),
      )
    );
