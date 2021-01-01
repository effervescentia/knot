open Kore;

let constant_decl =
  Keyword.const
  >> M.identifier
  >>= fst
  % (
    id =>
      Operator.assign >> Expression.parser >|= AST.of_const % (x => (id, x))
  );

let parser = choice([constant_decl]) >|= AST.of_decl;
