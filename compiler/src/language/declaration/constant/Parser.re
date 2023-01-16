open Knot.Kore;
open Parse.Kore;
open AST;

let parse =
    ((ctx: ParseContext.t('ast), export: ModuleStatement.ExportKind.t)) =>
  Matchers.keyword(Constants.Keyword.const)
  >>= (
    kwd =>
      Matchers.assign(
        KIdentifier.Parser.parse_raw(ctx),
        KExpression.Plugin.parse(ctx),
      )
      >|= (
        ((name, expression)) => {
          let scope =
            ctx |> Scope.of_parse_context(Node.get_range(expression));
          let expression' = expression |> KExpression.Plugin.analyze(scope);
          let type_ = Node.get_type(expression');

          // TODO: throw error if name already used in scope

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=ModuleStatement.ExportKind.is_main(export),
               fst(name),
               type_,
             );

          let result = expression' |> Node.wrap(Fun.id);
          let range = Node.join_ranges(kwd, expression);

          Node.raw((name, result), range);
        }
      )
      |> Matchers.terminated
  );
