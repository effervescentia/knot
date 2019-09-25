open Core;
open Scope;

let rec analyze_type =
  fun
  | BooleanDefn => Boolean_t
  | NumberDefn => Number_t
  | StringDefn => String_t
  | JSXDefn => JSX_t
  | NilDefn => Nil_t
  | ObjectDefn(members) =>
    Object_t(
      Hashtbl.fold(
        (key, value, tbl) => {
          analyze_type(value) |> Hashtbl.add(tbl, key);
          tbl;
        },
        members,
        Hashtbl.create(Hashtbl.length(members)),
      ),
    )
  | FunctionDefn(args, ret) =>
    Function_t(List.map(x => analyze_type(x), args), analyze_type(ret));

let analyze_stmt = (tbl, main_defn) =>
  fun
  | MainDefn(defn) =>
    switch (main_defn^) {
    | Some(_) => throw_semantic(MultipleMainDefinitions)
    | None => main_defn := Some(analyze_type(defn))
    };

let analyze =
  Knot.DefinitionAST.(
    fun
    | ModuleDefn(stmts) => {
        let tbl = Hashtbl.create(List.length(stmts));
        let main_defn = ref(None);

        List.iter(analyze_stmt(tbl, main_defn), stmts);

        Module_t([], tbl, main_defn^);
      }
  );
