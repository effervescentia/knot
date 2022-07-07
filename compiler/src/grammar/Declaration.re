open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ParseContext.t) =>
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
        Style.parser(ctx, f),
      ])
      >|= N.map(A.of_decl)
  );
