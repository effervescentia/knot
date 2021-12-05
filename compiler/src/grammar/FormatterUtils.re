open Kore;
open AST;
open Reference;
open Pretty;

let _sort_imports =
  List.sort((l, r) =>
    (l, r)
    |> Tuple.map2(
         fst
         % Namespace.(
             fun
             | Internal(name)
             | External(name) => name
           ),
       )
    |> Tuple.join2(String.compare)
  )
  % List.map(((namespace, imports)) => {
      let (main_import, named_imports) =
        imports
        |> List.fold_left(
             ((m, n)) =>
               Node.Raw.get_value
               % (
                 fun
                 | MainImport(id) => (Some(Node.Raw.get_value(id)), n)
                 | NamedImport(id, label) => (
                     m,
                     [(Node.Raw.get_value(id), label), ...n],
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

let extract_imports = (program: program_t) =>
  program
  |> List.filter_map(
       Node.Raw.get_value
       % (
         fun
         | Import(namespace, imports) => Some((namespace, imports))
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

let extract_declarations = (program: program_t) =>
  program
  |> List.filter_map(
       Node.Raw.get_value
       % (
         fun
         | Declaration(MainExport(name) | NamedExport(name), decl) =>
           Some((Node.Raw.get_value(name), Node.get_value(decl)))
         | _ => None
       ),
     );
