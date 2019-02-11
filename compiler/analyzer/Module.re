open Core;

type member_type =
  | Number
  | String
  | Boolean
  | Function
  | View
  | State
  | Style
  | Module;

type member_locality =
  | External(string)
  | Module;

type scope_member = {
  type_: member_type,
  locality: member_locality,
};

let add_import = (scope, module_, name) =>
  Hashtbl.add(scope, name, {type_: Module, locality: External(module_)});

let rec analyze_import = (scope, module_) =>
  fun
  | MainExport(name) => add_import(scope, module_, name)
  | ModuleExport(name) => add_import(scope, module_, name)
  | NamedExport(name, new_name) =>
    (
      switch (new_name) {
      | Some(s) => s
      | None => name
      }
    )
    |> add_import(scope, module_);

let analyze_declaration = scope =>
  fun
  | StateDecl(name, _, _) =>
    Hashtbl.add(scope, name, {type_: State, locality: Module})
  | _ => ();

let rec analyze = scope =>
  fun
  | Statements(stmts) => List.iter(analyze(scope), stmts)
  | Import(module_, imports) =>
    List.iter(analyze_import(scope, module_), imports)
  | Declaration(decl) => analyze_declaration(scope, decl);