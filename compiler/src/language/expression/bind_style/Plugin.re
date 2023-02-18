include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});

let tokenize: Interface.Plugin.tokenize_t('expr, 'prim, 'typ) =
  (tokenize_expr, (_, view, style)) =>
    AST.TokenTree2.(
      join(
        view |> tokenize_expr |> wrap(Knot.Node.get_range(view)),
        style |> tokenize_expr |> wrap(Knot.Node.get_range(style)),
      )
    );
