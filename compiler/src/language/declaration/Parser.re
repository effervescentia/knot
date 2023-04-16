open Kore;
open Parse.Kore;
open AST;

let parse = (is_main, ctx: ParseContext.t('ast)) => {
  let (&>) = (parse, to_declaration) =>
    parse(is_main, ctx)
    >|= Node.map(Tuple.map_snd2(Node.map(to_declaration)));

  choice([
    Constant.parse &> Interface.of_constant,
    Enumerated.parse &> Interface.of_enumerated,
    Function.parse &> Interface.of_function,
    View.parse &> Interface.of_view,
  ]);
};
