open Kore;
open Parse.Kore;
open AST;

// module ExportKind = KModuleStatement.Interface.ExportKind;

let parse = (is_main, ctx: ParseContext.t('ast)) => {
  let (&>) = (parse, to_declaration) =>
    parse(is_main, ctx)
    >|= Node.map(Tuple.map_snd2(Node.map(to_declaration)));

  choice([
    KConstant.parse &> Interface.of_constant,
    KEnumerated.parse &> Interface.of_enumerated,
    KFunction.parse &> Interface.of_function,
    KView.parse &> Interface.of_view,
  ]);
};

// let parse = (ctx: ParseContext.t('ast)) =>
//   ExportKind.Main
//   <$ Matchers.keyword(Constants.Keyword.main)
//   |> option(ExportKind.Named)
//   >|= Tuple.with_fst2(ctx)
//   >>= (
//     ((ctx, export)) => {
//       let (&>) = (parse, to_declaration) =>
//         parse(ExportKind.is_main(export), ctx)
//         >|= Node.map(((name, declaration)) =>
//               (export, name, declaration |> Node.map(to_declaration))
//               |> KModuleStatement.Interface.of_export
//             );

//       choice([
//         KConstant.parse &> Interface.of_constant,
//         KEnumerated.parse &> Interface.of_enumerated,
//         KFunction.parse &> Interface.of_function,
//         KView.parse &> Interface.of_view,
//       ]);
//     }
//   );
