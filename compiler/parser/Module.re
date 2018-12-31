open Parsing;
open AST;

let rec stmts = input => (many(stmt) ==> (l => Statements(l)))(input)
and stmt = input => _import(input)
and _import =
  Matchers.import
  >> Matchers.identifier
  >>= (
    main =>
      Matchers.from
      >> Matchers.string
      >>= (s => return(Import(s, [MainExport(main)])))
      << optional(Matchers.semicolon)
  );
/* and _decl = input =>
   (Matchers.import_ >> return(Import("sads", [])))(input); */