open Knot.Kore;
open Parse.Kore;

module Export = Reference.Export;
module Keyword = Constants.Keyword;
module ParseContext = AST.ParseContext;

let __import_keyword = Matchers.keyword(Keyword.import);
let __from_keyword = Matchers.keyword(Keyword.from);

let _parse_import_named = (ctx: ParseContext.t, import) =>
  fun
  | (((id, _), None), _) as no_alias =>
    ctx
    |> import(Export.Named(id), id)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(no_alias)))

  | (((id, _), Some((alias, _))), _) as with_alias =>
    ctx
    |> import(Export.Named(id), alias)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(with_alias)));

let _parse_import_module = (ctx, imports, import) =>
  imports
  |> List.map(
       AST.Module.(
         fun
         | (MainImport((alias, _)), _) as main_import =>
           ctx
           |> import(Export.Main, alias)
           |> Result.map_error(Tuple.with_snd2(Node.get_range(main_import)))

         | (NamedImport(id, alias), _) as named_import =>
           _parse_import_named(
             ctx,
             import,
             Node.untyped((id, alias), Node.get_range(named_import)),
           )
       ),
     );

let parse_namespace = imports =>
  Matchers.string
  >|= Node.map(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let parse_main_import =
  Matchers.identifier
  >|= (import => [import |> Node.wrap(AST.Result.of_main_import)]);

let parse_named_imports = (ctx: ParseContext.t) =>
  KIdentifier.Parser.parse_raw(ctx)
  >>= (
    id =>
      Matchers.keyword(Keyword.as_)
      >> KIdentifier.Parser.parse_raw(ctx)
      >|= (alias => (id, Some(alias)))
  )
  <|> (KIdentifier.Parser.parse_raw(ctx) >|= (id => (id, None)))
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(
        List.map(((name, alias) as import) =>
          Node.untyped(import, Node.join_ranges(name, alias |?: name))
        ),
      );

let parse_module_import = (ctx: ParseContext.t) =>
  __import_keyword
  >>= (
    kwd =>
      choice([
        parse_main_import,
        parse_named_imports(ctx)
        >|= fst
        >|= List.map(Node.map(AST.Result.of_named_import)),
      ])
      |> Matchers.comma_sep
      >|= List.flatten
      << __from_keyword
      >>= parse_namespace
      >@= (
        (((namespace, _), imports)) =>
          namespace
          |> ParseContext.import
          |> _parse_import_module(ctx, imports)
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

let parse_standard_import = (ctx: ParseContext.t) =>
  __import_keyword
  >>= (
    kwd =>
      parse_named_imports(ctx)
      >@= fst
      % List.iter(
          _parse_import_named(ctx, ParseContext.import(Stdlib))
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

let parse = (ctx: ParseContext.t) =>
  choice([parse_module_import(ctx), parse_standard_import(ctx)])
  |> Matchers.terminated;
