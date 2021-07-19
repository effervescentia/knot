open Kore;
open AST.Final.Util;

let namespace = imports =>
  M.string
  >|= Block.value
  >|= Reference.Namespace.of_string
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Tuple.split2(Block.value % to_public, Block.cursor)
  >|= to_main_import
  >|= (x => [x]);

let named_import = (ctx: Context.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Block.value
  >|= List.map(to_named_import);

let parser = (ctx: Context.t) =>
  Keyword.import
  >> (choice([main_import, named_import(ctx)]) |> M.comma_sep)
  >|= List.flatten
  << Keyword.from
  >>= namespace
  >@= (
    ((namespace, imports)) =>
      imports
      |> List.iter(
           AST.Final.(
             fun
             | MainImport((alias, cursor)) =>
               ctx |> Context.import(namespace, (Main, cursor), alias)
             | NamedImport((id, cursor), None) =>
               ctx |> Context.import(namespace, (Named(id), cursor), id)
             | NamedImport((id, cursor), Some((label, _))) =>
               ctx |> Context.import(namespace, (Named(id), cursor), label)
           ),
         )
  )
  >|= to_import
  |> M.terminated;
