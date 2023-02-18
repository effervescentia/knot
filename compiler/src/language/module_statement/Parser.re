open Knot.Kore;
open Parse.Kore;
open AST;

module ExportKind = Interface.ExportKind;

let parse_export = (parse_decl, ctx: ParseContext.t('ast)) =>
  ExportKind.Main
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(ExportKind.Named)
  >>= (
    export =>
      parse_decl(ExportKind.is_main(export), ctx)
      >|= Node.map(((name, declaration)) =>
            (export, name, declaration) |> Interface.of_export
          )
  );

let parse =
    (parse_decl)
    : Framework.Interface.contextual_parse_t('ast, Interface.t('decl)) =>
  ctx =>
    choice([
      KImport.Plugin.parse(
        (Interface.of_import, Interface.of_stdlib_import),
        ctx,
      ),
      parse_export(parse_decl, ctx),
    ]);
