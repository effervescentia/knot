open Kore;
open ModuleAliases;

module Namespace = Reference.Namespace;
module Identifier = Reference.Identifier;

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
           ),
       )
    |> Tuple.join2(String.compare)
  )
  % List.map(((namespace, imports)) => {
      let (main_import, named_imports) =
        imports
        |> List.fold_left(
             ((m, n)) =>
               NR.get_value
               % (
                 fun
                 | A.MainImport(id) => (Some(NR.get_value(id)), n)
                 | A.NamedImport(id, label) => (
                     m,
                     [(NR.get_value(id), label), ...n],
                   )
               ),
             (None, []),
           );

      let sorted_named_imports =
        named_imports
        |> List.sort((l, r) =>
             (l, r)
             |> Tuple.map2(fst % Identifier.to_string)
             |> Tuple.join2(String.compare)
           );

      (namespace, main_import, sorted_named_imports);
    });

let extract_imports = (program: A.program_t) =>
  program
  |> List.filter_map(
       NR.get_value
       % (
         fun
         | A.Import(namespace, imports) => Some((namespace, imports))
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

let extract_declarations = (program: A.program_t) =>
  program
  |> List.filter_map(
       NR.get_value
       % (
         fun
         | A.Declaration(MainExport(name) | NamedExport(name), decl) =>
           Some((NR.get_value(name), Node.get_value(decl)))
         | _ => None
       ),
     );
