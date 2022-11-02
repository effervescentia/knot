open Kore;
open ModuleAliases;

module Namespace = Reference.Namespace;

let _sort_imports =
  List.sort((l, r) =>
    (l, r)
    |> Tuple.map2(
         fst
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
  % List.map(((namespace, imports)) => {
      let (main_import, named_imports) =
        imports
        |> List.fold_left(
             ((m, n)) =>
               fst
               % (
                 fun
                 | AST.Result.MainImport(id) => (Some(fst(id)), n)
                 | AST.Result.NamedImport(id, label) => (
                     m,
                     [(fst(id), label), ...n],
                   )
               ),
             (None, []),
           );

      let sorted_named_imports =
        named_imports
        |> List.sort((l, r) =>
             (l, r) |> Tuple.map2(fst) |> Tuple.join2(String.compare)
           );

      (namespace, main_import, sorted_named_imports);
    });

let extract_imports = (program: AST.Result.program_t) =>
  program
  |> List.filter_map(
       fst
       % (
         fun
         | AST.Result.Import(namespace, imports) =>
           Some((namespace, imports))
         | _ => None
       ),
     )
  |> List.partition(
       Namespace.(
         fun
         | (Internal(_), _) => true
         | _ => false
       ),
     )
  |> Tuple.map2(_sort_imports);

let extract_declarations = (program: AST.Result.program_t) =>
  program
  |> List.filter_map(
       fst
       % (
         fun
         | AST.Result.Declaration(
             MainExport(name) | NamedExport(name),
             decl,
           ) =>
           Some((fst(name), fst(decl)))
         | _ => None
       ),
     );
