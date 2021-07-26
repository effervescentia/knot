open Kore;
open AST;

let namespace = imports =>
  M.string
  >|= Node.Raw.value
  >|= Reference.Namespace.of_string
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Tuple.split2(Node.Raw.value % of_public, Node.Raw.cursor)
  >|= (import => [(of_main_import(import), Node.Raw.cursor(import))]);

let named_import = (ctx: ModuleContext.t) => {
  let closure_ctx = ClosureContext.from_module(ctx);

  Identifier.parser(closure_ctx)
  >>= (
    id =>
      Keyword.as_
      >> Identifier.parser(closure_ctx)
      >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(closure_ctx) >|= (id => (id, None)))
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
};

let parser = (ctx: ModuleContext.t) =>
  Keyword.import
  >> (choice([main_import, named_import(ctx)]) |> M.comma_sep)
  >|= List.flatten
  << Keyword.from
  >>= namespace
  >@= (
    ((namespace, imports)) => {
      let import = ModuleContext.import(namespace);

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
  >|= of_import
  |> M.terminated;
