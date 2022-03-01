open Kore;

let namespace = imports =>
  M.string
  >|= NR.map_value(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= NR.map_value(A.of_public)
  >|= (import => [import |> NR.wrap(A.of_main_import)]);

let named_import = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= NR.get_value
  >|= List.map(((name, label) as import) =>
        (
          A.of_named_import(import),
          NR.(
            Range.join(
              get_range(name),
              label |> Option.map(get_range) |?: get_range(name),
            )
          ),
        )
      );

let parser = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= NR.get_range
  % (
    start =>
      choice([main_import, named_import(ctx)])
      |> M.comma_sep
      >|= List.flatten
      << Keyword.from
      >>= namespace
      >@= (
        (((namespace, namespace_range), imports)) => {
          ctx |> ModuleContext.assert_module(namespace, namespace_range);

          let import = ModuleContext.import(namespace);

          imports
          |> List.iter(
               fun
               | (A.MainImport((alias, _)), range) =>
                 ctx |> import((Main, range), alias)

               | (A.NamedImport((id, _), None), range) =>
                 ctx |> import((Named(id), range), id)

               | (A.NamedImport((id, _), Some(label)), range) =>
                 ctx |> import((Named(id), range), NR.get_value(label)),
             );
        }
      )
      >|= (
        ((namespace, imports)) => (
          (NR.get_value(namespace), imports) |> A.of_import,
          Range.join(start, NR.get_range(namespace)),
        )
      )
      |> M.terminated
  );
