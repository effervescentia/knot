open Kore;
open AST;

let namespace = imports =>
  M.string
  >|= Block.value
  >|= Reference.Namespace.of_string
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Tuple.split2(Block.value % of_public, Block.cursor)
  >|= of_main_import
  >|= (x => [x]);

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
  >|= Block.value
  >|= List.map(of_named_import);
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
           | MainImport((alias, cursor)) =>
             ctx |> import((Main, cursor), alias)
           | NamedImport((id, cursor), None) =>
             ctx |> import((Named(id), cursor), id)
           | NamedImport((id, cursor), Some(label)) =>
             ctx |> import((Named(id), cursor), Block.value(label)),
         );
    }
  )
  >|= of_import
  |> M.terminated;
