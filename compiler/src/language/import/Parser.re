open Knot.Kore;
open Parse.Onyx;

module Export = Reference.Export;
module Keyword = Constants.Keyword;
module Matchers = Parse.Matchers;
module ParseContext = AST.ParseContext;

let _import_keyword = Matchers.keyword(Keyword.import);
let _from_keyword = Matchers.keyword(Keyword.from);

let _import_named = (ctx: ParseContext.t, import) =>
  fun
  | (((id, _), None), _) as no_alias =>
    ctx
    |> import(Export.Named(id), id)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(no_alias)))

  | (((id, _), Some((alias, _))), _) as with_alias =>
    ctx
    |> import(Export.Named(id), alias)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(with_alias)));

let _import_module = (ctx, imports, import) =>
  imports
  |> List.map(
       AST.Module.(
         fun
         | (MainImport((alias, _)), _) as main_import =>
           ctx
           |> import(Export.Main, alias)
           |> Result.map_error(Tuple.with_snd2(Node.get_range(main_import)))

         | (NamedImport(id, alias), _) as named_import =>
           _import_named(
             ctx,
             import,
             Node.untyped((id, alias), Node.get_range(named_import)),
           )
       ),
     );

let namespace = imports =>
  Matchers.string
  >|= Node.map(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  Matchers.identifier
  >|= (import => [import |> Node.wrap(AST.Result.of_main_import)]);

let named_imports = (ctx: ParseContext.t) =>
  KIdentifier.Plugin.parse(ctx)
  >>= (
    id =>
      Matchers.keyword(Keyword.as_)
      >> KIdentifier.Plugin.parse(ctx)
      >|= (alias => (id, Some(alias)))
  )
  <|> (KIdentifier.Plugin.parse(ctx) >|= (id => (id, None)))
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(
        List.map(((name, alias) as import) =>
          Node.untyped(import, Node.join_ranges(name, alias |?: name))
        ),
      );

let module_import = (ctx: ParseContext.t) =>
  _import_keyword
  >>= (
    kwd =>
      choice([
        main_import,
        named_imports(ctx)
        >|= fst
        >|= List.map(Node.map(AST.Result.of_named_import)),
      ])
      |> Matchers.comma_sep
      >|= List.flatten
      << _from_keyword
      >>= namespace
      >@= (
        (((namespace, _), imports)) =>
          namespace
          |> ParseContext.import
          |> _import_module(ctx, imports)
          |> List.iter(
               Result.iter_error(((err, range)) =>
                 ctx |> ParseContext.report(TypeError(err), range)
               ),
             )
      )
      >|= (
        ((namespace, imports)) =>
          Node.untyped(
            (fst(namespace), imports) |> AST.Result.of_import,
            Node.join_ranges(kwd, namespace),
          )
      )
  );

let standard_import = (ctx: ParseContext.t) =>
  _import_keyword
  >>= (
    kwd =>
      named_imports(ctx)
      >@= fst
      % List.iter(
          _import_named(ctx, ParseContext.import(Stdlib))
          % Result.iter_error(_ => ()),
        )
      >|= (
        imports =>
          Node.untyped(
            AST.Result.of_standard_import(fst(imports)),
            Node.join_ranges(kwd, imports),
          )
      )
  );

let import = (ctx: ParseContext.t) =>
  choice([module_import(ctx), standard_import(ctx)]) |> Matchers.terminated;
