open Knot.Kore;
open Reference;

module Fmt = Pretty.Formatters;

type exports_t = list((Export.t, Type.t));
type scope_tree_t = RangeTree.t(option(exports_t));
type module_entries_t =
  list((string, Type.Container.module_entry_t(Type.t)));

type library_t = {symbols: SymbolTable.t};

type module_t = {
  symbols: SymbolTable.t,
  ast: Result.program_t,
  scopes: scope_tree_t,
};

type entry_t =
  | Pending
  | Purged
  | Library(string, library_t)
  | Valid(string, module_t)
  | Partial(string, module_t, list(Error.compile_err))
  | Invalid(string, list(Error.compile_err));

/**
 table for storing module ASTs
 */
type t = {
  modules: Hashtbl.t(Namespace.t, entry_t),
  mutable plugins: list((Plugin.t, module_entries_t)),
  mutable globals: module_entries_t,
};

/* static */

/**
 construct a new table for module information
 */
let create = (~plugins=[], size: int): t => {
  modules: Hashtbl.create(size),
  plugins,
  globals: [],
};

/* methods */

let find = (id: Namespace.t, {modules, _}: t) =>
  Hashtbl.find_opt(modules, id);

let mem = (id: Namespace.t, {modules, _}: t) => Hashtbl.mem(modules, id);

/**
 add a module with associated export types and AST
 */
let add = (id: Namespace.t, entry: entry_t, {modules, _}: t) =>
  Hashtbl.replace(modules, id, entry);

/**
 add types for a plugin module
 */
let add_plugin = (plugin: Plugin.t, types: module_entries_t, table: t) =>
  table.plugins = table.plugins @ [(plugin, types)];

/**
 set global types
 */
let set_globals = (globals: module_entries_t, table: t) =>
  table.globals = globals;

/**
 remove an entry from the table
 */
let remove = (id: Namespace.t, {modules, _}: t) =>
  Hashtbl.remove(modules, id);

/**
 purge an entry from the table
 */
let purge = (id: Namespace.t, {modules, _}: t) =>
  Hashtbl.replace(modules, id, Purged);

/**
 prepare a pending entry in the table
 */
let prepare = (id: Namespace.t, {modules, _}: t) =>
  Hashtbl.replace(modules, id, Pending);

/**
 unpack raw source code from an entry as an option
 */
let get_entry_raw: entry_t => option(string) =
  fun
  | Valid(raw, _) => Some(raw)
  /* | Invalid(Some(_) as data, _) => data */
  | _ => None;

/**
 unpack data from an entry as an option
 */
let get_entry_data: entry_t => option(module_t) =
  fun
  | Valid(_, data) => Some(data)
  | Partial(_, data, _) => Some(data)
  | _ => None;

let get_global_values = ({globals, _}: t) =>
  globals
  |> List.filter_map(
       fun
       | (id, Type.Container.Value(type_)) => Some((id, type_))
       | _ => None,
     );

let get_global_types = ({globals, _}: t) =>
  globals
  |> List.filter_map(
       fun
       | (id, Type.Container.Type(type_)) => Some((id, type_))
       | _ => None,
     );

let _compare_data = (x, y) => x.ast == y.ast && x.symbols == y.symbols;

/**
 compare two ModuleTables by direct equality
 */
let compare: (t, t) => bool =
  (lhs, rhs) =>
    lhs.plugins == rhs.plugins
    && lhs.globals == rhs.globals
    && Hashtbl.compare(
         ~compare=
           (x, y) =>
             switch (x, y) {
             | (Valid(x_raw, x_data), Valid(y_raw, y_data)) =>
               x_raw == y_raw && _compare_data(x_data, y_data)

             | (
                 Partial(x_raw, x_data, x_errors),
                 Partial(y_raw, y_data, y_errors),
               ) =>
               x_raw == y_raw
               && _compare_data(x_data, y_data)
               && x_errors == y_errors

             | _ => x == y
             },
         lhs.modules,
         rhs.modules,
       );

let iter = (f: (Namespace.t, entry_t) => unit, table: t) =>
  Hashtbl.iter(f, table.modules);

let reset = (table: t) => {
  table.plugins = [];
  table.globals = [];
  Hashtbl.reset(table.modules);
};

let to_module_list = ({modules, _}: t) =>
  modules |> Hashtbl.to_seq |> List.of_seq;

/* pretty printing */

let _pp_library: Fmt.t(library_t) =
  (ppf, {symbols}) =>
    Fmt.(
      [("symbols", symbols |> ~@SymbolTable.pp)]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_module: Fmt.t(Result.program_t) => Fmt.t(module_t) =
  (pp_program, ppf, {ast, symbols, _}) =>
    Fmt.(
      [
        ("ast", ast |> ~@pp_program),
        ("symbols", symbols |> ~@SymbolTable.pp),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq
      |> Hashtbl.pp(string, string, ppf)
    );

let _pp_entry: Fmt.t(Result.program_t) => Fmt.t(entry_t) =
  (pp_program, ppf) =>
    fun
    | Library(raw, library) =>
      Fmt.pf(ppf, "Library(%s, %a)", raw, _pp_library, library)
    | Valid(raw, module_) =>
      Fmt.pf(ppf, "Valid(%s, %a)", raw, _pp_module(pp_program), module_)
    | Partial(raw, data, errs) =>
      Fmt.(
        pf(
          ppf,
          "Partial(%s, %a, %a)",
          raw,
          _pp_module(pp_program),
          data,
          Error.pp_dump_err_list,
          errs,
        )
      )
    | Invalid(raw, errs) =>
      Fmt.(pf(ppf, "Invalid(%s, %a)", raw, Error.pp_dump_err_list, errs))
    | Purged => Fmt.pf(ppf, "Purged")
    | Pending => Fmt.pf(ppf, "Pending");

let pp: Fmt.t(Result.program_t) => Fmt.t(t) =
  (pp_program, ppf, table: t) =>
    Fmt.struct_(
      Fmt.string,
      Fmt.string,
      ppf,
      (
        "ModuleTable",
        [
          ("plugins", ""),
          (
            "globals",
            table.globals
            |> ~@
                 Fmt.(
                   record(string, Type.Container.pp_module_entry(Type.pp))
                 ),
          ),
          (
            "modules",
            table.modules
            |> ~@Hashtbl.pp(Namespace.pp, _pp_entry(pp_program)),
          ),
        ],
      ),
    );
