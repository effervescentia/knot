open Kore;
open AST;

let namespace = imports =>
  M.string
  >|= Tuple.map_fst2(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Tuple.split2(Node.Raw.value % of_public, Node.Raw.cursor)
  >|= (import => [(of_main_import(import), Node.Raw.cursor(import))]);

let named_import = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Node.Raw.value
  >|= List.map(((name, label) as import) =>
        (
          of_named_import(import),
          Cursor.join(
            Node.Raw.cursor(name),
            label |> Option.map(Node.Raw.cursor) |?: Node.Raw.cursor(name),
          ),
        )
      );

let parser = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= Node.Raw.cursor
  % (
    start =>
      choice([main_import, named_import(ctx)])
      |> M.comma_sep
      >|= List.flatten
      << Keyword.from
      >>= namespace
      >@= (
        ((namespace, imports)) => {
          let import = namespace |> Node.Raw.value |> ModuleContext.import;

          imports
          |> List.iter(
               fun
               | (MainImport((alias, _)), cursor) =>
                 ctx |> import((Main, cursor), alias)
               | (NamedImport((id, _), None), cursor) =>
                 ctx |> import((Named(id), cursor), id)
               | (NamedImport((id, _), Some(label)), cursor) =>
                 ctx |> import((Named(id), cursor), Node.Raw.value(label)),
             );
        }
      )
      >|= (
        ((namespace, imports)) => (
          (Node.Raw.value(namespace), imports) |> of_import,
          Cursor.join(start, Node.Raw.cursor(namespace)),
        )
      )
      |> M.terminated
  );
