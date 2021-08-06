open Kore;
open AST;

let namespace = imports =>
  M.string
  >|= Tuple.map_fst2(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Node.Raw.(Tuple.split2(get_value % of_public, get_range))
  >|= (import => [(of_main_import(import), Node.Raw.get_range(import))]);

let named_import = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Node.Raw.get_value
  >|= List.map(((name, label) as import) =>
        (
          of_named_import(import),
          Node.Raw.(
            Range.join(
              get_range(name),
              label |> Option.map(get_range) |?: get_range(name),
            )
          ),
        )
      );

let parser = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= Node.Raw.get_range
  % (
    start =>
      choice([main_import, named_import(ctx)])
      |> M.comma_sep
      >|= List.flatten
      << Keyword.from
      >>= namespace
      >@= (
        ((namespace, imports)) => {
          let import = namespace |> Node.Raw.get_value |> ModuleContext.import;

          imports
          |> List.iter(
               fun
               | (MainImport((alias, _)), range) =>
                 ctx |> import((Main, range), alias)
               | (NamedImport((id, _), None), range) =>
                 ctx |> import((Named(id), range), id)
               | (NamedImport((id, _), Some(label)), range) =>
                 ctx
                 |> import((Named(id), range), Node.Raw.get_value(label)),
             );
        }
      )
      >|= (
        ((namespace, imports)) => (
          (Node.Raw.get_value(namespace), imports) |> of_import,
          Range.join(start, Node.Raw.get_range(namespace)),
        )
      )
      |> M.terminated
  );
