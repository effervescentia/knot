open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let style = Style.parser;

let parser = (ctx: ModuleContext.t) =>
  A.of_main_export
  <$ Keyword.main
  |> option(A.of_named_export)
  >>= (
    f =>
      choice([
        Constant.parser(ctx, f),
        Enumerated.parser(ctx, f),
        Function.parser(ctx, f),
        View.parser(ctx, f),
      ])
      >|= N.map(A.of_decl)
  );
