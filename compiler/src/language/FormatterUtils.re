open Knot.Kore;
open AST;

module Namespace = Reference.Namespace;

let _sort_imports =
  List.sort((l, r) =>
    (l, r)
    |> Tuple.map2(
         Tuple.fst3
         % Namespace.(
             fun
             | Internal(name)
             | External(name) => name
             | Stdlib => ""
             | Ambient => ""
           ),
       )
    |> Tuple.join2(String.compare)
  )
  % List.map(((namespace, main_import, named_imports)) => {
      let main_import' = main_import |> Option.map(id => fst(id));
      let named_imports' =
        named_imports
        |> List.map((((id, alias), _)) =>
             (fst(id), alias |> Option.map(fst))
           );

      let sorted_named_imports =
        named_imports'
        |> List.sort((l, r) =>
             (l, r) |> Tuple.map2(fst) |> Tuple.join2(String.compare)
           );

      (namespace, main_import', sorted_named_imports);
    });

let extract_imports = (program: Module.program_t) =>
  program
  |> List.fold_left(
       acc =>
         fst
         % ModuleStatement.(
             fun
             | StdlibImport(named_imports) =>
               acc
               |> Tuple.map_fst3(
                    named_imports
                    |> List.map(
                         fst % Tuple.map_each2(fst, Option.map(fst)),
                       )
                    |> List.incl_all,
                  )
             | Import(External(_) as namespace, main_import, named_imports) =>
               acc
               |> Tuple.map_snd3(
                    List.cons((namespace, main_import, named_imports)),
                  )
             | Import(Internal(_) as namespace, main_import, named_imports) =>
               acc
               |> Tuple.map_thd3(
                    List.cons((namespace, main_import, named_imports)),
                  )
             | _ => acc
           ),
       ([], [], []),
     )
  |> Tuple.map_each3(Fun.id, _sort_imports, _sort_imports);

let extract_declarations = (program: Module.program_t) =>
  program
  |> List.filter_map(
       fst
       % ModuleStatement.(
           fun
           | Export(_, name, decl) => Some((fst(name), fst(decl)))
           | _ => None
         ),
     );
