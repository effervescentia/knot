open Knot.Kore;
open Parse.Kore;

module Export = Reference.Export;
module Keyword = Constants.Keyword;
module ParseContext = AST.ParseContext;
module Identifier = KIdentifier.Plugin;

let __import_keyword = Matchers.keyword(Keyword.import);
let __from_keyword = Matchers.keyword(Keyword.from);

let _import_named = (ctx: ParseContext.t('ast), import) =>
  fun
  | (((id, _), None), _) as no_alias =>
    ctx
    |> import(Export.Named(id), id)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(no_alias)))

  | (((id, _), Some((alias, _))), _) as with_alias =>
    ctx
    |> import(Export.Named(id), alias)
    |> Result.map_error(Tuple.with_snd2(Node.get_range(with_alias)));

let _import_main = (ctx: ParseContext.t('ast), import, alias) =>
  ctx
  |> import(Export.Main, fst(alias))
  |> Result.map_error(Tuple.with_snd2(Node.get_range(alias)));

let parse_namespace = ((main_import, named_imports)) =>
  Matchers.string
  >|= Node.map(Reference.Namespace.of_string)
  >|= (namespace => (namespace, main_import, named_imports));

let parse_main_import = Matchers.identifier;

let parse_named_imports = (ctx: ParseContext.t('ast)) =>
  Identifier.parse_raw(ctx)
  >>= (
    id =>
      Matchers.keyword(Keyword.as_)
      >> Identifier.parse_raw(ctx)
      >|= (alias => (id, Some(alias)))
  )
  <|> (Identifier.parse_raw(ctx) >|= (id => (id, None)))
  |> Matchers.comma_sep
  |> Matchers.between_braces
  >|= Node.map(
        List.map(((name, alias) as import) =>
          Node.raw(import, Node.join_ranges(name, alias |?: name))
        ),
      );

let parse_main_and_named_import = (ctx: ParseContext.t('ast)) =>
  option(None, parse_main_import >|= Option.some)
  << Matchers.symbol(Constants.Character.comma)
  >>= (
    main_import =>
      parse_named_imports(ctx) >|= fst >|= Tuple.with_fst2(main_import)
  );

let parse_module_import = (f, ctx: ParseContext.t('ast)) =>
  __import_keyword
  >>= (
    kwd =>
      choice([
        parse_main_and_named_import(ctx),
        parse_main_import >|= Option.some >|= Tuple.with_snd2([]),
        parse_named_imports(ctx) >|= fst >|= Tuple.with_fst2(None),
      ])
      << __from_keyword
      >>= parse_namespace
      >@= (
        (((namespace, _), main_import, named_imports)) => {
          let import = ParseContext.import(namespace);

          (
            main_import
            |> Option.map(main_import' =>
                 [_import_main(ctx, import, main_import')]
               )
            |?: []
          )
          @ (named_imports |> List.map(_import_named(ctx, import)))
          |> List.iter(
               Result.iter_error(((err, range)) =>
                 ctx |> ParseContext.report(TypeError(err), range)
               ),
             );
        }
      )
      >|= (
        ((namespace, main_import, named_imports)) =>
          Node.raw(
            (fst(namespace), main_import, named_imports) |> f,
            Node.join_ranges(kwd, namespace),
          )
      )
  );

let parse_standard_import = (f, ctx: ParseContext.t('ast)) =>
  __import_keyword
  >>= (
    kwd =>
      parse_named_imports(ctx)
      >@= fst
      % List.iter(
          _import_named(ctx, ParseContext.import(Stdlib))
          % Result.iter_error(_ => ()),
        )
      >|= (
        imports =>
          Node.raw(f(fst(imports)), Node.join_ranges(kwd, imports))
      )
  );

let parse = ((to_import, to_stdlib_import), ctx: ParseContext.t('ast)) =>
  choice([
    parse_module_import(to_import, ctx),
    parse_standard_import(to_stdlib_import, ctx),
  ])
  |> Matchers.terminated;
