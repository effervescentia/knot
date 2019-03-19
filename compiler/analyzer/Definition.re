open Core;
open Scope;

exception MultipleMainDefinitions;

let rec analyze_type =
  fun
  | ObjectDefn(members) =>
    Object_t(
      Hashtbl.fold(
        (key, value, tbl) => {
          ref(Declared(analyze_type(value))) |> Hashtbl.add(tbl, key);
          tbl;
        },
        members,
        Hashtbl.create(Hashtbl.length(members)),
      ),
    )
  | _ => raise(NotImplemented);

let analyze_stmt = (tbl, main_defn) =>
  fun
  | MainDefn(defn) =>
    switch (main_defn^) {
    | Some(_) => raise(MultipleMainDefinitions)
    | None => main_defn := Some(ref(Declared(analyze_type(defn))))
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
