open Core;
open AST;

module M = Matchers;

let rec stmts = input => (many(stmt) ==> (l => Statements(l)))(input)
and stmt = input => (_import <|> _decl)(input)
and _import =
  M.import
  >> M.identifier
  >>= (
    main =>
      M.from
      >> M.string
      >>= (s => return(Import(s, [MainExport(main)])))
      << optional(M.semicolon)
  )
and _decl = input =>
  (
    Const.decl
    <|> State.decl
    <|> Function.decl
    <|> View.decl
    ==> (d => Declaration(d))
  )(
    input,
  );